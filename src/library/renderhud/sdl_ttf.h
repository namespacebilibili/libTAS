/*
  SDL_ttf:  A companion library to SDL for working with TrueType (tm) fonts
  Copyright (C) 2001-2016 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/* This library is a wrapper around the excellent FreeType 2.0 library,
   available at:
    http://www.freetype.org/
*/

/* Note: In many places, SDL_ttf will say "glyph" when it means "code point."
   Unicode is hard, we learn as we go, and we apologize for adding to the
   confusion. */

#ifndef _SDL_TTF_H
#define _SDL_TTF_H

#ifdef __unix__
#include "config.h"
#endif

#include "SurfaceARGB.h"
#include <memory>
#include <iostream>

/* Printable format: "%d.%d.%d", MAJOR, MINOR, PATCHLEVEL
*/
#define SDL_TTF_MAJOR_VERSION   2
#define SDL_TTF_MINOR_VERSION   0
#define SDL_TTF_PATCHLEVEL      14

/* ZERO WIDTH NO-BREAKSPACE (Unicode byte order mark) */
#define UNICODE_BOM_NATIVE  0xFEFF
#define UNICODE_BOM_SWAPPED 0xFFFE

namespace libtas {

/* This function tells the library whether UNICODE text is generally
   byteswapped.  A UNICODE BOM character in a string will override
   this setting for the remainder of that string.
*/
void TTF_ByteSwappedUNICODE(int swapped);

/* The internal structure containing font information */
typedef struct _TTF_Font TTF_Font;

/* Initialize the TTF engine - returns 0 if successful, -1 on error */
int TTF_Init(void);

/* Open a font file and create a font of the specified point size.
 * Some .fon fonts will have several sizes embedded in the file, so the
 * point size becomes the index of choosing which size.  If the value
 * is too high, the last indexed size will be the default. */
TTF_Font * TTF_OpenFont(const char *file, int ptsize);
TTF_Font * TTF_OpenFontIndex(const char *file, int ptsize, long index);
TTF_Font * TTF_OpenFontRW(FILE *src, int freesrc, int ptsize);
TTF_Font * TTF_OpenFontIndexRW(FILE *src, int freesrc, int ptsize, long index);

/* Set and retrieve the font style */
#define TTF_STYLE_NORMAL        0x00
#define TTF_STYLE_BOLD          0x01
#define TTF_STYLE_ITALIC        0x02
#define TTF_STYLE_UNDERLINE     0x04
#define TTF_STYLE_STRIKETHROUGH 0x08
int TTF_GetFontStyle(const TTF_Font *font);
void TTF_SetFontStyle(TTF_Font *font, int style);
int TTF_GetFontOutline(const TTF_Font *font);
void TTF_SetFontOutline(TTF_Font *font, int outline);

/* Set and retrieve FreeType hinter settings */
#define TTF_HINTING_NORMAL    0
#define TTF_HINTING_LIGHT     1
#define TTF_HINTING_MONO      2
#define TTF_HINTING_NONE      3
int TTF_GetFontHinting(const TTF_Font *font);
void TTF_SetFontHinting(TTF_Font *font, int hinting);

/* Get the total height of the font - usually equal to point size */
int TTF_FontHeight(const TTF_Font *font);

/* Get the offset from the baseline to the top of the font
   This is a positive value, relative to the baseline.
 */
int TTF_FontAscent(const TTF_Font *font);

/* Get the offset from the baseline to the bottom of the font
   This is a negative value, relative to the baseline.
 */
int TTF_FontDescent(const TTF_Font *font);

/* Get the recommended spacing between lines of text for this font */
int TTF_FontLineSkip(const TTF_Font *font);

/* Get/Set whether or not kerning is allowed for this font */
int TTF_GetFontKerning(const TTF_Font *font);
void TTF_SetFontKerning(TTF_Font *font, int allowed);

/* Get the number of faces of the font */
long TTF_FontFaces(const TTF_Font *font);

/* Get the font face attributes, if any */
int TTF_FontFaceIsFixedWidth(const TTF_Font *font);
char * TTF_FontFaceFamilyName(const TTF_Font *font);
char * TTF_FontFaceStyleName(const TTF_Font *font);

/* Check wether a glyph is provided by the font or not */
int TTF_GlyphIsProvided(const TTF_Font *font, uint16_t ch);

/* Get the metrics (dimensions) of a glyph
   To understand what these metrics mean, here is a useful link:
    http://freetype.sourceforge.net/freetype2/docs/tutorial/step2.html
 */
int TTF_GlyphMetrics(TTF_Font *font, uint16_t ch,
                     int *minx, int *maxx,
                                     int *miny, int *maxy, int *advance);

/* Get the dimensions of a rendered string of text */
int TTF_SizeText(TTF_Font *font, const char *text, int *w, int *h);
int TTF_SizeUTF8(TTF_Font *font, const char *text, int *w, int *h);
int TTF_SizeUNICODE(TTF_Font *font, const uint16_t *text, int *w, int *h);

/* Create a 32-bit ARGB surface and render the given text at high quality,
   using alpha blending to dither the font with the given color.
   This function returns the new surface, or NULL if there was an error.
*/
std::unique_ptr<SurfaceARGB> TTF_RenderText_Blended(TTF_Font *font,
                const char *text, Color fg);
std::unique_ptr<SurfaceARGB> TTF_RenderUTF8_Blended(TTF_Font *font,
                const char *text, Color fg);
std::unique_ptr<SurfaceARGB> TTF_RenderUNICODE_Blended(TTF_Font *font,
                const uint16_t *text, Color fg);


/* Create a 32-bit ARGB surface and render the given text at high quality,
   using alpha blending to dither the font with the given color.
   Text is wrapped to multiple lines on line endings and on word boundaries
   if it extends beyond wrapLength in pixels.
   This function returns the new surface, or NULL if there was an error.
*/
std::unique_ptr<SurfaceARGB> TTF_RenderText_Blended_Wrapped(TTF_Font *font,
                const char *text, Color fg, uint32_t wrapLength);
std::unique_ptr<SurfaceARGB> TTF_RenderUTF8_Blended_Wrapped(TTF_Font *font,
                const char *text, Color fg, uint32_t wrapLength);
std::unique_ptr<SurfaceARGB> TTF_RenderUNICODE_Blended_Wrapped(TTF_Font *font,
                const uint16_t *text, Color fg, uint32_t wrapLength);

/* Create a 32-bit ARGB surface and render the given glyph at high quality,
   using alpha blending to dither the font with the given color.
   The glyph is rendered without any padding or centering in the X
   direction, and aligned normally in the Y direction.
   This function returns the new surface, or NULL if there was an error.
*/
std::unique_ptr<SurfaceARGB> TTF_RenderGlyph_Blended(TTF_Font *font,
                        uint16_t ch, Color fg);

/* Close an opened font file */
void TTF_CloseFont(TTF_Font *font);

/* De-initialize the TTF engine */
void TTF_Quit(void);

/* Check if the TTF engine is initialized */
int TTF_WasInit(void);

/* Get the kerning size of two glyphs */
int TTF_GetFontKerningSizeGlyphs(TTF_Font *font, uint16_t previous_ch, uint16_t ch);

#define TTF_SetError(MSG)    std::cerr << MSG << std::endl;

}

#endif /* _SDL_TTF_H */
