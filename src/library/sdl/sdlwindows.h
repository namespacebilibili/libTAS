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

#ifndef LIBTAS_SDLWINDOWS_H_INCL
#define LIBTAS_SDLWINDOWS_H_INCL

#include "../hook.h"
#include "../../external/SDL1.h"
#include <SDL2/SDL.h>

namespace libtas {

namespace sdl {
    /* SDL2 game window (we suppose there is only one) */
    extern SDL_Window* gameSDLWindow;    
}

/**
 * \brief Swap the OpenGL buffers for a window, if double-buffering is
 *        supported.
 */
OVERRIDE void SDL_GL_SwapWindow(SDL_Window* window);

/**
 *  \brief Create a window with the specified position, dimensions, and flags.
 *
 *  \param title The title of the window, in UTF-8 encoding.
 *  \param x     The x position of the window, ::SDL_WINDOWPOS_CENTERED, or
 *               ::SDL_WINDOWPOS_UNDEFINED.
 *  \param y     The y position of the window, ::SDL_WINDOWPOS_CENTERED, or
 *               ::SDL_WINDOWPOS_UNDEFINED.
 *  \param w     The width of the window, in screen coordinates.
 *  \param h     The height of the window, in screen coordinates.
 *  \param flags The flags for the window, a mask of any of the following:
 *               ::SDL_WINDOW_FULLSCREEN,    ::SDL_WINDOW_OPENGL,
 *               ::SDL_WINDOW_HIDDEN,        ::SDL_WINDOW_BORDERLESS,
 *               ::SDL_WINDOW_RESIZABLE,     ::SDL_WINDOW_MAXIMIZED,
 *               ::SDL_WINDOW_MINIMIZED,     ::SDL_WINDOW_INPUT_GRABBED,
 *               ::SDL_WINDOW_ALLOW_HIGHDPI.
 *
 *  \return The id of the window created, or zero if window creation failed.
 *
 *  If the window is created with the SDL_WINDOW_ALLOW_HIGHDPI flag, its size
 *  in pixels may differ from its size in screen coordinates on platforms with
 *  high-DPI support (e.g. iOS and Mac OS X). Use SDL_GetWindowSize() to query
 *  the client area's size in screen coordinates, and SDL_GL_GetDrawableSize()
 *  or SDL_GetRendererOutputSize() to query the drawable size in pixels.
 *
 *  \sa SDL_DestroyWindow()
 */
OVERRIDE SDL_Window* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags);

/**
 *  \brief Get the numeric ID of a window, for logging purposes.
 */
OVERRIDE Uint32 SDL_GetWindowID(SDL_Window* window);

/**
 *  \brief Get a window from a stored ID, or NULL if it doesn't exist.
 */
OVERRIDE SDL_Window* SDL_GetWindowFromID(Uint32 id);

/**
 *  \brief Get the window flags.
 */
OVERRIDE Uint32 SDL_GetWindowFlags(SDL_Window* window);

/**
 *  \brief Set the title of a window, in UTF-8 format.
 *
 *  \sa SDL_GetWindowTitle()
 */
OVERRIDE void SDL_SetWindowTitle(SDL_Window * window, const char *title);

/** SDL 1.2
 * Sets the title and icon text of the display window (UTF-8 encoded)
 */
OVERRIDE void SDL_WM_SetCaption(const char *title, const char *icon);

/**
 *  \brief Set the border state of a window.
 *
 *  This will add or remove the window's SDL_WINDOW_BORDERLESS flag and
 *  add or remove the border from the actual window. This is a no-op if the
 *  window's border already matches the requested state.
 *
 *  \param window The window of which to change the border state.
 *  \param bordered SDL_FALSE to remove border, SDL_TRUE to add border.
 *
 *  \note You can't change the border state of a fullscreen window.
 *
 *  \sa SDL_GetWindowFlags()
 */
OVERRIDE void SDL_SetWindowBordered(SDL_Window * window, SDL_bool bordered);

/**
 *  \brief Set a window's fullscreen state.
 *
 *  \return 0 on success, or -1 if setting the display mode failed.
 *
 *  \sa SDL_SetWindowDisplayMode()
 *  \sa SDL_GetWindowDisplayMode()
 */
OVERRIDE int SDL_SetWindowFullscreen(SDL_Window * window, Uint32 flags);

/**
 *  \brief Create an OpenGL context for use with an OpenGL window, and make it
 *         current.
 *
 *  \sa SDL_GL_DeleteContext()
 */
OVERRIDE void* SDL_GL_CreateContext(SDL_Window *window);

/**
 *  \brief Delete an OpenGL context.
 *
 *  \sa SDL_GL_CreateContext()
 */
OVERRIDE void SDL_GL_DeleteContext(SDL_GLContext context);

/**
 *  \brief Set the swap interval for the current OpenGL context.
 *
 *  \param interval 0 for immediate updates, 1 for updates synchronized with the
 *                  vertical retrace. If the system supports it, you may
 *                  specify -1 to allow late swaps to happen immediately
 *                  instead of waiting for the next retrace.
 *
 *  \return 0 on success, or -1 if setting the swap interval is not supported.
 *
 *  \sa SDL_GL_GetSwapInterval()
 */
OVERRIDE int SDL_GL_SetSwapInterval(int interval);

/**
 *  \brief Get the swap interval for the current OpenGL context.
 *
 *  \return 0 if there is no vertical retrace synchronization, 1 if the buffer
 *          swap is synchronized with the vertical retrace, and -1 if late
 *          swaps happen immediately instead of waiting for the next retrace.
 *          If the system can't determine the swap interval, or there isn't a
 *          valid current context, this will return 0 as a safe default.
 *
 *  \sa SDL_GL_SetSwapInterval()
 */
OVERRIDE int SDL_GL_GetSwapInterval(void);

/**
 *  \brief Destroy a window.
 */
OVERRIDE void SDL_DestroyWindow(SDL_Window* window);

/**
 *  \brief Create a window and default renderer
 *
 *  \param width    The width of the window
 *  \param height   The height of the window
 *  \param window_flags The flags used to create the window
 *  \param window   A pointer filled with the window, or NULL on error
 *  \param renderer A pointer filled with the renderer, or NULL on error
 *
 *  \return 0 on success, or -1 on error
 */
OVERRIDE int SDL_CreateWindowAndRenderer(
                 int width, int height, Uint32 window_flags,
                 SDL_Window **window, SDL_Renderer **renderer);

/**
 *  \brief Set the position of a window.
 *
 *  \param window   The window to reposition.
 *  \param x        The x coordinate of the window in screen coordinates, or
 *                  ::SDL_WINDOWPOS_CENTERED or ::SDL_WINDOWPOS_UNDEFINED.
 *  \param y        The y coordinate of the window in screen coordinates, or
 *                  ::SDL_WINDOWPOS_CENTERED or ::SDL_WINDOWPOS_UNDEFINED.
 *
 *  \note The window coordinate origin is the upper left of the display.
 *
 *  \sa SDL_GetWindowPosition()
 */
OVERRIDE void SDL_SetWindowPosition(SDL_Window * window, int x, int y);

/**
 *  \brief Get the position of a window.
 *
 *  \param window   The window to query.
 *  \param x        Pointer to variable for storing the x position, in screen
 *                  coordinates. May be NULL.
 *  \param y        Pointer to variable for storing the y position, in screen
 *                  coordinates. May be NULL.
 *
 *  \sa SDL_SetWindowPosition()
 */
OVERRIDE void SDL_GetWindowPosition(SDL_Window * window, int *x, int *y);

/**
 *  \brief Set the size of a window's client area.
 *
 *  \param window   The window to resize.
 *  \param w        The width of the window, in screen coordinates. Must be >0.
 *  \param h        The height of the window, in screen coordinates. Must be >0.
 *
 *  \note Fullscreen windows automatically match the size of the display mode,
 *        and you should use SDL_SetWindowDisplayMode() to change their size.
 *
 *  The window size in screen coordinates may differ from the size in pixels, if
 *  the window was created with SDL_WINDOW_ALLOW_HIGHDPI on a platform with
 *  high-dpi support (e.g. iOS or OS X). Use SDL_GL_GetDrawableSize() or
 *  SDL_GetRendererOutputSize() to get the real client area size in pixels.
 *
 *  \sa SDL_GetWindowSize()
 *  \sa SDL_SetWindowDisplayMode()
 */
OVERRIDE void SDL_SetWindowSize(SDL_Window* window, int w, int h);

/**
 * Set up a video mode with the specified width, height and bits-per-pixel.
 *
 * If 'bpp' is 0, it is treated as the current display bits per pixel.
 *
 * If SDL_ANYFORMAT is set in 'flags', the SDL library will try to set the
 * requested bits-per-pixel, but will return whatever video pixel format is
 * available.  The default is to emulate the requested pixel format if it
 * is not natively available.
 *
 * If SDL_HWSURFACE is set in 'flags', the video surface will be placed in
 * video memory, if possible, and you may have to call SDL_LockSurface()
 * in order to access the raw framebuffer.  Otherwise, the video surface
 * will be created in system memory.
 *
 * If SDL_ASYNCBLIT is set in 'flags', SDL will try to perform rectangle
 * updates asynchronously, but you must always lock before accessing pixels.
 * SDL will wait for updates to complete before returning from the lock.
 *
 * If SDL_HWPALETTE is set in 'flags', the SDL library will guarantee
 * that the colors set by SDL_SetColors() will be the colors you get.
 * Otherwise, in 8-bit mode, SDL_SetColors() may not be able to set all
 * of the colors exactly the way they are requested, and you should look
 * at the video surface structure to determine the actual palette.
 * If SDL cannot guarantee that the colors you request can be set,
 * i.e. if the colormap is shared, then the video surface may be created
 * under emulation in system memory, overriding the SDL_HWSURFACE flag.
 *
 * If SDL_FULLSCREEN is set in 'flags', the SDL library will try to set
 * a fullscreen video mode.  The default is to create a windowed mode
 * if the current graphics system has a window manager.
 * If the SDL library is able to set a fullscreen video mode, this flag
 * will be set in the surface that is returned.
 *
 * If SDL_DOUBLEBUF is set in 'flags', the SDL library will try to set up
 * two surfaces in video memory and swap between them when you call
 * SDL_Flip().  This is usually slower than the normal single-buffering
 * scheme, but prevents "tearing" artifacts caused by modifying video
 * memory while the monitor is refreshing.  It should only be used by
 * applications that redraw the entire screen on every update.
 *
 * If SDL_RESIZABLE is set in 'flags', the SDL library will allow the
 * window manager, if any, to resize the window at runtime.  When this
 * occurs, SDL will send a SDL_VIDEORESIZE event to you application,
 * and you must respond to the event by re-calling SDL_SetVideoMode()
 * with the requested size (or another size that suits the application).
 *
 * If SDL_NOFRAME is set in 'flags', the SDL library will create a window
 * without any title bar or frame decoration.  Fullscreen video modes have
 * this flag set automatically.
 *
 * This function returns the video framebuffer surface, or NULL if it fails.
 *
 * If you rely on functionality provided by certain video flags, check the
 * flags of the returned surface to make sure that functionality is available.
 * SDL will fall back to reduced functionality if the exact flags you wanted
 * are not available.
 */
OVERRIDE SDL1::SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags);

/**
 * Swap the OpenGL buffers, if double-buffering is supported.
 */
OVERRIDE void SDL_GL_SwapBuffers(void);

/** @name SDL_Update Functions
 * These functions should not be called while 'screen' is locked.
 */
/*@{*/
/**
 * Makes sure the given list of rectangles is updated on the given screen.
 */
OVERRIDE void SDL_UpdateRects(SDL1::SDL_Surface *screen, int numrects, SDL1::SDL_Rect *rects);

/**
 * If 'x', 'y', 'w' and 'h' are all 0, SDL_UpdateRect will update the entire
 * screen.
 */
OVERRIDE void SDL_UpdateRect(SDL1::SDL_Surface *screen, Sint32 x, Sint32 y, Uint32 w, Uint32 h);
/*@}*/

/**
 * Sets the color key (transparent pixel) in a blittable surface.
 * If 'flag' is SDL_SRCCOLORKEY (optionally OR'd with SDL_RLEACCEL),
 * 'key' will be the transparent pixel in the source image of a blit.
 * SDL_RLEACCEL requests RLE acceleration for the surface if present,
 * and removes RLE acceleration if absent.
 * If 'flag' is 0, this function clears any current color key.
 * This function returns 0, or -1 if there was an error.
 */
OVERRIDE int SDL_SetColorKey(SDL_Surface *surface, int flag, Uint32 key);

/**
 * On hardware that supports double-buffering, this function sets up a flip
 * and returns.  The hardware will wait for vertical retrace, and then swap
 * video buffers before the next video surface blit or lock will return.
 * On hardware that doesn not support double-buffering, this is equivalent
 * to calling SDL_UpdateRect(screen, 0, 0, 0, 0);
 * The SDL_DOUBLEBUF flag must have been passed to SDL_SetVideoMode() when
 * setting the video mode for this function to perform hardware flipping.
 * This function returns 0 if successful, or -1 if there was an error.
 */
OVERRIDE int SDL_Flip(SDL1::SDL_Surface *screen);

/**
 * This function allows you to set and query the input grab state of
 * the application.  It returns the new input grab state.
 *
 * Grabbing means that the mouse is confined to the application window,
 * and nearly all keyboard input is passed directly to the application,
 * and not interpreted by a window manager, if any.
 */
OVERRIDE SDL1::SDL_GrabMode SDL_WM_GrabInput(SDL1::SDL_GrabMode mode);

/**
 *  \brief Set an OpenGL window attribute before window creation.
 */
OVERRIDE int SDL_GL_SetAttribute(SDL_GLattr attr, int value);

/**
 *  \brief Copy the window surface to the screen.
 *
 *  \return 0 on success, or -1 on error.
 */
OVERRIDE int SDL_UpdateWindowSurface(SDL_Window * window);

/**
 *  \brief Copy a number of rectangles on the window surface to the screen.
 *
 *  \return 0 on success, or -1 on error.
 */
OVERRIDE int SDL_UpdateWindowSurfaceRects(SDL_Window * window,
                                                         const SDL_Rect * rects,
                                                         int numrects);


}

#endif
