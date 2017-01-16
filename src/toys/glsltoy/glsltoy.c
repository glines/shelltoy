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

#include "../../common/version.h"

#include "glsltoy.h"

int shelltoyVersion = ST_VERSION;

void st_GlslToy_init(
    st_GlslToy *self)
{
  /* FIXME: These symbols can never be resolved */
/*  st_Toy_init(&self->base); */
  /* Register all of the implemented virtual methods */
  self->base.buildFromJson =
    (st_ErrorCode (*)(st_Toy *, json_t *))st_GlslToy_buildFromJson;
  self->base.drawBackground =
    (void (*)(const st_Toy *, int, int))st_GlslToy_drawBackground;
}

st_ErrorCode
st_GlslToy_buildFromJson(
    st_GlslToy *self,
    json_t *config)
{
  return ST_NO_ERROR;
}

void st_GlslToy_drawBackground(
    const st_GlslToy *self,
    int width,
    int height)
{
  /* TODO: Draw something pretty */
}
