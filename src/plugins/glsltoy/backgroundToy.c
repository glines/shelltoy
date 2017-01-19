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
#include <string.h>

#include "../../common/glError.h"
#include "../../common/shader.h"

#include "backgroundToy.h"

SHELLTOY_BACKGROUND_TOY_DISPATCH(
    st_Glsltoy_BackgroundToy,  /* BACKGROUND_TOY_STRUCT */
    (st_BackgroundToy_Init)
    st_Glsltoy_BackgroundToy_init,  /* INIT_CB */
    (st_BackgroundToy_Destroy)
    st_Glsltoy_BackgroundToy_destroy,  /* DESTROY_CB */
    (st_BackgroundToy_Draw)
    st_Glsltoy_BackgroundToy_draw  /* DRAW_CB */
    )

/* Private methods */
void st_Glsltoy_BackgroundToy_initShader(
    st_Glsltoy_BackgroundToy *self);

void st_Glsltoy_BackgroundToy_initFramebuffer(
    st_Glsltoy_BackgroundToy *self);

void st_Glsltoy_BackgroundToy_initQuad(
    st_Glsltoy_BackgroundToy *self);

void st_Glsltoy_BackgroundToy_drawShader(
    st_Glsltoy_BackgroundToy *self);

void st_Glsltoy_BackgroundToy_drawQuad(
    st_Glsltoy_BackgroundToy *self,
    int viewportWidth,
    int viewportHeight);

typedef struct st_Glsltoy_BackgroundToy_QuadVertex_ {
  GLfloat pos[3], texCoord[2];
} st_Glsltoy_BackgroundToy_QuadVertex;

struct st_Glsltoy_BackgroundToy_Internal_ {
  st_Shader shader, blitShader;
  GLuint texture, framebuffer, quadVertexBuffer, quadIndexBuffer, vao;
  GLuint timeLocation, mouseLocation, resolutionLocation, toySamplerLocation;
  int initializedDrawObjects;
};

void st_Glsltoy_BackgroundToy_init(
    st_Glsltoy_BackgroundToy *self,
    const char *name,
    json_t *config)
{
  /* Allocate memory for internal structures */
  self->internal = (st_Glsltoy_BackgroundToy_Internal *)malloc(
      sizeof(st_Glsltoy_BackgroundToy_Internal));
  self->internal->initializedDrawObjects = 0;
}

void st_Glsltoy_BackgroundToy_destroy(
    st_Glsltoy_BackgroundToy *self)
{
  if (self->internal->initializedDrawObjects) {
    /* TODO: Clean up the GL objects that we initialized */
  }
  /* Free allocated memory */
  free(self->internal);
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

/* XXX: Example copied from glslsandbox.com */
static const char *frag =
  "#version 330\n"
  "\n"
  "uniform float time;\n"
  "uniform vec2 mouse;\n"
  "uniform vec2 resolution;\n"
  "\n"
  "void main( void ) {\n"
  "\n"
  "  vec2 position = ( gl_FragCoord.xy / resolution.xy ) + mouse / 4.0;\n"
  "\n"
  "  float color = 0.0;\n"
  "  color += sin( position.x * cos( time / 15.0 ) * 80.0 ) + cos( position.y * cos( time / 15.0 ) * 10.0 );\n"
  "  color += sin( position.y * sin( time / 10.0 ) * 40.0 ) + cos( position.x * sin( time / 25.0 ) * 40.0 );\n"
  "  color += sin( position.x * sin( time / 5.0 ) * 10.0 ) + sin( position.y * sin( time / 35.0 ) * 80.0 );\n"
  "  color *= sin( time / 10.0 ) * 0.5;\n"
  "\n"
  "  gl_FragColor = vec4( vec3( color, color * 0.5, sin( color + time / 3.0 ) * 0.75 ), 1.0 );\n"
  "}\n";

static const char *blit_vert =
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

static const char *blit_frag =
  "#version 330\n"
  "\n"
  "uniform sampler2D toySampler;\n"
  "\n"
  "smooth in vec2 texCoord;\n"
  "\n"
  "void main(void) {\n"
  "  gl_FragColor = vec4(1.0, texture(toySampler, texCoord).gb, 1.0);\n"
  "}\n";

void st_Glsltoy_BackgroundToy_initShader(
    st_Glsltoy_BackgroundToy *self)
{
  /* XXX: Initialize our shader with the test shader program */
  st_Shader_init(
      &self->internal->shader,
      vert,  /* vert */
      strlen(vert),  /* vert_len */
      frag,  /* frag */
      strlen(frag)  /* frag_len */
      );
  st_Shader_init(
      &self->internal->blitShader,
      blit_vert,  /* vert */
      strlen(blit_vert),  /* vert_len */
      blit_frag,  /* frag */
      strlen(blit_frag)  /* frag_len */
      );

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
  GET_UNIFORM(toySampler)
}

void st_Glsltoy_BackgroundToy_initFramebuffer(
    st_Glsltoy_BackgroundToy *self)
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

void st_Glsltoy_BackgroundToy_initQuad(
    st_Glsltoy_BackgroundToy *self)
{
  st_Glsltoy_BackgroundToy_QuadVertex vertices[] = {
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
      sizeof(st_Glsltoy_BackgroundToy_QuadVertex),  /* stride */
      (GLvoid *)offsetof(st_Glsltoy_BackgroundToy_QuadVertex, pos)  /* pointer */
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
      sizeof(st_Glsltoy_BackgroundToy_QuadVertex),  /* stride */
      (GLvoid *)offsetof(st_Glsltoy_BackgroundToy_QuadVertex, texCoord)  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();
  /* Clear the vertex array object binding */
  glBindVertexArray(
      0  /* array */
      );
  FORCE_ASSERT_GL_ERROR();
}

void st_Glsltoy_BackgroundToy_drawShader(
    st_Glsltoy_BackgroundToy *self)
{
  /* Prepare the shader for drawing */
  glUseProgram(
      self->internal->shader.program  /* program */
      );
  FORCE_ASSERT_GL_ERROR();
  glBindFramebuffer(
      GL_DRAW_FRAMEBUFFER,  /* target */
      self->internal->framebuffer  /* framebuffer */
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
      /* FIXME: Implement time */
      0.0f  /* v0 */
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

  /* Render the shader on a quad to our texture framebuffer */
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
  /* Clear the framebuffer binding */
  glBindFramebuffer(
      GL_DRAW_FRAMEBUFFER,  /* target */
      0  /* framebuffer */
      );
  FORCE_ASSERT_GL_ERROR();
}

void st_Glsltoy_BackgroundToy_drawQuad(
    st_Glsltoy_BackgroundToy *self,
    int viewportWidth,
    int viewportHeight)
{
  glUseProgram(
      self->internal->blitShader.program  /* program */
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
  fprintf(stderr, "vao: %d\n",
      self->internal->vao);

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
      GL_TEXTURE0  /* v0 */
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

void st_Glsltoy_BackgroundToy_draw(
    st_Glsltoy_BackgroundToy *self,
    int viewportWidth,
    int viewportHeight)
{
  if (!self->internal->initializedDrawObjects) {
    /* Initialize the GL objects on the first frame */
    st_Glsltoy_BackgroundToy_initShader(self);
    st_Glsltoy_BackgroundToy_initFramebuffer(self);
    st_Glsltoy_BackgroundToy_initQuad(self);
    self->internal->initializedDrawObjects = 1;
  }

  /* Render the shader to our texture framebuffer */
  st_Glsltoy_BackgroundToy_drawShader(self);

  /* Draw the rendered shader texture on a quad that fills the screen */
  st_Glsltoy_BackgroundToy_drawQuad(
      self,
      viewportWidth,
      viewportHeight);

  /* TODO */
  fprintf(stderr, "inside st_Glsltoy_BackgroundToy_draw()\n");
}