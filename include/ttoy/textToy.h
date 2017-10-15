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

#ifndef TTOY_TEXT_TOY_H_
#define TTOY_TEXT_TOY_H_

#include <jansson.h>
#include <stddef.h>

typedef struct ttoy_TextToy_Attributes_ {
  size_t size;
} ttoy_TextToy_Attributes;

struct ttoy_TextToy_Internal_;
typedef struct ttoy_TextToy_Internal_ ttoy_TextToy_Internal;

typedef struct ttoy_TextToy_ {
  const char *name;

  ttoy_TextToy_Internal *internal;
} ttoy_TextToy;

typedef void (*ttoy_TextToy_Init)(
    ttoy_TextToy *,  /* self */
    const char *,  /* name */
    json_t *  /* config */
    );

typedef void (*ttoy_TextToy_Destroy)(
    ttoy_TextToy *  /* self */
    );

typedef void (*ttoy_TextToy_Draw)(
    ttoy_TextToy *,  /* self */
    int,  /* viewportWidth */
    int  /* viewportHeight */
    );

typedef struct ttoy_TextToy_Dispatch_ {
  ttoy_TextToy_Init init;
  ttoy_TextToy_Destroy destroy;
  ttoy_TextToy_Draw draw;
} ttoy_TextToy_Dispatch;

void ttoy_TextToy_init(
    ttoy_TextToy *self,
    const char *name,
    const ttoy_TextToy_Dispatch *dispatch);

void ttoy_TextToy_destroy(
    ttoy_TextToy *self);

/* FIXME: ttoy_TextToy_draw does not take enough arguments for drawing text. We
 * will need to pass the glyph atlas and screen representation, among other
 * things. We might even need some additional callbacks for when the text
 * changes. */
void ttoy_TextToy_draw(
    ttoy_TextToy *self,
    int viewportWidth,
    int viewportHeight);

#define TTOY_TEXT_TOY_DISPATCH( \
    TEXT_TOY_STRUCT, \
    INIT_CB, \
    DESTROY_CB, \
    DRAW_CB) \
  const ttoy_TextToy_Attributes TTOY_TEXT_TOY_ATTRIBUTES = { \
    .size = sizeof(TEXT_TOY_STRUCT), \
  }; \
  const ttoy_TextToy_Dispatch TTOY_TEXT_TOY_DISPATCH = { \
    .init = INIT_CB, \
    .destroy = DESTROY_CB, \
    .draw = DRAW_CB, \
  };

#endif
