/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef WINDOWS_H_INCL
#define WINDOWS_H_INCL

#include "global.h"
#include "../external/SDL.h"

typedef void SDL_Window;

extern void* gameWindow;
extern Uint32 (*SDL_GetWindowID_real)(SDL_Window*);

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
OVERRIDE Uint32 SDL_GetWindowID(void* SDL_Window);

/**
 *  \brief Get the window flags.
 */
OVERRIDE Uint32 SDL_GetWindowFlags(void* SDL_Window);

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

OVERRIDE SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags);
OVERRIDE void SDL_GL_SwapBuffers(void);

void link_sdlwindows(void);

#endif

