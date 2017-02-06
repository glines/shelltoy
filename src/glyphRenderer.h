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

#ifndef ST_GLYPH_RENDERER_H_
#define ST_GLYPH_RENDERER_H_

#include <ft2build.h>
#include FT_FREETYPE_H

#include <inttypes.h>

#include "profile.h"

struct st_GlyphRenderer_Internal;

/** A wrapper around the glyph rendering faciliies of FreeType. This class
 * combines font faces from multiple sources, including boldface and asian
 * fonts, and renders any given character with the appropriate font face.
 */
typedef struct st_GlyphRenderer_ {
  struct st_GlyphRenderer_Internal *internal;
} st_GlyphRenderer;

void st_GlyphRenderer_init(
    st_GlyphRenderer *self,
    st_Profile *profile);

void st_GlyphRenderer_destroy(
    st_GlyphRenderer *self);

void st_GlyphRenderer_getCellSize(
    const st_GlyphRenderer *self,
    int *width, int *height);

int st_GlyphRenderer_getGlyphDimensions(
    st_GlyphRenderer *self,
    uint32_t character,
    int *width, int *height);

FT_Bitmap *st_GlyphRenderer_renderGlyph(
    st_GlyphRenderer *self,
    uint32_t character);

void st_GlyphRenderer_getGlyphOffset(
    st_GlyphRenderer *self,
    uint32_t character,
    int *x, int *y);

void st_GlyphRenderer_getUnderlineOffset(
    st_GlyphRenderer *self,
    int *offset);

st_ErrorCode
st_GlyphRenderer_loadFont(
    st_GlyphRenderer *self,
    const char *fontPath,
    float fontSize);

#endif
