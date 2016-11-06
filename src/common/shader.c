#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "shader.h"

void st_Shader_init(
    st_Shader *self,
    const char *vert,
    unsigned int vert_len,
    const char *frag,
    unsigned int frag_len)
{
  /* TODO: Compile the vertex and fragment shaders */
  self->vert = st_Shader_compileShader(vert, vert_len, GL_VERTEX_SHADER);
  self->frag = st_Shader_compileShader(frag, frag_len, GL_FRAGMENT_SHADER);
}

GLuint st_Shader_compileShader(
    const char *code,
    int code_len,
    GLenum type)
{
  GLuint shader;
  GLint status;

  /* Create a shader object in the GL */
  shader = glCreateShader(type);

  /* Load and compile the shader source */
  glShaderSource(shader, 1, &code, &code_len);
  glCompileShader(shader);

  /* Check for compilation errors */
  glGetShaderiv(
      shader,
      GL_COMPILE_STATUS,
      &status);
  if (status != GL_TRUE) {
    char *log;
    int logLength;
    glGetShaderiv(
        shader,
        GL_INFO_LOG_LENGTH,
        &logLength);
    log = (char*)malloc(logLength);
    glGetShaderInfoLog(
        shader,
        logLength,
        NULL,
        log);
    /* TODO: The shader compilation log needs to be passed out of this
     * context so that it can be displayed in a user-friendly manner */
    fprintf(stderr, "Error compiling shader: %s\n",
        log);
    free(log);
    assert(0);  /* XXX */
  }
}
