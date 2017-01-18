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

#include "dictionary.h"

typedef struct st_Dictionary_KeyValuePair_ {
  const char *key;
  void *value;
} st_Dictionary_KeyValuePair;

struct st_Dictionary_Internal_ {
  st_Dictionary_KeyValuePair *pairs;
  size_t sizePairs, numPairs;
};

#define INIT_DICTIONARY_SIZE 8

void st_Dictionary_init(
    st_Dictionary *self)
{
  /* Allocate memory for internal data structures */
  self->internal = (st_Dictionary_Internal *)malloc(
      sizeof(st_Dictionary_Internal));
  self->internal->pairs = (st_Dictionary_KeyValuePair *)malloc(
      sizeof(st_Dictionary_KeyValuePair) * INIT_DICTIONARY_SIZE);
  self->internal->sizePairs = INIT_DICTIONARY_SIZE;
  self->internal->numPairs = 0;
}

void st_Dictionary_destroy(
    st_Dictionary *self)
{
  /* Free all key strings (we have allocated that memory ourselves) */
  for (size_t i = 0; i < self->internal->numPairs; ++i) {
    free((char *)self->internal->pairs[i].key);
  }
  /* Free allocated memory */
  free(self->internal->pairs);
  free(self->internal);
}

int comp_keyValuePair(
    const st_Dictionary_KeyValuePair *a,
    const st_Dictionary_KeyValuePair *b)
{
  return strcmp(a->key, b->key);
}

void st_Dictionary_insert(
    st_Dictionary *self,
    const char *key,
    void *value)
{
  st_Dictionary_KeyValuePair *pair;

  /* Check for an existing pair with the given key */
  if (st_Dictionary_getValue(self, key) != NULL) {
    /* TODO: Maybe we should return an error code here? */
    return;
  }

  /* Ensure we have enough memory allocated to insert this pair */
  if (self->internal->numPairs + 1 > self->internal->sizePairs) {
    st_Dictionary_KeyValuePair *newPairs;
    newPairs = (st_Dictionary_KeyValuePair *)malloc(
        sizeof(st_Dictionary_KeyValuePair) * self->internal->sizePairs * 2);
    memcpy(
        newPairs,
        self->internal->pairs, 
        sizeof(st_Dictionary_KeyValuePair) * self->internal->numPairs);
    free(self->internal->pairs);
    self->internal->pairs = newPairs;
    self->internal->sizePairs *= 2;
  }

  /* Insert this key value pair */
  pair = &self->internal->pairs[self->internal->numPairs++];
  pair->value = value;
  /* Copy the key string (this string often comes from transient memory,
   * especially JSON config files) */
  pair->key = (const char *)malloc(strlen(key) + 1);
  strcpy((char *)pair->key, key);

  /* Sort the key value pairs by key */
  qsort(
      self->internal->pairs,  /* ptr */
      self->internal->numPairs,  /* count */
      sizeof(st_Dictionary_KeyValuePair),  /* size */
      (int (*)(const void *, const void *))comp_keyValuePair  /* comp */
      );
}

void *st_Dictionary_getValue(
    st_Dictionary *self,
    const char *key)
{
  int a, b, i;
  if (self->internal->numPairs == 0) {
    return NULL;
  }
  /* Binary search for the key value pair with the given key */
  a = 0; b = self->internal->numPairs;
  while (b - a > 0) {
    int result;
    i = (b - a) / 2 + a;
    result = strcmp(key, self->internal->pairs[i].key);
    if (result < 0) {
      b = i;
    } else if (result > 0) {
      i = a + 1;
    } else {
      /* result == 0 */
      return self->internal->pairs[i].value;
    }
  }
  /* Scan for the key value pair with the given key */
  for (i = a; i < b; ++i) {
    int result;
    result = strcmp(key, self->internal->pairs[i].key);
    if (result == 0) {
      return self->internal->pairs[i].value;
    }
  }
  /* Key value pair with the given key was not found */
  return NULL;
}

size_t st_Dictionary_size(
    st_Dictionary *self)
{
  return self->internal->numPairs;
}

void *st_Dictionary_getValueAtIndex(
    st_Dictionary *self,
    size_t index)
{
  assert(index < self->internal->numPairs);
  return self->internal->pairs[index].value;
}
