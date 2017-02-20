#ifndef SHELLTOY_COMMON_SHADER_H_
#define SHELLTOY_COMMON_SHADER_H_

#include <GL/glew.h>

#include <shelltoy/error.h>

struct st_Shader_Internal_;
typedef struct st_Shader_Internal_ st_Shader_Internal;

typedef struct {
  st_Shader_Internal *internal;
  GLuint program;
} st_Shader;

void st_Shader_init(
    st_Shader *self);

void st_Shader_destroy(
    st_Shader *self);

st_ErrorCode
st_Shader_compileShaderFromString(
    st_Shader *self,
    const GLchar *code,
    GLint length,
    GLenum type);

st_ErrorCode
st_Shader_compileShaderFromFile(
    st_Shader *self,
    const char *filePath,
    GLenum type);

st_ErrorCode
st_Shader_linkProgram(
    st_Shader *self);

void
st_Shader_getLog(
    st_Shader *self,
    const char **log);

#endif
