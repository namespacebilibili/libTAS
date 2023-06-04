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

#include "sdlrender.h"
#include "sdlwindows.h"
#include "../hook.h"
#include "../logging.h"
#include "../frame.h"
#include "../renderhud/RenderHUD_SDL2_renderer.h"
#include "../ScreenCapture.h"
#include "../global.h"
#include "../GlobalState.h"

namespace libtas {

DECLARE_ORIG_POINTER(SDL_CreateRenderer)
DECLARE_ORIG_POINTER(SDL_DestroyRenderer)
DECLARE_ORIG_POINTER(SDL_RenderPresent)
DECLARE_ORIG_POINTER(SDL_RenderSetViewport)
DECLARE_ORIG_POINTER(SDL_RenderGetViewport)
DECLARE_ORIG_POINTER(SDL_RenderSetScale)
DECLARE_ORIG_POINTER(SDL_RenderGetScale)

/* Override */ SDL_Renderer *SDL_CreateRenderer(SDL_Window * window, int index, Uint32 flags)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    LINK_NAMESPACE_SDL2(SDL_CreateRenderer);

    if (flags & SDL_RENDERER_SOFTWARE)
        debuglogstdio(LCF_SDL | LCF_WINDOW, "  flag SDL_RENDERER_SOFTWARE");
    if (flags & SDL_RENDERER_ACCELERATED)
        debuglogstdio(LCF_SDL | LCF_WINDOW, "  flag SDL_RENDERER_ACCELERATED");
    if (flags & SDL_RENDERER_PRESENTVSYNC)
        debuglogstdio(LCF_SDL | LCF_WINDOW, "   flag SDL_RENDERER_PRESENTVSYNC");
    if (flags & SDL_RENDERER_TARGETTEXTURE)
        debuglogstdio(LCF_SDL | LCF_WINDOW, "   flag SDL_RENDERER_TARGETTEXTURE");

    Global::game_info.video |=  GameInfo::SDL2_RENDERER;

    SDL_Renderer* renderer = orig::SDL_CreateRenderer(window, index, flags);

    ScreenCapture::init();

    return renderer;
}

/* Override */ void SDL_DestroyRenderer(SDL_Renderer * renderer)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    LINK_NAMESPACE_SDL2(SDL_DestroyRenderer);

    ScreenCapture::fini();

    Global::game_info.video &= ~GameInfo::SDL2_RENDERER;

    orig::SDL_DestroyRenderer(renderer);
}

/* Override */ void SDL_RenderPresent(SDL_Renderer * renderer)
{
    LINK_NAMESPACE_SDL2(SDL_RenderPresent);

    if (GlobalState::isNative())
        return orig::SDL_RenderPresent(renderer);

    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);

    /* Start the frame boundary and pass the function to draw */
    static RenderHUD_SDL2_renderer renderHUD;
    renderHUD.setRenderer(renderer);
    frameBoundary([&] () {orig::SDL_RenderPresent(renderer);}, renderHUD);
}

static int logical_w = 0;
static int logical_h = 0;

int SDL_RenderSetLogicalSize(SDL_Renderer * renderer, int w, int h)
{
    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s called with new size: %d x %d", __func__, w, h);

    /* Don't let the game have logical size that differs from screen size,
     * so we resize the window instead.
     */
    SDL_SetWindowSize(sdl::gameSDLWindow, w, h);
    logical_w = w;
    logical_h = h;

    return 0;
}

void SDL_RenderGetLogicalSize(SDL_Renderer * renderer, int *w, int *h)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);

    /* Set the stored values of logical size */
    *w = logical_w;
    *h = logical_h;
}

int SDL_RenderSetViewport(SDL_Renderer * renderer, const SDL_Rect * rect)
{
    if (rect)
        debuglogstdio(LCF_SDL | LCF_WINDOW | LCF_TODO, " called with new size: %d x %d", __func__, rect->w, rect->h);
    else
        debuglogstdio(LCF_SDL | LCF_WINDOW | LCF_TODO, "%s called with native size", __func__);

    LINK_NAMESPACE_SDL2(SDL_RenderSetViewport);
    return orig::SDL_RenderSetViewport(renderer, rect);
}

void SDL_RenderGetViewport(SDL_Renderer * renderer, SDL_Rect * rect)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW | LCF_TODO);
    LINK_NAMESPACE_SDL2(SDL_RenderGetViewport);
    return orig::SDL_RenderGetViewport(renderer, rect);
}

int SDL_RenderSetScale(SDL_Renderer * renderer, float scaleX, float scaleY)
{
    debuglogstdio(LCF_SDL | LCF_WINDOW | LCF_TODO, "%s called with new scaleX: %d and scaleY: %d", __func__, scaleX, scaleY);
    LINK_NAMESPACE_SDL2(SDL_RenderSetScale);
    return orig::SDL_RenderSetScale(renderer, scaleX, scaleY);
}

void SDL_RenderGetScale(SDL_Renderer * renderer, float *scaleX, float *scaleY)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW | LCF_TODO);
    LINK_NAMESPACE_SDL2(SDL_RenderGetScale);
    return orig::SDL_RenderGetScale(renderer, scaleX, scaleY);
}

}
