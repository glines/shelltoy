#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "glError.h"

#include "shader.h"

void st_Shader_init(
    st_Shader *self,
    const char *vert,
    unsigned int vert_len,
    const char *frag,
    unsigned int frag_len)
{
  /* Compile the vertex and fragment shaders */
  self->vert = st_Shader_compileShader(vert, vert_len, GL_VERTEX_SHADER);
  self->frag = st_Shader_compileShader(frag, frag_len, GL_FRAGMENT_SHADER);
  /* Link the vertex and fragment shaders */
  self->program = glCreateProgram();
  FORCE_ASSERT_GL_ERROR();
  st_Shader_linkProgram(self);
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
  FORCE_ASSERT_GL_ERROR();

  /* Load and compile the shader source */
  glShaderSource(shader, 1, &code, &code_len);
  FORCE_ASSERT_GL_ERROR();
  glCompileShader(shader);
  FORCE_ASSERT_GL_ERROR();

  /* Check for compilation errors */
  glGetShaderiv(
      shader,
      GL_COMPILE_STATUS,
      &status);
  FORCE_ASSERT_GL_ERROR();
  if (status != GL_TRUE) {
    char *log;
    int logLength;
    glGetShaderiv(
        shader,
        GL_INFO_LOG_LENGTH,
        &logLength);
    FORCE_ASSERT_GL_ERROR();
    log = (char*)malloc(logLength);
    glGetShaderInfoLog(
        shader,
        logLength,
        NULL,
        log);
    FORCE_ASSERT_GL_ERROR();
    /* TODO: The shader compilation log needs to be passed out of this
     * context so that it can be displayed in a user-friendly manner */
    fprintf(stderr, "Error compiling shader: %s\n",
        log);
    free(log);
    /* TODO: Fail gracefully here */
    assert(0);  /* XXX */
  }

  return shader;
}

void st_Shader_linkProgram(
    st_Shader *self)
{
  GLint status;
  char *log;
  int logLength;

  /* Attach the shaders */
  glAttachShader(self->program, self->vert);
  FORCE_ASSERT_GL_ERROR();
  glAttachShader(self->program, self->frag);
  FORCE_ASSERT_GL_ERROR();
  /* Link the shader program and check for linker errors */
  glLinkProgram(self->program);
  FORCE_ASSERT_GL_ERROR();
  glGetProgramiv(self->program, GL_LINK_STATUS, &status);
  FORCE_ASSERT_GL_ERROR();
  if (status != GL_TRUE) {
    /* Get the output of the linker log */
    glGetProgramiv(
        self->program,
        GL_INFO_LOG_LENGTH,
        &logLength);
    FORCE_ASSERT_GL_ERROR();
    log = (char *)malloc(logLength);
    glGetProgramInfoLog(
        self->program,
        logLength,
        NULL,
        log);
    FORCE_ASSERT_GL_ERROR();
    fprintf(stderr, "Error linking shader program:\n%s", log);
    free(log);
    /* TODO: Fail gracefully here */
    assert(0);  /* XXX */
  }
}
