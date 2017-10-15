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

#include "shaders.h"

#include "../logging.h"
#include "shader.h"

#include <stdio.h>

#define SHADER_DIR assets_shaders

#define CAT(a, b) a ## b

#define TTOY_DEFINE_SHADER_BASE(shader, dir) \
  GLuint ttoy_Shaders_ ## shader ## Shader() { \
    static ttoy_Shader instance; \
    if (instance.program == 0) { \
      ttoy_ErrorCode error; \
      /* This shader has not been initialized (or it might have encountered
       * an error) */ \
      /* FIXME: We should call ttoy_Shader_destroy() at some point */ \
      ttoy_Shader_init(&instance); \
      /* Compile the vertex shader */ \
      error = ttoy_Shader_compileShaderFromString( \
          &instance, \
          (const char *)CAT(dir, _ ## shader ## _vert), /* code */ \
          CAT(dir, _ ## shader ## _vert_len),  /* length */ \
          GL_VERTEX_SHADER  /* type */ \
          ); \
      if (error != TTOY_NO_ERROR) { \
        TTOY_LOG_ERROR( \
            "Failed to compile '%s' vertex shader", \
            #shader); \
        TTOY_LOG_ERROR_CODE(error); \
        ttoy_Shader_destroy(&instance); \
        return 0; \
      } \
      /* Compile the fragment shader */ \
      error = ttoy_Shader_compileShaderFromString( \
          &instance, \
          (const char *)CAT(dir, _ ## shader ## _frag), /* code */ \
          CAT(dir, _ ## shader ## _frag_len),  /* length */ \
          GL_FRAGMENT_SHADER  /* type */ \
          ); \
      if (error != TTOY_NO_ERROR) { \
        TTOY_LOG_ERROR( \
            "Failed to compile '%s' fragment shader", \
            #shader); \
        TTOY_LOG_ERROR_CODE(error); \
        ttoy_Shader_destroy(&instance); \
        return 0; \
      } \
      /* Link the shader program */ \
      error = ttoy_Shader_linkProgram(&instance); \
      if (error != TTOY_NO_ERROR) { \
        TTOY_LOG_ERROR( \
            "Failed to link '%s' shader program", \
            #shader); \
        TTOY_LOG_ERROR_CODE(error); \
        ttoy_Shader_destroy(&instance); \
        return 0; \
      } \
    } \
    /* FIXME: We might want to check if the shader program is actually ready
     * here. It is possible for instance.program to have a value without
     * referring to a valid shader program. */ \
    return instance.program; \
  }
#define TTOY_DEFINE_SHADER(shader) TTOY_DEFINE_SHADER_BASE(shader, SHADER_DIR)

#include "assets_shaders_glyph.vert.c"
#include "assets_shaders_glyph.frag.c"
TTOY_DEFINE_SHADER(glyph)
#include "assets_shaders_background.vert.c"
#include "assets_shaders_background.frag.c"
TTOY_DEFINE_SHADER(background)
#include "assets_shaders_underline.vert.c"
#include "assets_shaders_underline.frag.c"
TTOY_DEFINE_SHADER(underline)
