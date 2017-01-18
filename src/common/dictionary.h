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

#ifndef SHELLTOY_COMMON_DICTIONARY_H_
#define SHELLTOY_COMMON_DICTIONARY_H_

#include <stddef.h>

struct st_Dictionary_Internal_;
typedef struct st_Dictionary_Internal_ st_Dictionary_Internal;

typedef struct st_Dictionary_ {
  st_Dictionary_Internal *internal;
} st_Dictionary;

void st_Dictionary_init(
    st_Dictionary *self);

void st_Dictionary_destroy(
    st_Dictionary *self);

void st_Dictionary_insert(
    st_Dictionary *self,
    const char *key,
    void *value);

void *st_Dictionary_getValue(
    st_Dictionary *self,
    const char *key);

size_t st_Dictionary_size(
    st_Dictionary *self);

void *st_Dictionary_getValueAtIndex(
    st_Dictionary *self,
    size_t index);

#define ST_DECLARE_DICTIONARY( \
    VALUE_TYPE) \
typedef struct st_ ## VALUE_TYPE ## Dictionary_ { \
  st_Dictionary base; \
} VALUE_TYPE ## Dictionary; \
void VALUE_TYPE ## Dictionary_init( \
    VALUE_TYPE ## Dictionary *self); \
void VALUE_TYPE ## Dictionary_destroy( \
    VALUE_TYPE ## Dictionary *self); \
void VALUE_TYPE ## Dictionary_insert( \
    VALUE_TYPE ## Dictionary *self, \
    const char *key, \
    VALUE_TYPE *value); \
VALUE_TYPE *VALUE_TYPE ## Dictionary_getValue( \
    VALUE_TYPE ## Dictionary *self, \
    const char *key); \
size_t VALUE_TYPE ## Dictionary_size( \
    VALUE_TYPE ## Dictionary *self); \
VALUE_TYPE *VALUE_TYPE ## Dictionary_getValueAtIndex( \
    VALUE_TYPE ## Dictionary *self, \
    size_t index);


#define ST_DEFINE_DICTIONARY( \
    VALUE_TYPE) \
  void VALUE_TYPE ## Dictionary_init( \
      VALUE_TYPE ## Dictionary *self) \
  { \
    st_Dictionary_init((st_Dictionary *)self); \
  } \
  void VALUE_TYPE ## Dictionary_destroy( \
      VALUE_TYPE ## Dictionary *self) \
  { \
    st_Dictionary_destroy((st_Dictionary *)self); \
  } \
  void VALUE_TYPE ## Dictionary_insert( \
      VALUE_TYPE ## Dictionary *self, \
      const char *key, \
      VALUE_TYPE *value) \
  { \
    st_Dictionary_insert( \
        (st_Dictionary *)self, \
        key, \
        (void *)value); \
  } \
  VALUE_TYPE *VALUE_TYPE ## Dictionary_getValue( \
      VALUE_TYPE ## Dictionary *self, \
      const char *key) \
  { \
    return (VALUE_TYPE *) \
      st_Dictionary_getValue( \
        (st_Dictionary *)self, \
        key); \
  } \
  size_t VALUE_TYPE ## Dictionary_size( \
      VALUE_TYPE ## Dictionary *self) \
  { \
    return st_Dictionary_size((st_Dictionary *)self); \
  } \
  VALUE_TYPE *VALUE_TYPE ## Dictionary_getValueAtIndex( \
      VALUE_TYPE ## Dictionary *self, \
      size_t index) \
  { \
    return (VALUE_TYPE *)st_Dictionary_getValueAtIndex( \
        (st_Dictionary *)self, \
        index); \
  }

#endif
