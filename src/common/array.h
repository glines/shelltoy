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

#ifndef TTOY_COMMON_ARRAY_H_
#define TTOY_COMMON_ARRAY_H_

#include <ttoy/error.h>

#include <stddef.h>

struct ttoy_Array_Internal_;
typedef struct ttoy_Array_Internal_ ttoy_Array_Internal;

/* FIXME: Rename ttoy_Array to ttoy_List and then create a new
 * ttoy_Array that does not use so much pointer indirection. This will
 * require refactoring all code that currently uses ttoy_Array and
 * chosing the structure depending on their need for pointer
 * indirection. */
typedef struct ttoy_Array_ {
  ttoy_Array_Internal *internal;
} ttoy_Array;

void ttoy_Array_init(
    ttoy_Array *self);

void ttoy_Array_destroy(
    ttoy_Array *self);

ttoy_ErrorCode
ttoy_Array_append(
    ttoy_Array *self,
    void *value);

void *ttoy_Array_get(
    ttoy_Array *self,
    size_t index);

size_t ttoy_Array_size(
    const ttoy_Array *self);

void ttoy_Array_clear(
    ttoy_Array *self);

#define TTOY_DECLARE_ARRAY( \
    VALUE_TYPE) \
typedef struct ttoy_ ## VALUE_TYPE ## Array_ { \
  ttoy_Array base; \
} VALUE_TYPE ## Array; \
void VALUE_TYPE ## Array_init( \
    VALUE_TYPE ## Array *self); \
void VALUE_TYPE ## Array_destroy( \
    VALUE_TYPE ## Array *self); \
void VALUE_TYPE ## Array_append( \
    VALUE_TYPE ## Array *self, \
    VALUE_TYPE *value); \
VALUE_TYPE *VALUE_TYPE ## Array_get( \
    VALUE_TYPE ## Array *self, \
    size_t index); \
size_t VALUE_TYPE ## Array_size( \
    const VALUE_TYPE ## Array *self); \
void VALUE_TYPE ## Array_clear( \
    VALUE_TYPE ## Array *self);

#define TTOY_DEFINE_ARRAY( \
    VALUE_TYPE) \
void VALUE_TYPE ## Array_init( \
    VALUE_TYPE ## Array *self) \
{ \
  ttoy_Array_init((ttoy_Array *)self); \
} \
void VALUE_TYPE ## Array_destroy( \
    VALUE_TYPE ## Array *self) \
{ \
  ttoy_Array_destroy((ttoy_Array *)self); \
} \
void VALUE_TYPE ## Array_append( \
    VALUE_TYPE ## Array *self, \
    VALUE_TYPE *value) \
{ \
  ttoy_Array_append((ttoy_Array *)self, (void *)value); \
} \
VALUE_TYPE *VALUE_TYPE ## Array_get( \
    VALUE_TYPE ## Array *self, \
    size_t index) \
{ \
  return (VALUE_TYPE *)ttoy_Array_get((ttoy_Array *)self, index); \
} \
size_t VALUE_TYPE ## Array_size( \
    const VALUE_TYPE ## Array *self) \
{ \
  return ttoy_Array_size((ttoy_Array *)self); \
} \
void VALUE_TYPE ## Array_clear( \
    VALUE_TYPE ## Array *self) \
{ \
  ttoy_Array_clear((ttoy_Array *)self); \
}

#endif
