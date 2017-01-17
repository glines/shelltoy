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

#ifndef SHELLTOY_TOY_H_
#define SHELLTOY_TOY_H_

#include <jansson.h>

#include "backgroundRenderer.h"
#include "error.h"

struct st_Toy_Internal_;
typedef struct st_Toy_Internal_ st_Toy_Internal;

struct st_Toy_;
typedef struct st_Toy_ st_Toy;
struct st_Toy_ {
  const char *name;

  st_Toy_Internal *internal;
};

void st_Toy_init(
    st_Toy *self,
    const st_Toy_Dispatch *dispatch);

void st_Toy_destroy(
    st_Toy *self);

st_BackgroundRenderer *
st_Toy_getBackgroundRenderer(
    st_Toy *self);

#endif
