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

#ifndef TTOY_GLYPH_RENDERER_H_
#define TTOY_GLYPH_RENDERER_H_

#include <ft2build.h>
#include FT_FREETYPE_H

#include <inttypes.h>

#include <ttoy/error.h>

#include "font.h"
#include "profile.h"

struct ttoy_GlyphRenderer_Internal;

/** A wrapper around the glyph rendering faciliies of FreeType. This class
 * combines font faces from multiple sources, including boldface and asian
 * fonts, and renders any given character with the appropriate font face.
 */
typedef struct ttoy_GlyphRenderer_ {
  struct ttoy_GlyphRenderer_Internal *internal;
} ttoy_GlyphRenderer;

void ttoy_GlyphRenderer_init(
    ttoy_GlyphRenderer *self,
    ttoy_Profile *profile);

void ttoy_GlyphRenderer_destroy(
    ttoy_GlyphRenderer *self);

void ttoy_GlyphRenderer_getCellSize(
    const ttoy_GlyphRenderer *self,
    int *width, int *height);

void ttoy_GlyphRenderer_getUnderlineOffset(
    ttoy_GlyphRenderer *self,
    int *offset);

ttoy_ErrorCode
ttoy_GlyphRenderer_getFont(
    ttoy_GlyphRenderer *self,
    uint32_t character,
    int bold,
    ttoy_Font **font,
    int *fontIndex);

/**
 * Convenience function to get the font index for a glyph without needing to
 * provide storange for the font pointer.
 */
ttoy_ErrorCode
ttoy_GlyphRenderer_getFontIndex(
    ttoy_GlyphRenderer *self,
    uint32_t character,
    int bold,
    int *fontIndex);

ttoy_ErrorCode
ttoy_GlyphRenderer_getGlyphDimensions(
    ttoy_GlyphRenderer *self,
    uint32_t character,
    int bold,
    int *width,
    int *height);

ttoy_ErrorCode
ttoy_GlyphRenderer_getGlyphOffset(
    ttoy_GlyphRenderer *self,
    uint32_t character,
    int bold,
    int *x,
    int *y);

ttoy_ErrorCode
ttoy_GlyphRenderer_renderGlyph(
    ttoy_GlyphRenderer *self,
    uint32_t character,
    int bold,
    FT_Bitmap **bitmap,
    int *fontIndex);

#endif
