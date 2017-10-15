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
