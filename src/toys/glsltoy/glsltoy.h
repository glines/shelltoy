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

#ifndef SHELLTOY_TOYS_GLSLTOY_H_
#define SHELLTOY_TOYS_GLSLTOY_H_

#include "../../toy.h"

struct st_GlslToy_Internal;

typedef struct st_GlslToy_ {
  st_Toy base;

  struct st_GlslToy_Internal *internal;
} st_GlslToy;

void st_GlslToy_init(
    st_GlslToy *self);

void st_GlslToy_destroy(
    st_GlslToy *self);

void st_GlslToy_drawBackground(
    const st_GlslToy *self,
    int width,
    int height);

/* Virtual methods */
st_ErrorCode
st_GlslToy_buildFromJson(
    st_GlslToy *self,
    json_t *config);

#endif
