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

#ifndef SHELLTOY_PLUGINS_GLSLTOY_PLUGIN_H_
#define SHELLTOY_PLUGINS_GLSLTOY_PLUGIN_H_

#include <jansson.h>

#include "../../error.h"
#include "../../plugin.h"

typedef struct st_Glsltoy_Plugin_ {
  st_Plugin base;
} st_Glsltoy_Plugin;

void st_Glsltoy_Plugin_init(
    st_Glsltoy_Plugin *self,
    const char *name);
void st_Glsltoy_Plugin_destroy(
    st_Glsltoy_Plugin *self);

st_ErrorCode
st_Glsltoy_Plugin_buildToy(
    st_Glsltoy_Plugin *self,
    const char *name,
    json_t *config,
    st_Toy *toy);

#endif
