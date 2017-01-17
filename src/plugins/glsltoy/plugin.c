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
#include "plugin.h"

SHELLTOY_PLUGIN_DISPATCH(
    st_Glsltoy_Plugin,  /* PLUGIN_STRUCT */
    ST_GRAPHICS_API_OPENGL,  /* GRAPHICS_APIS */
    ST_TOY_TYPE_BACKGROUND,  /* TOY_TYPES */
    (st_Plugin_Init)st_Glsltoy_Plugin_init,  /* INIT_CB */
    (st_Plugin_Destroy)st_Glsltoy_Plugin_destroy  /* DESTROY_CB */
    )

void st_Glsltoy_Plugin_init(
    st_Glsltoy_Plugin *self,
    const char *name)
{
  /* TODO */
  /* XXX: Test OpenGL */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  FORCE_ASSERT_GL_ERROR();
}

void st_Glsltoy_Plugin_destroy(
    st_Glsltoy_Plugin *self)
{
}
