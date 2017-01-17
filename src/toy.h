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

#ifndef SHELLTOY_TOY_H_
#define SHELLTOY_TOY_H_

#include <jansson.h>

#include "backgroundRenderer.h"
#include "error.h"
#include "textRenderer.h"

typedef struct st_Toy_Attributes_ {
  size_t size;
} st_Toy_Attributes;

struct st_Toy_Internal_;
typedef struct st_Toy_Internal_ st_Toy_Internal;

struct st_Toy_;
typedef struct st_Toy_ st_Toy;
struct st_Toy_ {
  const char *name;

  st_Toy_Internal *internal;
};

typedef void (*st_Plugin_Toy_Init)(
    st_Toy *,  /* self */
    const char *  /* name */
    );
typedef void (*st_Plugin_Toy_Destroy)(
    st_Toy *  /* self */
    );
typedef st_BackgroundRenderer *
(*st_Plugin_Toy_GetBackgroundRenderer)(
    st_Toy *  /* self */
    );
typedef st_TextRenderer *
(*st_Plugin_Toy_GetTextRenderer)(
    st_Toy *  /* self */
    );

typedef struct st_Toy_Dispatch_ {
  st_Plugin_Toy_Init init;
  st_Plugin_Toy_Destroy destroy;
  st_Plugin_Toy_GetBackgroundRenderer getBackgroundRenderer;
  st_Plugin_Toy_GetTextRenderer getTextRenderer;
} st_Toy_Dispatch;

void st_Toy_init(
    st_Toy *self,
    const char *name,
    const st_Toy_Dispatch *dispatch);

void st_Toy_destroy(
    st_Toy *self);

st_BackgroundRenderer *
st_Toy_getBackgroundRenderer(
    st_Toy *self);

#define SHELLTOY_TOY_DISPATCH( \
    TOY_STRUCT, \
    TOY_INIT_CB, \
    TOY_DESTROY_CB, \
    TOY_GET_BACKGROUND_RENDERER_CB, \
    TOY_GET_TEXT_RENDERER_CB \
    ) \
  const st_Toy_Attributes SHELLTOY_TOY_ATTRIBUTES = { \
    .size = sizeof(TOY_STRUCT), \
  }; \
  const st_Toy_Dispatch SHELLTOY_TOY_DISPATCH = { \
    .init = TOY_INIT_CB, \
    .destroy = TOY_DESTROY_CB, \
    .getBackgroundRenderer = TOY_GET_BACKGROUND_RENDERER_CB, \
    .getTextRenderer = TOY_GET_TEXT_RENDERER_CB, \
  };

#endif
