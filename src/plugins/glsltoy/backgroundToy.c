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

#include <GL/glew.h>

#include "../../common/glError.h"

#include "backgroundToy.h"

SHELLTOY_BACKGROUND_TOY_DISPATCH(
    st_Glsltoy_BackgroundToy,  /* BACKGROUND_TOY_STRUCT */
    (st_BackgroundToy_Init)
    st_Glsltoy_BackgroundToy_init,  /* INIT_CB */
    (st_BackgroundToy_Destroy)
    st_Glsltoy_BackgroundToy_destroy,  /* DESTROY_CB */
    (st_BackgroundToy_Draw)
    st_Glsltoy_BackgroundToy_draw  /* DRAW_CB */
    )

struct st_Glsltoy_BackgroundToy_Internal_ {
  int foo;
};

void st_Glsltoy_BackgroundToy_init(
    st_Glsltoy_BackgroundToy *self,
    const char *name,
    json_t *config)
{
  /* Allocate memory for internal structures */
  self->internal = (st_Glsltoy_BackgroundToy_Internal *)malloc(
      sizeof(st_Glsltoy_BackgroundToy_Internal));
}

void st_Glsltoy_BackgroundToy_destroy(
    st_Glsltoy_BackgroundToy *self)
{
  /* Free allocated memory */
  free(self->internal);
}

void st_Glsltoy_BackgroundToy_draw(
    st_Glsltoy_BackgroundToy *self,
    int viewportWidth,
    int viewportHeight)
{
  /* TODO */
  fprintf(stderr, "inside st_Glsltoy_BackgroundToy_draw()\n");
  glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
  FORCE_ASSERT_GL_ERROR();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  FORCE_ASSERT_GL_ERROR();
}
