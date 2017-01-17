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

#include "toy.h"

SHELLTOY_TOY_DISPATCH(
    st_Glsltoy_Toy,  /* TOY_STRUCT */
    (st_Plugin_Toy_Init)st_Glsltoy_Toy_init,  /* TOY_INIT_CB */
    (st_Plugin_Toy_Destroy)st_Glsltoy_Toy_destroy,  /* TOY_DESTROY_CB */
    (st_Plugin_Toy_GetBackgroundRenderer)
    st_Glsltoy_Toy_getBackgroundRenderer,  /* TOY_GET_BACKGROUNDR_RENDERER_CB */
    (st_Plugin_Toy_GetTextRenderer)
    st_Glsltoy_Toy_getTextRenderer  /* TOY_GET_TEXT_RENDERER_CB */
    )

void st_Glsltoy_Toy_init(
    st_Glsltoy_Toy *self,
    const char *name)
{
  /* TODO */
  fprintf(stderr, "st_Glsltoy_Toy_init() called\n");
}

void st_Glsltoy_Toy_destroy(
    st_Glsltoy_Toy *self)
{
  /* TODO */
  fprintf(stderr, "st_Glsltoy_Toy_destroy() called\n");
}

st_BackgroundRenderer *
st_Glsltoy_Toy_getBackgroundRenderer(
    st_Glsltoy_Toy *self)
{
  /* TODO */
  fprintf(stderr, "st_Glsltoy_Toy_getBackgroundRenderer() called\n");
  return NULL;
}

st_TextRenderer *
st_Glsltoy_Toy_getTextRenderer(
    st_Glsltoy_Toy *self)
{
  /* TODO */
  fprintf(stderr, "st_Glsltoy_Toy_getTextRenderer() called\n");
  return NULL;
}
