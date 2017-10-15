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

#ifndef TTOY_COMMON_SHADER_H_
#define TTOY_COMMON_SHADER_H_

#include <GL/glew.h>

#include <ttoy/error.h>

struct ttoy_Shader_Internal_;
typedef struct ttoy_Shader_Internal_ ttoy_Shader_Internal;

typedef struct {
  ttoy_Shader_Internal *internal;
  GLuint program;
} ttoy_Shader;

void ttoy_Shader_init(
    ttoy_Shader *self);

void ttoy_Shader_destroy(
    ttoy_Shader *self);

ttoy_ErrorCode
ttoy_Shader_compileShaderFromString(
    ttoy_Shader *self,
    const GLchar *code,
    GLint length,
    GLenum type);

ttoy_ErrorCode
ttoy_Shader_compileShaderFromFile(
    ttoy_Shader *self,
    const char *filePath,
    GLenum type);

ttoy_ErrorCode
ttoy_Shader_linkProgram(
    ttoy_Shader *self);

void
ttoy_Shader_getLog(
    ttoy_Shader *self,
    const char **log);

#endif
