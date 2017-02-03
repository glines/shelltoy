/*
 * Copyright (c) 2017 Jonathan Glines
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

#ifndef SHELLTOY_BACKGROUND_RENDERER_H_
#define SHELLTOY_BACKGROUND_RENDERER_H_

#include "profile.h"

struct st_BackgroundRenderer_Internal_;
typedef struct st_BackgroundRenderer_Internal_ st_BackgroundRenderer_Internal;

typedef struct st_BackgroundRenderer_ {
  st_BackgroundRenderer_Internal *internal;
} st_BackgroundRenderer;

void st_BackgroundRenderer_init(
    st_BackgroundRenderer *self,
    st_Profile *profile);

void st_BackgroundRenderer_destroy(
    st_BackgroundRenderer *self);

void st_BackgroundRenderer_draw(
    st_BackgroundRenderer *self,
    int viewportWidth,
    int viewportHeight);

#endif
