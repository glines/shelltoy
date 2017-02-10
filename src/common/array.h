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

#ifndef SHELLTOY_COMMON_ARRAY_H_
#define SHELLTOY_COMMON_ARRAY_H_

#include <shelltoy/error.h>

#include <stddef.h>

struct st_Array_Internal_;
typedef struct st_Array_Internal_ st_Array_Internal;

typedef struct st_Array_ {
  st_Array_Internal *internal;
} st_Array;

void st_Array_init(
    st_Array *self);

void st_Array_destroy(
    st_Array *self);

st_ErrorCode
st_Array_append(
    st_Array *self,
    void *value);

void *st_Array_get(
    st_Array *self,
    size_t index);

size_t st_Array_size(
    const st_Array *self);

void st_Array_clear(
    st_Array *self);

#define ST_DECLARE_ARRAY( \
    VALUE_TYPE) \
typedef struct st_ ## VALUE_TYPE ## Array_ { \
  st_Array base; \
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

#define ST_DEFINE_ARRAY( \
    VALUE_TYPE) \
void VALUE_TYPE ## Array_init( \
    VALUE_TYPE ## Array *self) \
{ \
  st_Array_init((st_Array *)self); \
} \
void VALUE_TYPE ## Array_destroy( \
    VALUE_TYPE ## Array *self) \
{ \
  st_Array_destroy((st_Array *)self); \
} \
void VALUE_TYPE ## Array_append( \
    VALUE_TYPE ## Array *self, \
    VALUE_TYPE *value) \
{ \
  st_Array_append((st_Array *)self, (void *)value); \
} \
VALUE_TYPE *VALUE_TYPE ## Array_get( \
    VALUE_TYPE ## Array *self, \
    size_t index) \
{ \
  return (VALUE_TYPE *)st_Array_get((st_Array *)self, index); \
} \
size_t VALUE_TYPE ## Array_size( \
    const VALUE_TYPE ## Array *self) \
{ \
  return st_Array_size((st_Array *)self); \
} \
void VALUE_TYPE ## Array_clear( \
    VALUE_TYPE ## Array *self) \
{ \
  st_Array_clear((st_Array *)self); \
}

#endif
