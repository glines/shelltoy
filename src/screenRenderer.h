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

#ifndef ST_SCREEN_RENDERER_H_
#define ST_SCREEN_RENDERER_H_

#include <libtsm.h>

#include "glyphAtlas.h"

#define ST_SCREEN_RENDERER_INIT_SIZE_GLYPHS (80 * 24 * 2)

struct st_ScreenRenderer_Internal;

typedef struct st_ScreenRenderer_ {
  /* FIXME: Probably move atlas and glyphRenderer to an internal data
   * structure */
  st_GlyphAtlas atlas;
  st_GlyphRenderer glyphRenderer;

  struct st_ScreenRenderer_Internal *internal;
} st_ScreenRenderer;

void st_ScreenRenderer_init(
    st_ScreenRenderer *self);

void st_ScreenRenderer_destroy(
    st_ScreenRenderer *self);

void st_ScreenRenderer_updateScreen(
    st_ScreenRenderer *self,
    struct tsm_screen *screen);
void st_ScreenRenderer_draw(
    const st_ScreenRenderer *self,
    int viewportWidth,
    int viewportHeight);

#endif
