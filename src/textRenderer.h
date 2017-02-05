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

#ifndef ST_TEXT_RENDERER_H_
#define ST_TEXT_RENDERER_H_

#include <libtsm.h>

#include "glyphAtlas.h"
#include "profile.h"

struct st_TextRenderer_Internal;

typedef struct st_TextRenderer_ {
  struct st_TextRenderer_Internal *internal;
} st_TextRenderer;

void st_TextRenderer_init(
    st_TextRenderer *self,
    st_GlyphRenderer *glyphRenderer,
    st_Profile *profile);

void st_TextRenderer_destroy(
    st_TextRenderer *self);

void st_TextRenderer_updateScreen(
    st_TextRenderer *self,
    struct tsm_screen *screen,
    st_GlyphRenderer *glyphRenderer);

void st_TextRenderer_draw(
    const st_TextRenderer *self,
    int cellWidth,
    int cellHeight,
    int viewportWidth,
    int viewportHeight);

#endif
