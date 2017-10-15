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

#ifndef TTOY_COMMON_GL_ERROR_H_
#define TTOY_COMMON_GL_ERROR_H_

#include <assert.h>

#include <GL/glew.h>

/* Checking for errors significantly affects performance with WebGL, so we
 * disable checking when compiling with Emscripten. */
#ifdef __EMSCRIPTEN__
#define DISABLE_CHECK_GL_ERROR 1
#else
#define DISABLE_CHECK_GL_ERROR 0
#endif

#define FORCE_CHECK_GL_ERROR() \
  (ttoy_checkGLError(__FILE__, __LINE__))

#define FORCE_ASSERT_GL_ERROR() \
  if (ttoy_checkGLError(__FILE__, __LINE__)) \
    assert(0);

#if DISABLE_CHECK_GL_ERROR

#define CHECK_GL_ERROR() (0)

#define ASSERT_GL_ERROR() ;

#else  // DISABLE_CHECK_GL_ERROR

#define CHECK_GL_ERROR() \
  (ttoy_checkGLError(__FILE__, __LINE__))

#define ASSERT_GL_ERROR() \
  if (ttoy_checkGLError(__FILE__, __LINE__)) \
    assert(0);

#endif  // DISABLE_CHECK_GL_ERROR

int ttoy_checkGLError(const char *file, int line);
const char *ttoy_glErrorToString(GLenum error);

#endif
