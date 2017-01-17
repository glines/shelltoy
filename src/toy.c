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
#include <string.h>

#include "plugin.h"
#include "toy.h"

struct st_Toy_Internal_ {
  const st_Toy_Dispatch *dispatch;
};

void st_Toy_init(
    st_Toy *self,
    const char *name,
    const st_Toy_Dispatch *dispatch)
{
  /* Copy the name string of this toy */
  self->name = (const char *)malloc(strlen(name) + 1);
  strcpy((char *)self->name, name);
  /* Initialize memory for internal data structures */
  self->internal = (st_Toy_Internal *)malloc(sizeof(st_Toy_Internal));
  /* Store pointer to the dispatch table */
  self->internal->dispatch = dispatch;
}

void st_Toy_destroy(
    st_Toy *self)
{
  /* Free memory for internal data structures */
  free(self->internal);
  /* Free memory for the name that we copied */
  free((char *)self->name);
}

st_BackgroundRenderer *
st_Toy_getBackgroundRenderer(
    st_Toy *self)
{
  if (self->internal->dispatch->getBackgroundRenderer == NULL) {
    return NULL;
  }
  return self->internal->dispatch->getBackgroundRenderer(self);
}
