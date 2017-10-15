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

#ifndef TTOY_FONTS_H_
#define TTOY_FONTS_H_

#include <ft2build.h>
#include FT_FREETYPE_H

#include <fontconfig/fontconfig.h>
#include <inttypes.h>

typedef struct {
  int w, h, x, y;
} ttoy_AtlasPos;

typedef struct {
  ttoy_AtlasPos aaPos, sdfPos;
  int xOffset, yOffset;
  int width, height;
  int halfwidth;
  uint32_t ch;
} ttoy_MonospaceGlyph;

typedef struct {
  int glyphWidth, glyphHeight;
  unsigned char *atlas_aa;  /* TODO: Use GL buffer? */
  float *atlas_sdf;  /* TODO: Use GL buffer? */
  ttoy_MonospaceGlyph *glyphs;
  size_t numGlyphs, sizeGlyphs;
} ttoy_MonospaceFontFace;

/*
typedef struct {
  ttoy_AtlasPos position;
} ttoy_GlyphAtlas;

*/
/**
 * Fonts used in ttoy are loaded by a central facility.
 */
void ttoy_Fonts_init();
void ttoy_Fonts_destroy();

FT_Library ttoy_Fonts_getFreeTypeInstance();
FcConfig *ttoy_Fonts_getFontconfigInstance();

ttoy_MonospaceFontFace *ttoy_Fonts_loadMonospace(
    int width, int height,
    const char *fontPath);
void ttoy_printGlyphDebug(const FT_Bitmap *bitmap);
void ttoy_printAntiAliasedGlyphDebug(const FT_Bitmap *bitmap);

void ttoy_MonospaceFontFace_init(ttoy_MonospaceFontFace *self);
void ttoy_MonospaceFontFace_loadGlyph(
    ttoy_MonospaceFontFace *self,
    FT_Face face,
    uint32_t ch);
ttoy_MonospaceGlyph *ttoy_MonospaceFontFace_getGlyph(
    ttoy_MonospaceFontFace *self,
    uint32_t ch);

#endif
