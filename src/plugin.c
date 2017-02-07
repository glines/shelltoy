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

#include <shelltoy/plugin.h>

struct st_Plugin_Internal_ {
  const st_Plugin_Dispatch *dispatch;
  const st_BackgroundToy_Attributes *backgroundToyAttributes;
  const st_BackgroundToy_Dispatch *backgroundToyDispatch;
};

void
st_Plugin_init(
    st_Plugin *self,
    const char *name,
    const st_Plugin_Dispatch *dispatch,
    const st_BackgroundToy_Attributes *backgroundToyAttributes,
    const st_BackgroundToy_Dispatch *backgroundToyDispatch)
{
  /* Allocate memory for internal structures */
  self->internal = (st_Plugin_Internal *)malloc(
      sizeof(st_Plugin_Internal));
  /* Copy the name string */
  self->name = (const char *)malloc(strlen(name) + 1);
  strcpy((char *)self->name, name);
  /* Store pointers to the dispatch tables */
  self->internal->dispatch = dispatch;
  self->internal->backgroundToyDispatch = backgroundToyDispatch;
  /* Store pointer to the toy attributes */
  self->internal->backgroundToyAttributes = backgroundToyAttributes;
}

void
st_Plugin_destroy(
    st_Plugin *self)
{
  /* Free memory from internal structures */
  free(self->internal);
  free((char *)self->name);
}

const st_BackgroundToy_Attributes *
st_Plugin_getBackgroundToyAttributes(
    st_Plugin *self)
{
  return self->internal->backgroundToyAttributes;
}

const st_BackgroundToy_Dispatch *
st_Plugin_getBackgroundToyDispatch(
    st_Plugin *self)
{
  return self->internal->backgroundToyDispatch;
}
