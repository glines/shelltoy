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

#include <stdlib.h>

#include "backgroundRenderer.h"

struct st_BackgroundRenderer_Internal_ {
  int foo;
};

void st_BackgroundRenderer_init(
    st_BackgroundRenderer *self)
{
  /* Allocate memory for internal data structures */
  self->internal = (st_BackgroundRenderer_Internal *)malloc(
      sizeof(st_BackgroundRenderer_Internal));
}

void st_BackgroundRenderer_destroy(
    st_BackgroundRenderer *self)
{
  free(self->internal);
}

void st_BackgroundRenderer_draw(
    const st_BackgroundRenderer *self,
    int viewportWidth,
    int viewportHeight)
{
  /* TODO: Replace this with a virtual call to draw? */
}
