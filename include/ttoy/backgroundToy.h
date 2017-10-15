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

#ifndef TTOY_BACKGROUND_TOY_H_
#define TTOY_BACKGROUND_TOY_H_

#include <jansson.h>

typedef struct ttoy_BackgroundToy_Attributes_ {
  size_t size;
} ttoy_BackgroundToy_Attributes;

struct ttoy_BackgroundToy_Internal_;
typedef struct ttoy_BackgroundToy_Internal_
ttoy_BackgroundToy_Internal;

typedef struct ttoy_BackgroundToy_ {
  const char *name;

  ttoy_BackgroundToy_Internal *internal;
} ttoy_BackgroundToy;

typedef void (*ttoy_BackgroundToy_Init)(
    ttoy_BackgroundToy *,  /* self */
    const char *,  /* name */
    json_t *  /* config */
    );

typedef void (*ttoy_BackgroundToy_Destroy)(
    ttoy_BackgroundToy *  /* self */
    );

typedef void (*ttoy_BackgroundToy_Draw)(
    ttoy_BackgroundToy *,  /* self */
    int,  /* viewportWidth */
    int  /* viewportHeight */
    );

typedef struct ttoy_BackgroundToy_Dispatch_ {
  ttoy_BackgroundToy_Init init;
  ttoy_BackgroundToy_Destroy destroy;
  ttoy_BackgroundToy_Draw draw;
} ttoy_BackgroundToy_Dispatch;

void ttoy_BackgroundToy_init(
    ttoy_BackgroundToy *self,
    const char *name,
    const ttoy_BackgroundToy_Dispatch *dispatch);

void ttoy_BackgroundToy_destroy(
    ttoy_BackgroundToy *self);

void ttoy_BackgroundToy_draw(
    ttoy_BackgroundToy *self,
    int viewportWidth,
    int viewportHeight);

#define TTOY_BACKGROUND_TOY_DISPATCH( \
    BACKGROUND_TOY_STRUCT, \
    INIT_CB, \
    DESTROY_CB, \
    DRAW_CB) \
  const ttoy_BackgroundToy_Attributes TTOY_BACKGROUND_TOY_ATTRIBUTES = { \
    .size = sizeof(BACKGROUND_TOY_STRUCT), \
  }; \
  const ttoy_BackgroundToy_Dispatch TTOY_BACKGROUND_TOY_DISPATCH = { \
    .init = INIT_CB, \
    .destroy = DESTROY_CB, \
    .draw = DRAW_CB, \
  };

#endif
