/*
 * Copyright (c) 2016 Jonathan Glines
 * Jonathan Glines <jonathan@glines.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef SHELLTOY_FONTS_H_
#define SHELLTOY_FONTS_H_

#include <ft2build.h>
#include FT_FREETYPE_H

#include <inttypes.h>

typedef struct {
  int w, h, x, y;
} st_AtlasPos;

typedef struct {
  st_AtlasPos aaPos, sdfPos;
  int xOffset, yOffset;
  int width, height;
  int halfwidth;
  uint32_t ch;
} st_MonospaceGlyph;

typedef struct {
  int glyphWidth, glyphHeight;
  unsigned char *atlas_aa;  /* TODO: Use GL buffer? */
  float *atlas_sdf;  /* TODO: Use GL buffer? */
  st_MonospaceGlyph *glyphs;
  size_t numGlyphs, sizeGlyphs;
} st_MonospaceFontFace;

/*
typedef struct {
  st_AtlasPos position;
} st_GlyphAtlas;

*/
/**
 * Fonts used in Shelltoy are loaded by a central facility.
 */
void st_Fonts_init();
void st_Fonts_destroy();

void st_Fonts_initFreetype();

FT_Library st_Fonts_getFreeTypeInstance();

st_MonospaceFontFace *st_Fonts_loadMonospace(
    int width, int height,
    const char *fontPath);
void st_printGlyphDebug(const FT_Bitmap *bitmap);
void st_printAntiAliasedGlyphDebug(const FT_Bitmap *bitmap);

/*
void st_GlyphAtlas_addGlyph(
    st_GlyphAtlas *self,
    int width, int height, uint32_t ch);
void st_GlyphAtlas_packGlyphs(st_GlyphAtlas *self);
*/

void st_MonospaceFontFace_init(st_MonospaceFontFace *self);
void st_MonospaceFontFace_loadGlyph(
    st_MonospaceFontFace *self,
    FT_Face face,
    uint32_t ch);
st_MonospaceGlyph *st_MonospaceFontFace_getGlyph(
    st_MonospaceFontFace *self,
    uint32_t ch);

#endif
