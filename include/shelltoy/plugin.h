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

#include <shelltoy/backgroundToy.h>
#include <shelltoy/error.h>
#include <shelltoy/textToy.h>
#include <shelltoy/version.h>

typedef enum {
  ST_TOY_TYPE_BACKGROUND = 0x01,
  ST_TOY_TYPE_TEXT = 0x02,
} st_ToyTypeMask;

typedef enum {
  ST_GRAPHICS_API_OPENGL = 0x01,
  ST_GRAPHICS_API_VULKAN = 0x02,
  ST_GRAPHICS_API_DX11 = 0x04,
  ST_GRAPHICS_API_DX12 = 0x08,
} st_GraphicsApiMask;

typedef struct st_Plugin_Attributes_ {
  size_t size;
  st_ToyTypeMask toyTypes;
  st_GraphicsApiMask graphicsApis;
} st_Plugin_Attributes;

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

typedef struct st_Plugin_Dispatch_ {
  st_Plugin_Init init;
  st_Plugin_Destroy destroy;
} st_Plugin_Dispatch;

void
st_Plugin_init(
    st_Plugin *self,
    const char *name,
    const st_Plugin_Dispatch *dispatch,
    const st_BackgroundToy_Attributes *backgroundToyAttributes,
    const st_BackgroundToy_Dispatch *backgroundToyDispatch,
    const st_TextToy_Attributes *textToyAttributes,
    const st_TextToy_Dispatch *textToyDispatch);

void
st_Plugin_destroy(
    st_Plugin *self);

const st_BackgroundToy_Attributes *
st_Plugin_getBackgroundToyAttributes(
    st_Plugin *self);
const st_BackgroundToy_Dispatch *
st_Plugin_getBackgroundToyDispatch(
    st_Plugin *self);

const st_TextToy_Attributes *
st_Plugin_getTextToyAttributes(
    st_Plugin *self);
const st_TextToy_Dispatch *
st_Plugin_getTextToyDispatch(
    st_Plugin *self);

#define SHELLTOY_PLUGIN_DISPATCH( \
    PLUGIN_STRUCT, \
    GRAPHICS_APIS, \
    TOY_TYPES, \
    INIT_CB, \
    DESTROY_CB \
    ) \
  const uint32_t shelltoy_version = SHELLTOY_VERSION; \
  const st_Plugin_Attributes SHELLTOY_PLUGIN_ATTRIBUTES = { \
    .size = sizeof(PLUGIN_STRUCT), \
    .graphicsApis = GRAPHICS_APIS, \
    .toyTypes = TOY_TYPES, \
  }; \
  const st_Plugin_Dispatch SHELLTOY_PLUGIN_DISPATCH = { \
    .init = INIT_CB, \
    .destroy = DESTROY_CB, \
  };

#endif
