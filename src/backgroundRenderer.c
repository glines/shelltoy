/*
 * Copyright (c) 2017 Jonathan Glines
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
#include <stdlib.h>
#include <string.h>

#include "./common/glError.h"
#include "./common/shader.h"

#include "backgroundRenderer.h"

/* Private methods */
void st_BackgroundRenderer_initShader(
    st_BackgroundRenderer *self);
void st_BackgroundRenderer_initFramebuffer(
    st_BackgroundRenderer *self);
void st_BackgroundRenderer_initQuad(
    st_BackgroundRenderer *self);

void st_BackgroundRenderer_drawBackgroundTexture(
    st_BackgroundRenderer *self);

typedef struct st_BackgroundRenderer_QuadVertex_ {
  GLfloat pos[3], texCoord[2];
} st_BackgroundRenderer_QuadVertex;

struct st_BackgroundRenderer_Internal_ {
  st_BackgroundToy *backgroundToy;
  st_Shader shader;
  GLuint texture, framebuffer, quadVertexBuffer, quadIndexBuffer, vao;
  GLuint toySamplerLocation;
  int initializedDrawObjects;
};

void st_BackgroundRenderer_init(
    st_BackgroundRenderer *self,
    st_Profile *profile)
{
  /* Allocate memory for internal data structures */
  self->internal = (st_BackgroundRenderer_Internal *)malloc(
      sizeof(st_BackgroundRenderer_Internal));
  self->internal->initializedDrawObjects = 0;
  /* Store a pointer to the background toy */
  self->internal->backgroundToy = st_Profile_getBackgroundToy(profile);
}

static const char *vert_shader =
  "#version 330\n"
  "\n"
  "layout (location = 0) in vec3 vertPos;\n"
  "layout (location = 1) in vec2 vertTexCoord;\n"
  "\n"
  "smooth out vec2 texCoord;\n"
  "\n"
  "void main(void) {\n"
  "  gl_Position = vec4(vertPos, 1.0);\n"
  "  texCoord = vertTexCoord;\n"
  "}\n";

static const char *frag_shader =
  "#version 330\n"
  "\n"
  "uniform sampler2D toySampler;\n"
  "\n"
  "smooth in vec2 texCoord;\n"
  "\n"
  "void main(void) {\n"
  "  gl_FragColor = vec4(texture(toySampler, texCoord).rgb, 1.0);\n"
  "}\n";


void st_BackgroundRenderer_initShader(
    st_BackgroundRenderer *self)
{
  /* Compile and link our shader for drawing the toy texture */
  st_Shader_init(
      &self->internal->shader,
      vert_shader,  /* vert */
      strlen(vert_shader),  /* vert_len */
      frag_shader,  /* frag */
      strlen(frag_shader)  /* frag_len */
      );

  /* Get the uniform locations we are interested in */
#define GET_UNIFORM(NAME) \
  self->internal->NAME ## Location = \
    glGetUniformLocation( \
        self->internal->shader.program,  /* program */ \
        #NAME  /* name */ \
        ); \
  FORCE_ASSERT_GL_ERROR();
  GET_UNIFORM(toySampler)
}

void st_BackgroundRenderer_initFramebuffer(
    st_BackgroundRenderer *self)
{
  GLenum result;

  /* Prepare the texture */
  glGenTextures(
      1,  /* n */
      &self->internal->texture  /* textures */
      );
  FORCE_ASSERT_GL_ERROR();
  glBindTexture(
      GL_TEXTURE_2D,  /* target */
      self->internal->texture  /* texture */
      );
  FORCE_ASSERT_GL_ERROR();
  glTexParameteri(
      GL_TEXTURE_2D,  /* target */
      GL_TEXTURE_MIN_FILTER,  /* pname */
      GL_LINEAR  /* param */
      );
  FORCE_ASSERT_GL_ERROR();
  glTexParameteri(
      GL_TEXTURE_2D,  /* target */
      GL_TEXTURE_MAG_FILTER,  /* pname */
      GL_LINEAR  /* param */
      );
  FORCE_ASSERT_GL_ERROR();
  glTexImage2D(
      GL_TEXTURE_2D,  /* target */
      0,  /* level */
      GL_RGBA,  /* internalFormat */
      640,  /* width */
      480,  /* height */
      0,  /* border */
      GL_RGBA,  /* format */
      GL_UNSIGNED_INT_8_8_8_8,  /* type */
      NULL  /* data */
      );
  FORCE_ASSERT_GL_ERROR();

  /* Prepare the framebuffer */
  glGenFramebuffers(
      1,  /* n */
      &self->internal->framebuffer  /* ids */
      );
  FORCE_ASSERT_GL_ERROR();
  glBindFramebuffer(
      GL_DRAW_FRAMEBUFFER,  /* target */
      self->internal->framebuffer  /* framebuffer */
      );
  FORCE_ASSERT_GL_ERROR();
  glFramebufferTexture2D(
      GL_DRAW_FRAMEBUFFER,  /* target */
      GL_COLOR_ATTACHMENT0,  /* attachment */
      GL_TEXTURE_2D,  /* textarget */
      self->internal->texture,  /* texture */
      0  /* level */
      );
  FORCE_ASSERT_GL_ERROR();

  /* Check the validity of the framebuffer */
  result = glCheckFramebufferStatus(
      GL_DRAW_FRAMEBUFFER  /* target */
      );
  FORCE_ASSERT_GL_ERROR();
  assert(result == GL_FRAMEBUFFER_COMPLETE);

  /* Clear the framebuffer binding */
  glBindFramebuffer(
      GL_DRAW_FRAMEBUFFER,  /* target */
      0  /* framebuffer */
      );
  FORCE_ASSERT_GL_ERROR();
}

void st_BackgroundRenderer_initQuad(
    st_BackgroundRenderer *self)
{
  st_BackgroundRenderer_QuadVertex vertices[] = {
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
      sizeof(st_BackgroundRenderer_QuadVertex),  /* stride */
      (GLvoid *)offsetof(st_BackgroundRenderer_QuadVertex, pos)  /* pointer */
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
      sizeof(st_BackgroundRenderer_QuadVertex),  /* stride */
      (GLvoid *)offsetof(st_BackgroundRenderer_QuadVertex, texCoord)  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();
  /* Clear the vertex array object binding */
  glBindVertexArray(
      0  /* array */
      );
  FORCE_ASSERT_GL_ERROR();
}

void st_BackgroundRenderer_destroy(
    st_BackgroundRenderer *self)
{
  if (self->internal->initializedDrawObjects) {
    /* TODO: Clean up the GL objects that we initialized */
  }
  free(self->internal);
}

void st_BackgroundRenderer_drawBackgroundTexture(
    st_BackgroundRenderer *self)
{
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

  /* Prepare the toy texture sampler */
  glActiveTexture(
      GL_TEXTURE0  /* texture */
      );
  FORCE_ASSERT_GL_ERROR();
  glBindTexture(
      GL_TEXTURE_2D,  /* target */
      self->internal->texture  /* texture */
      );
  FORCE_ASSERT_GL_ERROR();
  glUniform1i(
      self->internal->toySamplerLocation,  /* location */
      0  /* v0 */
      );
  FORCE_ASSERT_GL_ERROR();

  /* Draw our texture to the screen */
  glDisable(GL_DEPTH_TEST);
  FORCE_ASSERT_GL_ERROR();
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

  /* Restore the depth test */
  glEnable(GL_DEPTH_TEST);
  FORCE_ASSERT_GL_ERROR();

  /* Clear the vertex array object binding */
  glBindVertexArray(
      0  /* array */
      );
  FORCE_ASSERT_GL_ERROR();
}

void st_BackgroundRenderer_draw(
    st_BackgroundRenderer *self,
    int viewportWidth,
    int viewportHeight)
{
  if (self->internal->backgroundToy != NULL) {
    if (!self->internal->initializedDrawObjects) {
      /* Initialize our GL objects on the first frame */
      st_BackgroundRenderer_initShader(self);
      st_BackgroundRenderer_initFramebuffer(self);
      st_BackgroundRenderer_initQuad(self);
      self->internal->initializedDrawObjects = 1;
    }

    /* Render the background toy to our texture framebuffer */
    glBindFramebuffer(
        GL_DRAW_FRAMEBUFFER,  /* target */
        self->internal->framebuffer  /* framebuffer */
        );
    FORCE_ASSERT_GL_ERROR();
    st_BackgroundToy_draw(self->internal->backgroundToy,
        viewportWidth,  /* viewportWidth */
        viewportHeight  /* viewportHeight */
        );
    /* Clear the framebuffer binding */
    glBindFramebuffer(
        GL_DRAW_FRAMEBUFFER,  /* target */
        0  /* framebuffer */
        );
    FORCE_ASSERT_GL_ERROR();
    /* Draw the rendered shader texture on a quad that fills the screen */
    st_BackgroundRenderer_drawBackgroundTexture(self);
  }
  /* TODO: Without a background toy, what should we do really? We need to draw
   * a solid color. */
}
