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

#ifndef SHELLTOY_TOY_FACTORY_H_
#define SHELLTOY_TOY_FACTORY_H_

#include <jansson.h>

#include "error.h"
#include "toy.h"

struct st_ToyFactory_Internal;

typedef struct st_ToyFactory_ {
  struct st_ToyFactory_Internal *internal;
} st_ToyFactory;

void st_ToyFactory_init(
    st_ToyFactory *self,
    const char *pluginPath);
void st_ToyFactory_destroy(
    st_ToyFactory *self);

void st_ToyFactory_setPluginPath(
    st_ToyFactory *self,
    const char *path);

st_ErrorCode
st_ToyFactory_registerPlugin(
    st_ToyFactory *self,
    const char *name,
    const char *dlPath);

st_ErrorCode
st_ToyFactory_buildToy(
    st_ToyFactory *self,
    const char *pluginName,
    json_t *config,
    st_Toy *toy);

#endif
