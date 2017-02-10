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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"

/* Fit the array in at most two cache lines? Ehh... */
#define ST_INIT_ARRAY_SIZE 64 / sizeof(void *)

struct st_Array_Internal_ {
  void **values;
  size_t sizeValues, numValues;
};

void st_Array_init(
    st_Array *self)
{
  /* Initialize internal memory */
  self->internal = (st_Array_Internal *)malloc(sizeof(st_Array_Internal));
  self->internal->values = (void **)malloc(
      sizeof(void *) * ST_INIT_ARRAY_SIZE);
  self->internal->sizeValues = ST_INIT_ARRAY_SIZE;
  self->internal->numValues = 0;
}

void st_Array_destroy(
    st_Array *self)
{
  /* Free internal memory */
  free(self->internal->values);
  free(self->internal);
}

st_ErrorCode
st_Array_append(
    st_Array *self,
    void *value)
{
  /* Ensure we have enough memory to hold an additional value */
  if (self->internal->numValues + 1 > self->internal->sizeValues) {
    void **newValues;
    /* Double the size of our internal memory */
    newValues = (void **)malloc(self->internal->sizeValues * 2);
    if (newValues == NULL) {
      return ST_ERROR_OUT_OF_MEMORY;
    }
    memcpy(
        newValues,
        self->internal->values,
        sizeof(void *) * self->internal->numValues);
    free(self->internal->values);
    self->internal->values = newValues;
    self->internal->sizeValues *= 2;
  }

  /* Add the new value to the end of the values array */
  self->internal->values[self->internal->numValues++] = value;

  return ST_NO_ERROR;
}

void *st_Array_get(
    st_Array *self,
    size_t index)
{
  assert(index < self->internal->numValues);

  return self->internal->values[index];
}

size_t st_Array_size(
    const st_Array *self)
{
  return self->internal->numValues;
}

void st_Array_clear(
    st_Array *self)
{
  self->internal->numValues = 0;
}
