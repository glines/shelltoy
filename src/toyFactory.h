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

#ifndef TTOY_TOY_FACTORY_H_
#define TTOY_TOY_FACTORY_H_

#include <jansson.h>

#include <ttoy/backgroundToy.h>
#include <ttoy/error.h>
#include <ttoy/textToy.h>

#include "pluginDictionary.h"

struct ttoy_ToyFactory_Internal;

typedef struct ttoy_ToyFactory_ {
  struct ttoy_ToyFactory_Internal *internal;
} ttoy_ToyFactory;

void ttoy_ToyFactory_init(
    ttoy_ToyFactory *self,
    const char *pluginPath);
void ttoy_ToyFactory_destroy(
    ttoy_ToyFactory *self);

void ttoy_ToyFactory_setPluginPath(
    ttoy_ToyFactory *self,
    const char *path);

ttoy_ErrorCode
ttoy_ToyFactory_registerPlugin(
    ttoy_ToyFactory *self,
    const char *name,
    const char *dlPath);

ttoy_PluginDictionary *
ttoy_ToyFactory_getPlugins(
    ttoy_ToyFactory *self);

ttoy_ErrorCode
ttoy_ToyFactory_buildBackgroundToy(
    ttoy_ToyFactory *self,
    const char *pluginName,
    const char *toyName,
    json_t *config,
    ttoy_BackgroundToy **toy);

ttoy_ErrorCode
ttoy_ToyFactory_buildTextToy(
    ttoy_ToyFactory *self,
    const char *pluginName,
    const char *toyName,
    json_t *config,
    ttoy_TextToy **toy);

#endif
