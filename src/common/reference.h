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

#ifndef SHELLTOY_COMMON_REFERENCE_H_
#define SHELLTOY_COMMON_REFERENCE_H_

#include <stdlib.h>

#define ST_DECLARE_REFERENCE(TYPE) \
  typedef struct TYPE ## Ref_ { \
    TYPE base; \
    int refCount; \
  } TYPE ## Ref; \
  void TYPE ## Ref_init( \
      TYPE ## Ref **self); \
  TYPE *TYPE ## Ref_get( \
      TYPE ## Ref *self); \
  void TYPE ## Ref_increment( \
      TYPE ## Ref *self); \
  void TYPE ## Ref_decrement( \
      TYPE ## Ref *self);

#define ST_DEFINE_REFERENCE(TYPE) \
  void TYPE ## Ref_init( \
      TYPE ## Ref **self) \
  { \
    *self = (TYPE ## Ref *)malloc(sizeof(TYPE ## Ref)); \
    (*self)->refCount = 1; \
  } \
  TYPE *TYPE ## Ref_get( \
      TYPE ## Ref *self) \
  { \
    return &self->base; \
  } \
  void TYPE ## Ref_increment( \
      TYPE ## Ref *self) \
  { \
    self->refCount += 1; \
  } \
  void TYPE ## Ref_decrement( \
      TYPE ## Ref *self) \
  { \
    self->refCount -= 1; \
    if (self->refCount == 0) { \
      TYPE ## _destroy(&self->base); \
      free(self); \
    } \
  }

#endif
