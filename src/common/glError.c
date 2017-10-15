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

#include <stdio.h>

#include "glError.h"

int ttoy_checkGLError(const char *file, int line) {
  int result = 0;
  GLenum error;
  while ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "GL error: '%s'\n",
        ttoy_glErrorToString(error));
    result = 1;
  }
  return result;
}

const char *ttoy_glErrorToString(GLenum error) {
#define TTOY_GL_E2S(error) \
  case GL_ ## error: \
                     return "GL_" #error;

  switch (error) {
    // From: <https://www.opengl.org/wiki/OpenGL_Error#Meaning_of_errors>
#ifdef GL_INVALID_ENUM
      TTOY_GL_E2S(INVALID_ENUM)
#endif
#ifdef GL_INVALID_VALUE
      TTOY_GL_E2S(INVALID_VALUE)
#endif
#ifdef GL_INVALID_OPERATION
      TTOY_GL_E2S(INVALID_OPERATION)
#endif
#ifdef GL_STACK_OVERFLOW
      TTOY_GL_E2S(STACK_OVERFLOW)
#endif
#ifdef GL_STACK_UNDERFLOW
      TTOY_GL_E2S(STACK_UNDERFLOW)
#endif
#ifdef GL_OUT_OF_MEMORY
      TTOY_GL_E2S(OUT_OF_MEMORY)
#endif
#ifdef GL_INVALID_FRAMEBUFFER_OPERATION
      TTOY_GL_E2S(INVALID_FRAMEBUFFER_OPERATION)
#endif
#ifdef GL_CONTEXT_LOST
      TTOY_GL_E2S(CONTEXT_LOST)
#endif
#ifdef GL_TABLE_TOO_LARGE
      TTOY_GL_E2S(TABLE_TOO_LARGE)
#endif
    default:
      return "UNKNOWN";
  }
}
