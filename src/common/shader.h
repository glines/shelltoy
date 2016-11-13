#ifndef SHELLTOY_COMMON_SHADER_H_
#define SHELLTOY_COMMON_SHADER_H_

#include <GL/glew.h>

typedef struct {
  GLuint vert, frag, program;
} st_Shader;

void st_Shader_init(
    st_Shader *self,
    const char *vert,
    unsigned int vert_len,
    const char *frag,
    unsigned int frag_len);
void st_Shader_destroy(
    st_Shader *self);

GLuint st_Shader_compileShader(
    const char *code,
    int code_len,
    GLenum type);

void st_Shader_linkProgram(
    st_Shader *self);

#endif
