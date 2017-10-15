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

#include <ttoy/textToy.h>

struct ttoy_TextToy_Internal_ {
  const ttoy_TextToy_Dispatch *dispatch;
};

void ttoy_TextToy_init(
    ttoy_TextToy *self,
    const char *name,
    const ttoy_TextToy_Dispatch *dispatch)
{
  /* FIXME: Change this to call the derived init method? */
  /* Allocate memory for internal structures */
  self->internal = (ttoy_TextToy_Internal *)malloc(
      sizeof(ttoy_TextToy_Internal));
  /* Copy the name string */
  self->name = (const char *)malloc(strlen(name) + 1);
  strcpy((char *)self->name, name);
}

void ttoy_TextToy_destroy(
    ttoy_TextToy *self)
{
  /* Call the derived destructor */
  self->internal->dispatch->destroy(self);
  /* Free allocated memory */
  free((char *)self->name);
  free(self->internal);
}

void ttoy_TextToy_draw(
    ttoy_TextToy *self,
    int viewportWidth,
    int viewportHeight)
{
  /* Call the derived draw method */
  self->internal->dispatch->draw(self,
      viewportWidth,  /* viewportWidth */
      viewportHeight  /* viewportHeight */
      );
}
