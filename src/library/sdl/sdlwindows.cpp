/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sdlwindows.h"
#include "../hook.h"
#include "sdlversion.h"
#include "sdldisplay.h" // SDL_GetCurrentDisplayMode
#include "../logging.h"
#include "../../shared/sockethelpers.h"
#include "../../shared/messages.h"
#include "../../shared/SharedConfig.h"
#include "../frame.h"
#include "../renderhud/RenderHUD_GL.h"
#include "../renderhud/RenderHUD_SDL1.h"
#include "../renderhud/RenderHUD_SDL2_surface.h"
#include "../timewrappers.h"
#include "../ScreenCapture.h"
#include "../DeterministicTimer.h"
#include "../WindowTitle.h"
#include "../encoding/AVEncoder.h"
#include "SDLEventQueue.h"
#ifdef __unix__
#include "../glxwrappers.h" // checkMesa()
#endif
#include "../checkpoint/ThreadManager.h"
#include "../global.h"
#include "../GlobalState.h"

namespace libtas {

DECLARE_ORIG_POINTER(SDL_GL_SwapWindow)
DECLARE_ORIG_POINTER(SDL_CreateWindow)
DECLARE_ORIG_POINTER(SDL_GetWindowID)
DECLARE_ORIG_POINTER(SDL_GetWindowFromID)
DECLARE_ORIG_POINTER(SDL_GetWindowFlags)
DECLARE_ORIG_POINTER(SDL_SetWindowTitle)
DEFINE_ORIG_POINTER(SDL_WM_SetCaption)
DECLARE_ORIG_POINTER(SDL_GL_CreateContext)
DECLARE_ORIG_POINTER(SDL_GL_DeleteContext)
DECLARE_ORIG_POINTER(SDL_GL_SetSwapInterval)
DECLARE_ORIG_POINTER(SDL_DestroyWindow)
DECLARE_ORIG_POINTER(SDL_SetWindowSize)
DECLARE_ORIG_POINTER(SDL_CreateWindowAndRenderer)
DEFINE_ORIG_POINTER(SDL_SetVideoMode)
DEFINE_ORIG_POINTER(SDL_GL_SwapBuffers)
DEFINE_ORIG_POINTER(SDL_Flip)
DECLARE_ORIG_POINTER(SDL_SetColorKey)
DEFINE_ORIG_POINTER(SDL_UpdateRects)
DEFINE_ORIG_POINTER(SDL_UpdateRect)
DECLARE_ORIG_POINTER(SDL_GL_SetAttribute)
DECLARE_ORIG_POINTER(SDL_UpdateWindowSurface)
DECLARE_ORIG_POINTER(SDL_UpdateWindowSurfaceRects)

SDL_Window* sdl::gameSDLWindow = nullptr;

static int swapInterval = 0;
static bool windowFullscreen = false;

/* SDL 1.2 */
/* Override */ void SDL_GL_SwapBuffers(void)
{
    LINK_NAMESPACE_SDL1(SDL_GL_SwapBuffers);

    if (GlobalState::isNative())
        return orig::SDL_GL_SwapBuffers();

    DEBUGLOGCALL(LCF_SDL | LCF_OGL | LCF_WINDOW);

    /* Start the frame boundary and pass the function to draw */
    static RenderHUD_GL renderHUD_GL;
    frameBoundary([] () {orig::SDL_GL_SwapBuffers();}, renderHUD_GL);
}

/* Override */ void SDL_GL_SwapWindow(SDL_Window* window)
{
    LINK_NAMESPACE_SDL2(SDL_GL_SwapWindow);

    if (GlobalState::isNative())
        return orig::SDL_GL_SwapWindow(window);

    DEBUGLOGCALL(LCF_SDL | LCF_OGL | LCF_WINDOW);

    /* Start the frame boundary and pass the function to draw */
    static RenderHUD_GL renderHUD_GL;
    frameBoundary([&] () {orig::SDL_GL_SwapWindow(window);}, renderHUD_GL);
}

void* SDL_GL_CreateContext(SDL_Window *window)
{
    DEBUGLOGCALL(LCF_SDL | LCF_OGL | LCF_WINDOW);
    LINK_NAMESPACE_SDL2(SDL_GL_CreateContext);

    void* context;
    NATIVECALL(context = orig::SDL_GL_CreateContext(window));

    /* We override this function to disable vsync,
     * except when using non deterministic timer.
     */
    if (!(Global::shared_config.debug_state & SharedConfig::DEBUG_UNCONTROLLED_TIME)) {
        LINK_NAMESPACE_SDL2(SDL_GL_SetSwapInterval);
        orig::SDL_GL_SetSwapInterval(0);
        debuglogstdio(LCF_WINDOW, "Disable vsync !!");
    }

    /* If the context creation failed, we stop here */
    if (!context) {
        return context;
    }

    /* Now that the context is created, we can init the screen capture */
    ScreenCapture::init();

#ifdef __unix__
    /* Alerting the user if software rendering is not active */
    checkMesa();
#endif
    
    return context;
}

void SDL_GL_DeleteContext(SDL_GLContext context)
{
    DEBUGLOGCALL(LCF_SDL | LCF_OGL | LCF_WINDOW);
    LINK_NAMESPACE_SDL2(SDL_GL_DeleteContext);

    /* Delete texture and fbo in the OSD */
    RenderHUD_GL::fini();

    /* Games can destroy the GL context without closing the window. It still
     * invalidates GL objects, so we must close the screen capture. */
    ScreenCapture::fini();

    orig::SDL_GL_DeleteContext(context);
}

/* Override */ int SDL_GL_SetSwapInterval(int interval)
{
    debuglogstdio(LCF_SDL | LCF_OGL | LCF_WINDOW, "%s call - setting to %d", __func__, interval);
    LINK_NAMESPACE_SDL2(SDL_GL_SetSwapInterval);

    /* We save the interval if the game wants it later */
    swapInterval = interval;

    /* When using non deterministic timer, we let the game set vsync */
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_UNCONTROLLED_TIME) {
        debuglogstdio(LCF_WINDOW, "Set swap interval !!");
        int ret = orig::SDL_GL_SetSwapInterval(interval);
        debuglogstdio(LCF_WINDOW, "   return %d", ret);
        return ret;
        // return orig::SDL_GL_SetSwapInterval(interval);
    }

    return 0; // Success
}

/* Override */ int SDL_GL_GetSwapInterval(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_OGL | LCF_WINDOW);
    return swapInterval;
}

/* Override */ SDL_Window* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags){
    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call - title: %s, pos: (%d,%d), size: (%d,%d), flags: %x", __func__,  title?title:"", x, y, w, h, flags);
    LINK_NAMESPACE_SDL2(SDL_CreateWindow);

    ThreadManager::setMainThread();

    WindowTitle::setOriginalTitle(title);

    /* Disable fullscreen */
    windowFullscreen = (flags & SDL_WINDOW_FULLSCREEN);
    flags &= 0xFFFFFFFF ^ SDL_WINDOW_FULLSCREEN_DESKTOP;

    /* Disable hidden windows */
    // flags &= 0xFFFFFFFF ^ SDL_WINDOW_HIDDEN;

    /* Disable high DPI mode */
    flags &= 0xFFFFFFFF ^ SDL_WINDOW_ALLOW_HIGHDPI;

    /* Disable resizable window */
    flags &= 0xFFFFFFFF ^ SDL_WINDOW_RESIZABLE;

    if (Global::shared_config.screen_width && w > Global::shared_config.screen_width)
        w = Global::shared_config.screen_width;

    if (Global::shared_config.screen_height && h > Global::shared_config.screen_height)
        h = Global::shared_config.screen_height;

    sdl::gameSDLWindow = orig::SDL_CreateWindow(title, x, y, w, h, flags); // Save the game window

    if (flags & SDL_WINDOW_OPENGL) {
        Global::game_info.video |= GameInfo::OPENGL;
        Global::game_info.tosend = true;
    }
    else {
        Global::game_info.video &= ~GameInfo::OPENGL;
        Global::game_info.tosend = true;
    }

    LINK_NAMESPACE_SDL2(SDL_SetWindowTitle);
    WindowTitle::setUpdateFunc([] (const char* t) {orig::SDL_SetWindowTitle(sdl::gameSDLWindow, t);});

    /* Push the few events that generated by a window creation */
    struct timespec time = detTimer.getTicks();
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    SDL_Event event;
    event.type = SDL_WINDOWEVENT;
    event.window.windowID = 1;
    event.window.timestamp = timestamp;
    event.window.event = SDL_WINDOWEVENT_SHOWN;
    sdlEventQueue.insert(&event);

    event.window.event = SDL_WINDOWEVENT_MOVED;
    sdlEventQueue.insert(&event);

    event.window.event = SDL_WINDOWEVENT_SHOWN;
    sdlEventQueue.insert(&event);

    event.window.event = SDL_WINDOWEVENT_ENTER;
    sdlEventQueue.insert(&event);

    event.window.event = SDL_WINDOWEVENT_FOCUS_GAINED;
    sdlEventQueue.insert(&event);

#if defined(__APPLE__) && defined(__MACH__)
    /* As a temp measure on MacOS, send SDL window handle so that the
     * GameLoop can process events. Will be replaced by the low-level window
     * handle when implemented. */
    uint32_t win = reinterpret_cast<uintptr_t>(sdl::gameSDLWindow);
    lockSocket();
    sendMessage(MSGB_WINDOW_ID);
    sendData(&win, sizeof(win));
    unlockSocket();
    debuglogstdio(LCF_WINDOW, "Sent X11 window id %d", win);
#endif

    return sdl::gameSDLWindow;
}

/* Override */ void SDL_DestroyWindow(SDL_Window* window)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    LINK_NAMESPACE_SDL2(SDL_DestroyWindow);

    orig::SDL_DestroyWindow(window);

    if (sdl::gameSDLWindow == window)
        sdl::gameSDLWindow = nullptr;

    /* Destroy the AVEncoder object */
    if (avencoder)
        avencoder.reset(nullptr);

    ScreenCapture::fini();
}

/* Override */ Uint32 SDL_GetWindowID(SDL_Window* window)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    if (sdl::gameSDLWindow == window)
        return 1;
    LINK_NAMESPACE_SDL2(SDL_GetWindowID);
    return orig::SDL_GetWindowID(window);
}

/* Override */ SDL_Window* SDL_GetWindowFromID(Uint32 id)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    if (id == 1)
        return sdl::gameSDLWindow;
    LINK_NAMESPACE_SDL2(SDL_GetWindowFromID);
    return orig::SDL_GetWindowFromID(id);
}

/* Override */ Uint32 SDL_GetWindowFlags(SDL_Window* window){
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    LINK_NAMESPACE_SDL2(SDL_GetWindowFlags);
    Uint32 flags = orig::SDL_GetWindowFlags(window);
    flags |= SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS;
    if (windowFullscreen)
        flags |= SDL_WINDOW_FULLSCREEN;
    debuglogstdio(LCF_SDL | LCF_WINDOW, "  flags: %d", flags);
    return flags;
}

/* Override */ void SDL_SetWindowTitle(SDL_Window * window, const char *title)
{
    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with title %s", __func__, title?title:"[null]");
    LINK_NAMESPACE_SDL2(SDL_SetWindowTitle);

    WindowTitle::setOriginalTitle(title);
    WindowTitle::setUpdateFunc([window] (const char* t) {orig::SDL_SetWindowTitle(window, t);});
}

/* Override */ void SDL_WM_SetCaption(const char *title, const char *icon)
{
    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with title %s", __func__, title?title:"[null]");
    LINK_NAMESPACE_SDL1(SDL_WM_SetCaption);
    WindowTitle::setOriginalTitle(title);
    WindowTitle::setUpdateFunc([icon] (const char* t) {orig::SDL_WM_SetCaption(t, icon);});
}

/* Override */ int SDL_SetWindowFullscreen(SDL_Window * window, Uint32 flags)
{
    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with flags %d", __func__, flags);

    windowFullscreen = (flags & SDL_WINDOW_FULLSCREEN);

    if (flags == 0) // Windowed
        return 0;

    /* Resize the window to the screen or fake resolution */
    int w, h;
    if (Global::shared_config.screen_width) {
        w = Global::shared_config.screen_width;
        h = Global::shared_config.screen_height;
    }
    else {
        /* Change the window size to monitor size */
        SDL_DisplayMode dm;
        NATIVECALL(SDL_GetCurrentDisplayMode(0, &dm));
        w = dm.w;
        h = dm.h;
    }

    NATIVECALL(SDL_SetWindowSize(window, w, h));
    ScreenCapture::resize(w, h);
    return 0; // success
}

/* Override */ void SDL_SetWindowBordered(SDL_Window * window, SDL_bool bordered)
{
    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with border %d", __func__, bordered);
    /* Don't do anything */
}


/* Override */ int SDL_CreateWindowAndRenderer(int width, int height,
        Uint32 window_flags, SDL_Window **window, SDL_Renderer **renderer)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    debuglogstdio(LCF_SDL | LCF_WINDOW, "  size %d x %d", width, height);
    LINK_NAMESPACE_SDL2(SDL_CreateWindowAndRenderer);

    ThreadManager::setMainThread();

    /* Disable fullscreen */
    windowFullscreen = (window_flags & SDL_WINDOW_FULLSCREEN);
    window_flags &= 0xFFFFFFFF ^ SDL_WINDOW_FULLSCREEN_DESKTOP;

    /* Disable hidden windows */
    // window_flags &= 0xFFFFFFFF ^ SDL_WINDOW_HIDDEN;

    /* Disable high DPI mode */
    window_flags &= 0xFFFFFFFF ^ SDL_WINDOW_ALLOW_HIGHDPI;

    Global::game_info.video |= GameInfo::SDL2_RENDERER;

    int ret = orig::SDL_CreateWindowAndRenderer(width, height, window_flags, window, renderer);
    sdl::gameSDLWindow = *window;

    /* If we are going to save the screen when savestating, we need to init
     * our pixel access routine */
    ScreenCapture::init();

    return ret;
}

/* Override */ void SDL_SetWindowPosition(SDL_Window*, int, int)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    /* Preventing the game to change the window position */
}

/* Override */ void SDL_GetWindowPosition(SDL_Window *, int *x, int *y)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    /* Always simulate the game window being on top-left corner, so that games
     * using global mouse coords do not desync on different window positions.
     */
    *x = 0;
    *y = 0;
}

/* Override */ void SDL_SetWindowSize(SDL_Window* window, int w, int h)
{
    LINK_NAMESPACE_SDL2(SDL_SetWindowSize);

    if (GlobalState::isNative()) {
        NATIVECALL(orig::SDL_SetWindowSize(window, w, h));
        return;
    }

    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with new size: %d x %d", __func__, w, h);

    /* Ignored if game window is fullscreen */
    if (windowFullscreen)
        return;
        
    NATIVECALL(orig::SDL_SetWindowSize(window, w, h));

    ScreenCapture::resize(w, h);
}

/* SDL 1.2 */
/* Override */ SDL1::SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags)
{
    LINK_NAMESPACE_SDL1(SDL_SetVideoMode);

    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with size (%d,%d), bpp %d and flags %x", __func__, width, height, bpp, flags);

    ThreadManager::setMainThread();

    /* Disable fullscreen */
    windowFullscreen = (flags & /*SDL_FULLSCREEN*/ 0x80000000);
    flags &= (0xFFFFFFFF ^ /*SDL_FULLSCREEN*/ 0x80000000);

    /* The game may call SDL_SetVideoMode() multiple times to resize the game
     * window, which invalidates all previous textures, so we destroy all our
     * textures before this call and so we call recreate valid ones after.
     */
    ScreenCapture::fini();

    /* Call real function, but do not return yet */
    SDL1::SDL_Surface *surf = orig::SDL_SetVideoMode(width, height, bpp, flags);

    if (flags & /*SDL_OPENGL*/ 0x00000002) {
        Global::game_info.video |= GameInfo::OPENGL;
        Global::game_info.tosend = true;
    }
    else {
        Global::game_info.video &= ~GameInfo::OPENGL;
    }

    /* If we are going to save the screen when savestating, we need to init
     * our pixel access routine */
    ScreenCapture::init();

    SDL1::SDL_Event event;
    event.type = SDL1::SDL_ACTIVEEVENT;
    event.active.gain = 1;
    event.active.state = 0x7;
    sdlEventQueue.insert(&event);

    return surf;
}

/* Override */ int SDL_SetColorKey(SDL_Surface *surface, int flag, Uint32 key)
{
    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with flag %d and key %d", __func__, flag, key);
    LINK_NAMESPACE_SDLX(SDL_SetColorKey);
    return orig::SDL_SetColorKey(surface, flag, key);
}

/* Override */ int SDL_Flip(SDL1::SDL_Surface *screen)
{
    LINK_NAMESPACE_SDL1(SDL_Flip);

    if (GlobalState::isNative())
        return orig::SDL_Flip(screen);

    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);

    /* Start the frame boundary and pass the function to draw */
    static RenderHUD_SDL1 renderHUD;
    frameBoundary([&] () {orig::SDL_Flip(screen);}, renderHUD);

    return 0;
}

OVERRIDE void SDL_UpdateRects(SDL1::SDL_Surface *screen, int numrects, SDL1::SDL_Rect *rects)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_SDL1(SDL_UpdateRects);
        return orig::SDL_UpdateRects(screen, numrects, rects);
    }

    LINK_NAMESPACE_SDL1(SDL_UpdateRect);
    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with %d rects", __func__, numrects);

    /* Start the frame boundary and pass the function to draw */
    static RenderHUD_SDL1 renderHUD;
    frameBoundary([&] () {orig::SDL_UpdateRect(screen, 0, 0, 0, 0);}, renderHUD);
}

/* Override */ void SDL_UpdateRect(SDL1::SDL_Surface *screen, Sint32 x, Sint32 y, Uint32 w, Uint32 h)
{
    LINK_NAMESPACE_SDL1(SDL_UpdateRect);

    if (GlobalState::isNative())
        return orig::SDL_UpdateRect(screen, x, y, w, h);

    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with pos (%d,%d) and size (%u,%u)", __func__, x, y, w, h);

    /* Start the frame boundary and pass the function to draw */
    static RenderHUD_SDL1 renderHUD;
    frameBoundary([&] () {orig::SDL_UpdateRect(screen, 0, 0, 0, 0);}, renderHUD);
}

/* Override */ SDL1::SDL_GrabMode SDL_WM_GrabInput(SDL1::SDL_GrabMode mode)
{
    debuglogstdio(LCF_SDL | LCF_KEYBOARD | LCF_MOUSE | LCF_WINDOW, "%s call with mode %d", __func__, mode);
    static SDL1::SDL_GrabMode fakeGrab = SDL1::SDL_GRAB_OFF;
    if (mode != SDL1::SDL_GRAB_QUERY)
        fakeGrab = mode;
    return fakeGrab;
}


/* Override */ int SDL_GL_SetAttribute(SDL_GLattr attr, int value)
{
    debuglogstdio(LCF_SDL | LCF_OGL | LCF_WINDOW, "%s call with attr %d and value %d", __func__, attr, value);
    LINK_NAMESPACE_SDL2(SDL_GL_SetAttribute);

    switch (attr) {
    case SDL_GL_CONTEXT_MAJOR_VERSION:
        Global::game_info.opengl_major = value;
        Global::game_info.tosend = true;
        break;
    case SDL_GL_CONTEXT_MINOR_VERSION:
        Global::game_info.opengl_minor = value;
        Global::game_info.tosend = true;
        break;
    case SDL_GL_CONTEXT_PROFILE_MASK:
        switch (value) {
        case SDL_GL_CONTEXT_PROFILE_CORE:
            Global::game_info.opengl_profile = GameInfo::CORE;
            break;
        case SDL_GL_CONTEXT_PROFILE_COMPATIBILITY:
            Global::game_info.opengl_profile = GameInfo::COMPATIBILITY;
            break;
        case SDL_GL_CONTEXT_PROFILE_ES:
            Global::game_info.opengl_profile = GameInfo::ES;
            break;
        default:
            break;
        }
        Global::game_info.tosend = true;
        break;
    default:
        break;
    }

    return orig::SDL_GL_SetAttribute(attr, value);
}

/* Override */ int SDL_UpdateWindowSurface(SDL_Window * window)
{
    LINK_NAMESPACE_SDL2(SDL_UpdateWindowSurface);

    if (GlobalState::isNative())
        return orig::SDL_UpdateWindowSurface(window);

    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    Global::game_info.video |= GameInfo::SDL2_SURFACE;
    /* We can't guess that the game will use SDL2 surface before updating it.
     * so we initialize our screen capture here. */
    ScreenCapture::init();

    /* Start the frame boundary and pass the function to draw */
    static RenderHUD_SDL2_surface renderHUD;
    frameBoundary([&] () {orig::SDL_UpdateWindowSurface(window);}, renderHUD);

    return 0;
}

/* Override */ int SDL_UpdateWindowSurfaceRects(SDL_Window * window, const SDL_Rect * rects, int numrects)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_SDL2(SDL_UpdateWindowSurfaceRects);
        return orig::SDL_UpdateWindowSurfaceRects(window, rects, numrects);
    }

    LINK_NAMESPACE_SDL2(SDL_UpdateWindowSurface);
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    Global::game_info.video |= GameInfo::SDL2_SURFACE;
    /* We can't guess that the game will use SDL2 surface before updating it.
     * so we initialize our screen capture here. */
    ScreenCapture::init();

    /* Start the frame boundary and pass the function to draw */
    static RenderHUD_SDL2_surface renderHUD;
    frameBoundary([&] () {orig::SDL_UpdateWindowSurface(window);}, renderHUD);

    return 0;
}


}
