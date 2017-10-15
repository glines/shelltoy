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

#include "textToy.h"

TTOY_TEXT_TOY_DISPATCH(
    ttoy_Glsltoy_TextToy,  /* TEXT_TOY_STRUCT */
    (ttoy_TextToy_Init)ttoy_Glsltoy_TextToy_init,  /* INIT_CB */
    (ttoy_TextToy_Destroy)ttoy_Glsltoy_TextToy_destroy,  /* DESTROY_CB */
    (ttoy_TextToy_Draw)ttoy_Glsltoy_TextToy_draw  /* DRAW_CB */
    )

struct ttoy_Glsltoy_TextToy_Internal_ {
  int foo;
};

void ttoy_Glsltoy_TextToy_init(
    ttoy_Glsltoy_TextToy *self,
    const char *name,
    json_t *config)
{
  /* Allocate memory for internal structures */
  self->internal = (ttoy_Glsltoy_TextToy_Internal *)malloc(
      sizeof(ttoy_Glsltoy_TextToy_Internal));
}

void ttoy_Glsltoy_TextToy_destroy(
    ttoy_Glsltoy_TextToy *self)
{
  /* Free allocated memory */
  free(self->internal);
}

void ttoy_Glsltoy_TextToy_draw(
    ttoy_Glsltoy_TextToy *self,
    int viewportWidth,
    int viewportHeight)
{
  /* TODO */
  fprintf(stderr, "inside ttoy_Glsltoy_TextToy_draw()\n");
}
