/*
 * Copyright (c) 2016-2017 Jonathan Glines
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

#ifndef TTOY_TEXT_RENDERER_H_
#define TTOY_TEXT_RENDERER_H_

#include <libtsm.h>

#include "glyphAtlas.h"
#include "glyphRendererRef.h"
#include "profile.h"

struct ttoy_TextRenderer_Internal;

typedef struct ttoy_TextRenderer_ {
  struct ttoy_TextRenderer_Internal *internal;
} ttoy_TextRenderer;

void ttoy_TextRenderer_init(
    ttoy_TextRenderer *self,
    ttoy_GlyphRendererRef *glyphRenderer,
    ttoy_Profile *profile);

void ttoy_TextRenderer_destroy(
    ttoy_TextRenderer *self);

void ttoy_TextRenderer_setGlyphRenderer(
    ttoy_TextRenderer *self,
    ttoy_GlyphRendererRef *glyphRenderer);

void ttoy_TextRenderer_updateScreen(
    ttoy_TextRenderer *self,
    struct tsm_screen *screen,
    int cellWidth,
    int cellHeight);

void ttoy_TextRenderer_draw(
    const ttoy_TextRenderer *self,
    int cellWidth,
    int cellHeight,
    int viewportWidth,
    int viewportHeight);

#endif
