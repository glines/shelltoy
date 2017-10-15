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

#ifndef TTOY_PLUGIN_H_
#define TTOY_PLUGIN_H_

#include <inttypes.h>
#include <jansson.h>

#include <ttoy/backgroundToy.h>
#include <ttoy/error.h>
#include <ttoy/textToy.h>
#include <ttoy/version.h>

typedef enum {
  TTOY_TOY_TYPE_BACKGROUND = 0x01,
  TTOY_TOY_TYPE_TEXT = 0x02,
} ttoy_ToyTypeMask;

typedef enum {
  TTOY_GRAPHICS_API_OPENGL = 0x01,
  TTOY_GRAPHICS_API_VULKAN = 0x02,
  TTOY_GRAPHICS_API_DX11 = 0x04,
  TTOY_GRAPHICS_API_DX12 = 0x08,
} ttoy_GraphicsApiMask;

typedef struct ttoy_Plugin_Attributes_ {
  size_t size;
  ttoy_ToyTypeMask toyTypes;
  ttoy_GraphicsApiMask graphicsApis;
} ttoy_Plugin_Attributes;

struct ttoy_Plugin_Internal_;
typedef struct ttoy_Plugin_Internal_ ttoy_Plugin_Internal;

typedef struct ttoy_Plugin_ {
  const char *name;

  ttoy_Plugin_Internal *internal;
} ttoy_Plugin;

typedef void (*ttoy_Plugin_Init)(
    ttoy_Plugin *,  /* self */
    const char *  /* name */
    );
typedef void (*ttoy_Plugin_Destroy)(
    ttoy_Plugin *  /* self */
    );

typedef struct ttoy_Plugin_Dispatch_ {
  ttoy_Plugin_Init init;
  ttoy_Plugin_Destroy destroy;
} ttoy_Plugin_Dispatch;

void
ttoy_Plugin_init(
    ttoy_Plugin *self,
    const char *name,
    const ttoy_Plugin_Dispatch *dispatch,
    const ttoy_BackgroundToy_Attributes *backgroundToyAttributes,
    const ttoy_BackgroundToy_Dispatch *backgroundToyDispatch,
    const ttoy_TextToy_Attributes *textToyAttributes,
    const ttoy_TextToy_Dispatch *textToyDispatch);

void
ttoy_Plugin_destroy(
    ttoy_Plugin *self);

const ttoy_BackgroundToy_Attributes *
ttoy_Plugin_getBackgroundToyAttributes(
    ttoy_Plugin *self);
const ttoy_BackgroundToy_Dispatch *
ttoy_Plugin_getBackgroundToyDispatch(
    ttoy_Plugin *self);

const ttoy_TextToy_Attributes *
ttoy_Plugin_getTextToyAttributes(
    ttoy_Plugin *self);
const ttoy_TextToy_Dispatch *
ttoy_Plugin_getTextToyDispatch(
    ttoy_Plugin *self);

#define TTOY_PLUGIN_DISPATCH( \
    PLUGIN_STRUCT, \
    GRAPHICS_APIS, \
    TOY_TYPES, \
    INIT_CB, \
    DESTROY_CB \
    ) \
  const uint32_t ttoy_version = TTOY_VERSION; \
  const ttoy_Plugin_Attributes TTOY_PLUGIN_ATTRIBUTES = { \
    .size = sizeof(PLUGIN_STRUCT), \
    .graphicsApis = GRAPHICS_APIS, \
    .toyTypes = TOY_TYPES, \
  }; \
  const ttoy_Plugin_Dispatch TTOY_PLUGIN_DISPATCH = { \
    .init = INIT_CB, \
    .destroy = DESTROY_CB, \
  };

#endif
