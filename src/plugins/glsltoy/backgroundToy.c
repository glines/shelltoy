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

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_mutex.h>
#include <string.h>

#include <ttoy/fileWatcher.h>

#include "../../common/glError.h"
#include "../../common/shader.h"
#include "../../logging.h"

#include "backgroundToy.h"

TTOY_BACKGROUND_TOY_DISPATCH(
    ttoy_Glsltoy_BackgroundToy,  /* BACKGROUND_TOY_STRUCT */
    (ttoy_BackgroundToy_Init)
    ttoy_Glsltoy_BackgroundToy_init,  /* INIT_CB */
    (ttoy_BackgroundToy_Destroy)
    ttoy_Glsltoy_BackgroundToy_destroy,  /* DESTROY_CB */
    (ttoy_BackgroundToy_Draw)
    ttoy_Glsltoy_BackgroundToy_draw  /* DRAW_CB */
    )

/* Private methods */
void ttoy_Glsltoy_BackgroundToy_readConfig(
    ttoy_Glsltoy_BackgroundToy *self,
    json_t *config);

float ttoy_Glsltoy_BackgroundToy_getTime(
    ttoy_Glsltoy_BackgroundToy *self);

static
ttoy_ErrorCode
ttoy_Glsltoy_BackgroundToy_initShader(
    ttoy_Glsltoy_BackgroundToy *self);

void ttoy_Glsltoy_BackgroundToy_initQuad(
    ttoy_Glsltoy_BackgroundToy *self);

void ttoy_Glsltoy_BackgroundToy_drawShader(
    ttoy_Glsltoy_BackgroundToy *self);

void ttoy_Glsltoy_BackgroundToy_shaderFileChanged(
    ttoy_Glsltoy_BackgroundToy *self,
    const char *filePath);

int ttoy_Glsltoy_BackgroundToy_checkShaderChanges(
    ttoy_Glsltoy_BackgroundToy *self);

ttoy_ErrorCode
ttoy_Glsltoy_BackgroundToy_recompileShader(
    ttoy_Glsltoy_BackgroundToy *self);

typedef struct ttoy_Glsltoy_BackgroundToy_QuadVertex_ {
  GLfloat pos[3], texCoord[2];
} ttoy_Glsltoy_BackgroundToy_QuadVertex;

struct ttoy_Glsltoy_BackgroundToy_Internal_ {
  ttoy_FileWatcher shaderWatcher;
  ttoy_Shader shader;
  SDL_mutex *shaderChangedMutex;
  uint32_t shaderChanged, shaderChangedThreshold;
  char *shaderPath;
  GLuint quadVertexBuffer, quadIndexBuffer, vao;
  GLuint timeLocation, mouseLocation, resolutionLocation;
  int initializedDrawObjects;
  uint32_t startTicks;
};

void ttoy_Glsltoy_BackgroundToy_init(
    ttoy_Glsltoy_BackgroundToy *self,
    const char *name,
    json_t *config)
{
  /* Allocate memory for internal structures */
  self->internal = (ttoy_Glsltoy_BackgroundToy_Internal *)malloc(
      sizeof(ttoy_Glsltoy_BackgroundToy_Internal));
  self->internal->initializedDrawObjects = 0;
  self->internal->startTicks = SDL_GetTicks();
  self->internal->shaderPath = NULL;
  /* Initialize timestamp and mutex for signaling shader file changes to the
   * main thread */
  self->internal->shaderChanged = 0;
  self->internal->shaderChangedMutex = SDL_CreateMutex();
  /* TODO: Allow the user to specify the shader changed threshold (in
   * milliseconds) */
  self->internal->shaderChangedThreshold = 500;
  /* Watch for changes to our fragment shader source file */
  ttoy_FileWatcher_init(
      &self->internal->shaderWatcher);
  ttoy_FileWatcher_setCallback(&self->internal->shaderWatcher,
      (ttoy_FileWatcher_FileChangedCallback)
      ttoy_Glsltoy_BackgroundToy_shaderFileChanged,  /* callback */
      self  /* data */
      );
  /* TODO: Read the config */
  /* FIXME: We might want to move the readConfig method outside of init, for
   * better handling of error codes. */
  ttoy_Glsltoy_BackgroundToy_readConfig(self, config);
}

void ttoy_Glsltoy_BackgroundToy_readConfig(
    ttoy_Glsltoy_BackgroundToy *self,
    json_t *config)
{
  json_t *shaderPath_json;

  /* Traverse the config, which is represented as a JSON object */
  if (config == NULL || !json_is_object(config)) {
    fprintf(stderr, "glsltoy background config must be a JSON object\n");
    return;
  }
  /* Store the shader path */
  shaderPath_json = json_object_get(config, "shaderPath");
  if (shaderPath_json == NULL) {
    fprintf(stderr, "glsltoy background config missing shaderPath\n");
    return;
  } else if (!json_is_string(shaderPath_json)) {
    fprintf(stderr, "glsltoy background shaderPath must be a JSON string\n");
    return;
  }
  free(self->internal->shaderPath);
  self->internal->shaderPath = (char *)malloc(
      strlen(json_string_value(shaderPath_json)) + 1);
  strcpy(self->internal->shaderPath, json_string_value(shaderPath_json));
  /* XXX: Register the shader file with our file watcher */
  fprintf(stderr, "glsltoy registering file to watch: '%s'\n",
      self->internal->shaderPath);
  ttoy_FileWatcher_watchFile(&self->internal->shaderWatcher,
      self->internal->shaderPath  /* filePath */
      );
}

void ttoy_Glsltoy_BackgroundToy_destroy(
    ttoy_Glsltoy_BackgroundToy *self)
{
  if (self->internal->initializedDrawObjects) {
    /* TODO: Clean up the GL objects that we initialized */
  }
  /* Destroy and free our mutex */
  SDL_DestroyMutex(self->internal->shaderChangedMutex);
  /* Free allocated memory */
  free(self->internal->shaderPath);
  free(self->internal);
}

/* FIXME: I think glslsandbox might operate using tenths of a second? This
 * function returns tenths of a second accordingly.*/
float ttoy_Glsltoy_BackgroundToy_getTime(
    ttoy_Glsltoy_BackgroundToy *self)
{
  uint32_t start, current;
  start = self->internal->startTicks;
  current = SDL_GetTicks();
  return (float)(current - start) * 0.0001f;
}

static const char *vert =
  "#version 330\n"
  "\n"
  "layout (location = 0) in vec3 vertPos;\n"
  "layout (location = 1) in vec2 vertTexCoord;\n"
  "\n"
  "void main(void) {\n"
  "  gl_Position = vec4(vertPos, 1.0);\n"
  "}\n";

ttoy_ErrorCode
ttoy_Glsltoy_BackgroundToy_initShader(
    ttoy_Glsltoy_BackgroundToy *self)
{
  ttoy_ErrorCode error;
  /* XXX: Initialize our shader with the test shader program */
  ttoy_Shader_init(&self->internal->shader);

  /* Compile our internal vertex shader */
  error = ttoy_Shader_compileShaderFromString(&self->internal->shader,
      vert,  /* code */
      strlen(vert),  /* length */
      GL_VERTEX_SHADER  /* type */
      );
  if (error == TTOY_ERROR_SHADER_COMPILATION_FAILED) {
    /* TODO: Pass the compilation log up to ttoy_Terminal and display it to the
     * user. It is probably best to use the TTOY_LOG_ERROR facility. */
    /* NOTE: This is a fatal error, since the user has no hope of editing the
     * vertex shader. */
    return error;
  } else if (error != TTOY_NO_ERROR) {
    TTOY_LOG_ERROR_CODE(error);
    return error;
  }
  /* Compile the user-provided fragment shader */
  error = ttoy_Shader_compileShaderFromFile(&self->internal->shader,
      self->internal->shaderPath,  /* filePath */
      GL_FRAGMENT_SHADER  /* type */
      );
  if (error == TTOY_ERROR_SHADER_COMPILATION_FAILED) {
    /* TODO: Pass the compilation log up to ttoy_Terminal and display it to the
     * user. It is probably best to use the TTOY_LOG_ERROR facility. */
    return error;
  } else if (error == TTOY_ERROR_SHADER_FILE_NOT_FOUND) {
    /* TODO: Pass this error up to the ttoy_Terminal and display it to the user
     * through the GUI. */
    return error;
  } else if (error != TTOY_NO_ERROR) {
    TTOY_LOG_ERROR_CODE(error);
    return error;
  }
  /* Link the shader program */
  error = ttoy_Shader_linkProgram(&self->internal->shader);
  if (error == TTOY_ERROR_SHADER_LINKING_FAILED) {
    /* TODO: Pass the linking log up to ttoy_Terminal and display it to the user.
     * It is probably best to use the TTOY_LOG_ERROR facility. */
    return error;
  } else if (error != TTOY_NO_ERROR) {
    TTOY_LOG_ERROR_CODE(error);
    return error;
  }

  /* FIXME: If the user provided shader is invalid, we need to provide a dummy
   * shader? Or perhaps inform ttoy that we do not need to render a
   * background toy texture? */

  /* Get the uniform locations we are interested in */
#define GET_UNIFORM(NAME) \
  self->internal->NAME ## Location = \
    glGetUniformLocation( \
        self->internal->shader.program,  /* program */ \
        #NAME  /* name */ \
        ); \
  FORCE_ASSERT_GL_ERROR();
  GET_UNIFORM(time)
  GET_UNIFORM(mouse)
  GET_UNIFORM(resolution)

  return TTOY_NO_ERROR;
}

void ttoy_Glsltoy_BackgroundToy_initQuad(
    ttoy_Glsltoy_BackgroundToy *self)
{
  ttoy_Glsltoy_BackgroundToy_QuadVertex vertices[] = {
    {
      .pos = { -1.0f, -1.0f, 0.0f },
      .texCoord = { 0.0f, 0.0f },
    },
    {
      .pos = { 1.0f, -1.0f, 0.0f },
      .texCoord = { 1.0f, 0.0f },
    },
    {
      .pos = { -1.0f, 1.0f, 0.0f },
      .texCoord = { 0.0f, 1.0f },
    },
    {
      .pos = { 1.0f, 1.0f, 0.0f },
      .texCoord = { 1.0f, 1.0f },
    },
  };
  GLuint indices[] = {
    0, 1, 2,
    3, 2, 1,
  };

  /* Prepare the buffer for quad vertices */
  glGenBuffers(
      1,  /* n */
      &self->internal->quadVertexBuffer  /* buffers */
      );
  FORCE_ASSERT_GL_ERROR();
  glBindBuffer(
      GL_ARRAY_BUFFER,  /* target */
      self->internal->quadVertexBuffer  /* buffer */
      );
  FORCE_ASSERT_GL_ERROR();
  glBufferData(
      GL_ARRAY_BUFFER,  /* target */
      sizeof(vertices),  /* size */
      vertices,  /* data */
      GL_STATIC_DRAW  /* usage */
      );
  FORCE_ASSERT_GL_ERROR();

  /* Prepare the buffer for quad indices */
  glGenBuffers(
      1,  /* n */
      &self->internal->quadIndexBuffer  /* buffers */
      );
  FORCE_ASSERT_GL_ERROR();
  glBindBuffer(
      GL_ELEMENT_ARRAY_BUFFER,  /* target */
      self->internal->quadIndexBuffer  /* buffer */
      );
  FORCE_ASSERT_GL_ERROR();
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER,  /* target */
      sizeof(indices),  /* size */
      indices,  /* data */
      GL_STATIC_DRAW  /* usage */
      );
  FORCE_ASSERT_GL_ERROR();

  /* Prepare the vertex array object */
  glGenVertexArrays(
      1,  /* n */
      &self->internal->vao  /* arrays */
      );
  FORCE_ASSERT_GL_ERROR();
  glBindVertexArray(
      self->internal->vao  /* array */
      );
  FORCE_ASSERT_GL_ERROR();
  /* Prepare the pos vertex attribute */
  glEnableVertexAttribArray(
      0  /* index */
      );
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribPointer(
      0,  /* index */
      3,  /* size */
      GL_FLOAT,  /* type */
      0,  /* normalized */
      sizeof(ttoy_Glsltoy_BackgroundToy_QuadVertex),  /* stride */
      (GLvoid *)offsetof(ttoy_Glsltoy_BackgroundToy_QuadVertex, pos)  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();
  /* Prepare the texCoord vertex attribute */
  glEnableVertexAttribArray(
      1  /* index */
      );
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribPointer(
      1,  /* index */
      2,  /* size */
      GL_FLOAT,  /* type */
      0,  /* normalized */
      sizeof(ttoy_Glsltoy_BackgroundToy_QuadVertex),  /* stride */
      (GLvoid *)offsetof(ttoy_Glsltoy_BackgroundToy_QuadVertex, texCoord)  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();
  /* Clear the vertex array object binding */
  glBindVertexArray(
      0  /* array */
      );
  FORCE_ASSERT_GL_ERROR();
}

void ttoy_Glsltoy_BackgroundToy_drawShader(
    ttoy_Glsltoy_BackgroundToy *self)
{
  /* Prepare the shader for drawing */
  glUseProgram(
      self->internal->shader.program  /* program */
      );
  FORCE_ASSERT_GL_ERROR();
  glBindBuffer(
      GL_ARRAY_BUFFER,  /* target */
      self->internal->quadVertexBuffer  /* buffer */
      );
  FORCE_ASSERT_GL_ERROR();
  glBindVertexArray(
      self->internal->vao  /* array */
      );
  FORCE_ASSERT_GL_ERROR();
  glUniform1f(
      self->internal->timeLocation,  /* location */
      ttoy_Glsltoy_BackgroundToy_getTime(self)  /* v0 */
      );
  FORCE_ASSERT_GL_ERROR();
  glUniform2f(
      self->internal->mouseLocation,  /* location */
      /* FIXME: Implement mouse */
      50.0f,  /* v0 */
      50.0f  /* v1 */
      );
  FORCE_ASSERT_GL_ERROR();
  glUniform2f(
      self->internal->resolutionLocation,  /* location */
      /* FIXME: Implement resolution */
      640.0f,  /* v0 */
      480.0f  /* v1 */
      );
  FORCE_ASSERT_GL_ERROR();

  /* Draw the shader on a quad to fill the current framebuffer */
  glBindBuffer(
      GL_ELEMENT_ARRAY_BUFFER,  /* target */
      self->internal->quadIndexBuffer  /* buffer */
      );
  FORCE_ASSERT_GL_ERROR();
  glDrawElements(
      GL_TRIANGLES,  /* mode */
      6,  /* count */
      GL_UNSIGNED_INT,  /* type */
      0  /* indices */
      );
  FORCE_ASSERT_GL_ERROR();

  /* Clear the vertex array object binding */
  glBindVertexArray(
      0  /* array */
      );
  FORCE_ASSERT_GL_ERROR();
}

void ttoy_Glsltoy_BackgroundToy_draw(
    ttoy_Glsltoy_BackgroundToy *self,
    int viewportWidth,
    int viewportHeight)
{
  if (!self->internal->initializedDrawObjects) {
    /* Initialize our GL objects on the first frame */
    ttoy_Glsltoy_BackgroundToy_initShader(self);
    ttoy_Glsltoy_BackgroundToy_initQuad(self);
    self->internal->initializedDrawObjects = 1;
  }

  /* Check for pending shader changes */
  if (ttoy_Glsltoy_BackgroundToy_checkShaderChanges(self)) {
    fprintf(stderr, "\033[1mRecompiling shader...\033[0m\n");
    ttoy_Glsltoy_BackgroundToy_recompileShader(self);
  }

  /* Render our shader to the current framebuffer */
  ttoy_Glsltoy_BackgroundToy_drawShader(self);
}

void ttoy_Glsltoy_BackgroundToy_shaderFileChanged(
    ttoy_Glsltoy_BackgroundToy *self,
    const char *filePath)
{
  /* TODO: Attempt to re-compile the shader? We actually need to notify the
   * main thread, since we cannot compile shaders outside of the GL thread. */
  fprintf(stderr, "ttoy_Glsltoy_BackgroundToy_shaderFileChanged\n");
  /* TODO: Flag the main thread to re-compile the shader */
  /* NOTE: Since shaders must be compiled on the main graphics thread with
   * OpenGL, we use a timestamp to limit the adverse effect on terminal
   * interactivity. */
  assert(strcmp(filePath, self->internal->shaderPath) == 0);
  SDL_LockMutex(self->internal->shaderChangedMutex);
  /* FIXME: The value of shaderChanged will wrap after ~49 days of uptime. */
  self->internal->shaderChanged= SDL_GetTicks();
  SDL_UnlockMutex(self->internal->shaderChangedMutex);
}

int ttoy_Glsltoy_BackgroundToy_checkShaderChanges(
    ttoy_Glsltoy_BackgroundToy *self)
{
  int result;

  SDL_LockMutex(self->internal->shaderChangedMutex);
  if (self->internal->shaderChanged == 0) {
    result = 0;
  } else {
    uint32_t delta;
    delta = SDL_GetTicks() - self->internal->shaderChanged;
    if (delta >= self->internal->shaderChangedThreshold) {
      result = 1;
      self->internal->shaderChanged = 0;
    } else {
      result = 0;
    }
  }
  SDL_UnlockMutex(self->internal->shaderChangedMutex);
  return result;
}

ttoy_ErrorCode
ttoy_Glsltoy_BackgroundToy_recompileShader(
    ttoy_Glsltoy_BackgroundToy *self)
{
  ttoy_ErrorCode error;

  /* Compile the user-provided fragment shader */
  error = ttoy_Shader_compileShaderFromFile(&self->internal->shader,
      self->internal->shaderPath,  /* filePath */
      GL_FRAGMENT_SHADER  /* type */
      );
  if (error == TTOY_ERROR_SHADER_COMPILATION_FAILED) {
    /* TODO: Pass the compilation log up to ttoy_Terminal and display it to the
     * user. It is probably best to use the TTOY_LOG_ERROR facility. */
    return error;
  } else if (error == TTOY_ERROR_SHADER_FILE_NOT_FOUND) {
    /* TODO: Pass this error up to the ttoy_Terminal and display it to the user
     * through the GUI. */
    return error;
  } else if (error != TTOY_NO_ERROR) {
    TTOY_LOG_ERROR_CODE(error);
    return error;
  }

  /* Link the shader program */
  error = ttoy_Shader_linkProgram(&self->internal->shader);
  if (error == TTOY_ERROR_SHADER_LINKING_FAILED) {
    /* TODO: Pass the linking log up to ttoy_Terminal and display it to the user.
     * It is probably best to use the TTOY_LOG_ERROR facility. */
    return error;
  } else if (error != TTOY_NO_ERROR) {
    TTOY_LOG_ERROR_CODE(error);
    return error;
  }

  return TTOY_NO_ERROR;
}
