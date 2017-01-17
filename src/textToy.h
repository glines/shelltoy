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

#ifndef SHELLTOY_TEXT_TOY_H_
#define SHELLTOY_TEXT_TOY_H_

#include <jansson.h>
#include <stddef.h>

typedef struct st_TextToy_Attributes_ {
  size_t size;
} st_TextToy_Attributes;

struct st_TextToy_Internal_;
typedef struct st_TextToy_Internal_ st_TextToy_Internal;

typedef struct st_TextToy_ {
  const char *name;

  st_TextToy_Internal *internal;
} st_TextToy;

typedef void (*st_TextToy_Init)(
    st_TextToy *,  /* self */
    const char *,  /* name */
    json_t *  /* config */
    );

typedef void (*st_TextToy_Destroy)(
    st_TextToy *  /* self */
    );

typedef void (*st_TextToy_Draw)(
    st_TextToy *,  /* self */
    int,  /* viewportWidth */
    int  /* viewportHeight */
    );

typedef struct st_TextToy_Dispatch_ {
  st_TextToy_Init init;
  st_TextToy_Destroy destroy;
  st_TextToy_Draw draw;
} st_TextToy_Dispatch;

void st_TextToy_init(
    st_TextToy *self,
    const char *name,
    const st_TextToy_Dispatch *dispatch);

void st_TextToy_destroy(
    st_TextToy *self);

/* FIXME: st_TextToy_draw does not take enough arguments for drawing text. We
 * will need to pass the glyph atlas and screen representation, among other
 * things. We might even need some additional callbacks for when the text
 * changes. */
void st_TextToy_draw(
    st_TextToy *self,
    int viewportWidth,
    int viewportHeight);

#define SHELLTOY_TEXT_TOY_DISPATCH( \
    TEXT_TOY_STRUCT, \
    INIT_CB, \
    DESTROY_CB, \
    DRAW_CB) \
  const st_TextToy_Attributes SHELLTOY_TEXT_TOY_ATTRIBUTES = { \
    .size = sizeof(TEXT_TOY_STRUCT), \
  }; \
  const st_TextToy_Dispatch SHELLTOY_TEXT_TOY_DISPATCH = { \
    .init = INIT_CB, \
    .destroy = DESTROY_CB, \
    .draw = DRAW_CB, \
  };

#endif
