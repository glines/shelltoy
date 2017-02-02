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

#ifndef SHELLTOY_BACKGROUND_TOY_H_
#define SHELLTOY_BACKGROUND_TOY_H_

#include <jansson.h>

typedef struct st_BackgroundToy_Attributes_ {
  size_t size;
} st_BackgroundToy_Attributes;

struct st_BackgroundToy_Internal_;
typedef struct st_BackgroundToy_Internal_
st_BackgroundToy_Internal;

typedef struct st_BackgroundToy_ {
  const char *name;

  st_BackgroundToy_Internal *internal;
} st_BackgroundToy;

typedef void (*st_BackgroundToy_Init)(
    st_BackgroundToy *,  /* self */
    const char *,  /* name */
    json_t *  /* config */
    );

typedef void (*st_BackgroundToy_Destroy)(
    st_BackgroundToy *  /* self */
    );

typedef void (*st_BackgroundToy_Draw)(
    st_BackgroundToy *,  /* self */
    int,  /* viewportWidth */
    int  /* viewportHeight */
    );

typedef struct st_BackgroundToy_Dispatch_ {
  st_BackgroundToy_Init init;
  st_BackgroundToy_Destroy destroy;
  st_BackgroundToy_Draw draw;
} st_BackgroundToy_Dispatch;

void st_BackgroundToy_init(
    st_BackgroundToy *self,
    const char *name,
    const st_BackgroundToy_Dispatch *dispatch);

void st_BackgroundToy_destroy(
    st_BackgroundToy *self);

void st_BackgroundToy_draw(
    st_BackgroundToy *self,
    int viewportWidth,
    int viewportHeight);

#define SHELLTOY_BACKGROUND_TOY_DISPATCH( \
    BACKGROUND_TOY_STRUCT, \
    INIT_CB, \
    DESTROY_CB, \
    DRAW_CB) \
  const st_BackgroundToy_Attributes SHELLTOY_BACKGROUND_TOY_ATTRIBUTES = { \
    .size = sizeof(BACKGROUND_TOY_STRUCT), \
  }; \
  const st_BackgroundToy_Dispatch SHELLTOY_BACKGROUND_TOY_DISPATCH = { \
    .init = INIT_CB, \
    .destroy = DESTROY_CB, \
    .draw = DRAW_CB, \
  };

#endif
