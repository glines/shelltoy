#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "glError.h"
#include "../logging.h"

#include "shader.h"

struct st_Shader_Internal_ {
  GLuint vert, frag;
  char *log;
};

void st_Shader_init(
    st_Shader *self)
{
  /* Allocate internal memory */
  self->internal = (st_Shader_Internal *)malloc(sizeof(st_Shader_Internal));
  self->internal->log = NULL;
  self->program = 0;
}

void st_Shader_destroy(
    st_Shader *self)
{
  /* TODO: Free resources allocated in the GL */
  /* Free allocated memory */
  free(self->internal->log);
  free(self->internal);
}

st_ErrorCode
st_Shader_compileShaderFromString(
    st_Shader *self,
    const GLchar *code,
    GLint length,
    GLenum type)
{
  GLuint *shader;
  GLint status;

  switch (type) {
    case GL_VERTEX_SHADER:
      shader = &self->internal->vert;
      break;
    case GL_FRAGMENT_SHADER:
      shader = &self->internal->frag;
      break;
    default:
      return ST_ERROR_UNKNOWN_SHADER_TYPE;
  }

  /* Create a shader object in the GL */
  *shader = glCreateShader(type);
  FORCE_ASSERT_GL_ERROR();

  /* Load and compile the shader source */
  glShaderSource(*shader, 1, &code, &length);
  FORCE_ASSERT_GL_ERROR();
  glCompileShader(*shader);
  FORCE_ASSERT_GL_ERROR();

  /* Check for compilation errors */
  glGetShaderiv(
      *shader,  /* shader */
      GL_COMPILE_STATUS,  /* pname */
      &status  /* params */
      );
  FORCE_ASSERT_GL_ERROR();
  if (status != GL_TRUE) {
    int logLength;
    /* Store the shader compilation log for future access via the
     * st_Shader_getLog() method */
    glGetShaderiv(
        *shader,  /* shader */
        GL_INFO_LOG_LENGTH,  /* pname */
        &logLength  /* params */
        );
    FORCE_ASSERT_GL_ERROR();
    free(self->internal->log);
    self->internal->log = (char*)malloc(logLength);
    if (self->internal->log == NULL) {
      return ST_ERROR_OUT_OF_MEMORY;
    }
    glGetShaderInfoLog(
        *shader,  /* shader */
        logLength,  /* maxLength */
        NULL,  /* length */
        self->internal->log  /* infoLog */
        );
    FORCE_ASSERT_GL_ERROR();
    ST_LOG_ERROR(
        "Error compiling shader: \n"
        "%s",
        self->internal->log);
    return ST_ERROR_SHADER_COMPILATION_FAILED;
  }

  return ST_NO_ERROR;
}

st_ErrorCode
st_Shader_compileShaderFromFile(
    st_Shader *self,
    const char *filePath,
    GLenum type)
{
  FILE *fp;
  char *code;
  int result;
  long length;
  size_t bytesRead;
  st_ErrorCode error;

  /* Load the entire file into a string */
  fp = fopen(filePath, "rb");
  if (fp == NULL) {
    ST_LOG_ERROR(
        "Error opening shader file '%s': %s",
        filePath,
        strerror(errno));
    return ST_ERROR_OPENING_SHADER_FILE;
  }
  /* Seek to the end of the file to determine its length */
  result = fseek(fp, 0, SEEK_END);
  if (result != 0) {
    if (ferror(fp)) {
      ST_LOG_ERROR(
          "fseek() failed on shader file '%s'",
          filePath);
      return ST_ERROR_OPENING_SHADER_FILE;
    }
  }
  length = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  code = (char *)malloc(length);
  if (code == NULL) {
    return ST_ERROR_OUT_OF_MEMORY;
  }
  bytesRead = fread(
      code,  /* buffer */
      1,  /* size */
      length,  /* count */
      fp  /* stream */
      );
  if (bytesRead != length || ferror(fp)) {
    ST_LOG_ERROR(
        "Error reading shader file '%s'",
        filePath);
    free(code);
    return ST_ERROR_OPENING_SHADER_FILE;
  }

  /* Compile the shader string */
  error = st_Shader_compileShaderFromString(self,
      code,  /* code */
      length,  /* length */
      type  /* type */
      );

  free(code);

  return error;
}

st_ErrorCode
st_Shader_linkProgram(
    st_Shader *self)
{
  GLint status;
  char *log;
  int logLength;

  /* Create the program object */
  self->program = glCreateProgram();
  /* Attach the shaders */
  /* TODO: Support attaching shader types other than vert and frag */
  glAttachShader(self->program, self->internal->vert);
  FORCE_ASSERT_GL_ERROR();
  glAttachShader(self->program, self->internal->frag);
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
    return ST_ERROR_SHADER_LINKING_FAILED;
  }
  return ST_NO_ERROR;
}
