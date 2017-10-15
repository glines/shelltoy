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

#include <stdlib.h>
#include <string.h>

#include <ttoy/backgroundToy.h>

struct ttoy_BackgroundToy_Internal_ {
  const ttoy_BackgroundToy_Dispatch *dispatch;
};

void ttoy_BackgroundToy_init(
    ttoy_BackgroundToy *self,
    const char *name,
    const ttoy_BackgroundToy_Dispatch *dispatch)
{
  /* NOTE: This is the only method that is not virtual, i.e. we are not
   * responsible for calling into the dispatch table here. FIXME: Clarify why
   * this is the case. We could probably actually pull this off... */
  /* Allocate memory for internal structures */
  self->internal = (ttoy_BackgroundToy_Internal *)malloc(
      sizeof(ttoy_BackgroundToy_Internal));
  /* Copy the name string */
  self->name = (const char *)malloc(strlen(name) + 1);
  strcpy((char *)self->name, name);
  /* Store pointer to the method dispatch table */
  self->internal->dispatch = dispatch;
}

void ttoy_BackgroundToy_destroy(
    ttoy_BackgroundToy *self)
{
  /* Call destroy for derived class through dispatch table */
  self->internal->dispatch->destroy(self);
  /* Free allocated memory */
  free((char *)self->name);
  free(self->internal);
}

void ttoy_BackgroundToy_draw(
    ttoy_BackgroundToy *self,
    int viewportWidth,
    int viewportHeight)
{
  /* Call draw for derived class through dispatch table */
  self->internal->dispatch->draw(self,
      viewportWidth,
      viewportHeight);
}
