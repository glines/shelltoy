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

#ifndef SHELLTOY_PLUGIN_H_
#define SHELLTOY_PLUGIN_H_

#include <inttypes.h>
#include <jansson.h>

#include "./common/version.h"
#include "error.h"
#include "toy.h"

typedef enum {
  ST_TOY_TYPE_BACKGROUND = 0x01,
  ST_TOY_TYPE_TEXT = 0x02,
} st_PluginToyTypeMask;

typedef enum {
  ST_GRAPHICS_API_OPENGL = 0x01,
  ST_GRAPHICS_API_VULKAN = 0x02,
  ST_GRAPHICS_API_DX11 = 0x04,
  ST_GRAPHICS_API_DX12 = 0x08,
} st_PluginGraphicsApiMask;

typedef struct st_PluginAttributes_ {
  size_t pluginSize, toySize;
  st_PluginToyTypeMask toyTypes;
  st_PluginGraphicsApiMask graphicsApis;
} st_PluginAttributes;

struct st_Plugin_Internal_;
typedef struct st_Plugin_Internal_ st_Plugin_Internal;

typedef struct st_Plugin_ {
  const char *name;

  st_Plugin_Internal *internal;
} st_Plugin;

typedef void (*st_Plugin_Init)(
    st_Plugin *,  /* self */
    const char *  /* name */
    );
typedef void (*st_Plugin_Destroy)(
    st_Plugin *  /* self */
    );
typedef st_ErrorCode (*st_Plugin_BuildToy)(
    st_Plugin *,  /* self */
    const char *,  /* name */
    json_t *,  /* config */
    st_Toy *  /* toy */
    );

typedef struct st_PluginDispatch_ {
  st_Plugin_Init init;
  st_Plugin_Destroy destroy;
  st_Plugin_BuildToy buildToy;
} st_PluginDispatch;

void
st_Plugin_init(
    st_Plugin *self,
    const char *name);

void
st_Plugin_destroy(
    st_Plugin *self);

#define ST_DEFINE_PLUGIN( \
    PLUGIN_STRUCT, \
    GRAPHICS_APIS, \
    TOY_TYPES, \
    INIT_CB, \
    DESTROY_CB, \
    BUILD_TOY_CB \
    ) \
  const uint32_t SHELLTOY_VERSION = ST_VERSION; \
  const st_PluginAttributes SHELLTOY_PLUGIN_ATTRIBUTES = { \
    .pluginSize = sizeof(PLUGIN_STRUCT), \
    .graphicsApis = GRAPHICS_APIS, \
    .toyTypes = TOY_TYPES, \
  }; \
  const st_PluginDispatch SHELLTOY_PLUGIN_DISPATCH = { \
    .init = INIT_CB, \
    .destroy = DESTROY_CB, \
    .buildToy = BUILD_TOY_CB, \
  };

#endif
