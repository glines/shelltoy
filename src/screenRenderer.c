/*
 * Copyright (c) 2016 Jonathan Glines
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

#include "boundingBox.h"
#include "common/glError.h"
#include "common/shaders.h"

#include "screenRenderer.h"

/* Private internal structures */
typedef struct st_ScreenRenderer_QuadVertex_ {
  float pos[2];
} st_ScreenRenderer_QuadVertex;

typedef struct st_ScreenRenderer_GlyphInstance_ {
  st_BoundingBox atlasPos;
  GLuint atlasSampler;
  int cell[2];
  int offset[2];
} st_ScreenRenderer_GlyphInstance;

struct st_ScreenRenderer_Internal {
  st_ScreenRenderer_GlyphInstance *glyphs;
  size_t numGlyphs, sizeGlyphs;
  GLuint quadVertexBuffer, quadIndexBuffer;
  GLuint glyphInstanceBuffer, glyphInstanceVAO;
  GLuint glyphShader;
};

/* Private method declarations */
void st_ScreenRenderer_initShaders(
    st_ScreenRenderer *self);
void st_ScreenRenderer_initBuffers(
    st_ScreenRenderer *self);
void st_ScreenRenderer_initVAO(
    st_ScreenRenderer *self);
void st_ScreenRenderer_screenDrawCallback(
    struct tsm_screen *con,
    uint32_t id,
    const uint32_t *ch,
    size_t len,
    unsigned int width,
    unsigned int posx,
    unsigned int posy,
    const struct tsm_screen_attr *attr,
    tsm_age_t age,
    st_ScreenRenderer *self);

void st_ScreenRenderer_init(
    st_ScreenRenderer *self)
{
  /* Allocate memory for internal data structures */
  self->internal = (struct st_ScreenRenderer_Internal*)malloc(
      sizeof(struct st_ScreenRenderer_Internal));
  self->internal->sizeGlyphs = ST_SCREEN_RENDERER_INIT_SIZE_GLYPHS;
  self->internal->glyphs = (st_ScreenRenderer_GlyphInstance *)malloc(
      sizeof(st_ScreenRenderer_GlyphInstance) * self->internal->sizeGlyphs);
  self->internal->numGlyphs = 0;
  st_ScreenRenderer_initShaders(self);
  st_ScreenRenderer_initBuffers(self);
  st_ScreenRenderer_initVAO(self);
  /* Initialize the glyph atlas */
  st_GlyphAtlas_init(&self->atlas);
}

void st_ScreenRenderer_initShaders(
    st_ScreenRenderer *self)
{
  self->internal->glyphShader = st_Shaders_glyphShader();
}

void st_ScreenRenderer_initBuffers(
    st_ScreenRenderer *self)
{
  static const st_ScreenRenderer_QuadVertex quadVertices[] = {
    { .pos = { 0.0f, 0.0f } },
    { .pos = { 1.0f, 0.0f } },
    { .pos = { 0.0f, 1.0f } },
    { .pos = { 1.0f, 1.0f } },
  };
  static const int quadIndices[] = {
    0, 1, 2,
    3, 2, 1,
  };

  /* Initialize the quad buffers */
  glGenBuffers(1, &self->internal->quadVertexBuffer);
  FORCE_ASSERT_GL_ERROR();
  glGenBuffers(1, &self->internal->quadIndexBuffer);
  FORCE_ASSERT_GL_ERROR();
  /* Send quad buffer data to the GL */
  glBindBuffer(GL_ARRAY_BUFFER, self->internal->quadVertexBuffer);
  FORCE_ASSERT_GL_ERROR();
  glBufferData(
      GL_ARRAY_BUFFER,  /* target */
      sizeof(quadVertices),  /* size */
      quadVertices,  /* data */
      GL_STATIC_DRAW  /* usage */
      );
  FORCE_ASSERT_GL_ERROR();
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->internal->quadIndexBuffer);
  FORCE_ASSERT_GL_ERROR();
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER,  /* target */
      sizeof(quadIndices),  /* size */
      quadIndices,  /* data */
      GL_STATIC_DRAW  /* usage */
      );
  FORCE_ASSERT_GL_ERROR();

  /* Initialize the glyph instance buffer */
  glGenBuffers(1, &self->internal->glyphInstanceBuffer);
  FORCE_ASSERT_GL_ERROR();
  fprintf(stderr, "initialized self->internal->glyphInstanceBuffer: %d\n",
      self->internal->glyphInstanceBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, self->internal->glyphInstanceBuffer);
  FORCE_ASSERT_GL_ERROR();
}

void st_ScreenRenderer_initVAO(
    st_ScreenRenderer *self)
{
  GLuint vertPosLocation;

  glGenVertexArrays(1, &self->internal->glyphInstanceVAO);
  FORCE_ASSERT_GL_ERROR();
  glBindVertexArray(self->internal->glyphInstanceVAO);
  FORCE_ASSERT_GL_ERROR();

  /* Configure the vertex attributes from the quad buffer */
  glBindBuffer(GL_ARRAY_BUFFER, self->internal->quadVertexBuffer);
  FORCE_ASSERT_GL_ERROR();
  /* Configure vertPos */
  vertPosLocation = glGetAttribLocation(
      self->internal->glyphShader,
      "vertPos");
  FORCE_ASSERT_GL_ERROR();
  glEnableVertexAttribArray(vertPosLocation);
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribPointer(
      vertPosLocation,  /* index */
      4,  /* size */
      GL_FLOAT,  /* type */
      GL_FALSE,  /* normalized */
      sizeof(st_ScreenRenderer_QuadVertex),  /* stride */
      ((st_ScreenRenderer_QuadVertex*)0)->pos  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();

  /* Configure the vertex attributes from the glyph instance buffer */
  glBindBuffer(GL_ARRAY_BUFFER, self->internal->glyphInstanceBuffer);
  FORCE_ASSERT_GL_ERROR();
  vertPosLocation = glGetAttribLocation(
      self->internal->glyphShader,
      "vertPos");
  FORCE_ASSERT_GL_ERROR();

  glBindVertexArray(0);
  FORCE_ASSERT_GL_ERROR();
}

void st_ScreenRenderer_destroy(
    st_ScreenRenderer *self)
{
  /* Free internal data structures */
  free(self->internal->glyphs);
  free(self->internal);
  /* Destroy the glyph atlas */
  st_GlyphAtlas_destroy(&self->atlas);
}

void st_ScreenRenderer_updateScreen(
    st_ScreenRenderer *self,
    struct tsm_screen *screen)
{
  /* Fill the glyph instance buffer with the latest screen contents */
  self->internal->numGlyphs = 0;
  tsm_screen_draw(
      screen,  /* con */
      (tsm_screen_draw_cb)st_ScreenRenderer_screenDrawCallback,  /* draw_cb */
      self  /* data */
      );
  /* Send the recently updated glyph instance buffer to the GL */
  /* TODO: Check for GL errors */
  fprintf(stderr, "self->internal->glyphInstanceBuffer: %d\n",
      self->internal->glyphInstanceBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, self->internal->glyphInstanceBuffer);
  ASSERT_GL_ERROR();
  glBufferData(
      GL_ARRAY_BUFFER,  /* target */
      self->internal->numGlyphs
      * sizeof(st_ScreenRenderer_GlyphInstance),  /* size */
      self->internal->glyphs,  /* data */
      GL_DYNAMIC_DRAW);
  ASSERT_GL_ERROR();
}

/** This routine "draws" the each glyph by adding an instance of the glyph to
 * our buffer of glyph instances. The glyphs are not actually drawn
 * immediately, but the GL glyph instances are updated. */
void st_ScreenRenderer_screenDrawCallback(
  struct tsm_screen *con,
  uint32_t id,
  const uint32_t *ch,
  size_t len,
  unsigned int width,
  unsigned int posx,
  unsigned int posy,
  const struct tsm_screen_attr *attr,
  tsm_age_t age,
  st_ScreenRenderer *self)
{
  st_ScreenRenderer_GlyphInstance glyphInstance;
  GLuint atlasTextureBuffer;
  int error;

  /* Skip whitespace */
  if (!(*ch))
    return;

  /* Look this glyph in our atlas */
  error = st_GlyphAtlas_getGlyph(&self->atlas,
      *ch,  /* character */
      &glyphInstance.atlasPos,  /* bbox */
      &atlasTextureBuffer  /* textureBuffer */
      );
  if (error) {
    /* A glyph for the given character could not be found in the atlas */
    /* TODO: Try to add a glyph for this character */
    fprintf(stderr, "Could not find glyph for '0x%08x'", *ch);
    return;
  }

  /* Set up therest of the glyph instance data structure */
  glyphInstance.cell[0] = posx;
  glyphInstance.cell[1] = posy;
  /* FIXME: The glyph offset needs to be retrieved from somewhere */
  glyphInstance.offset[0] = 0;
  glyphInstance.offset[1] = 0;
  /* FIXME: We need to determine which samplers are assigned for which atlas
   * textures */
  glyphInstance.atlasSampler = 0;

  /* Make sure we have memory allocated for the new glyph instance */
  if (self->internal->numGlyphs + 1 > self->internal->sizeGlyphs) {
    st_ScreenRenderer_GlyphInstance *newGlyphs;

    /* Double the number of glyphs to ensure we have enough space */
    newGlyphs = (st_ScreenRenderer_GlyphInstance*)malloc(
        sizeof(st_ScreenRenderer_GlyphInstance) * self->internal->sizeGlyphs * 2);
    memcpy(newGlyphs, self->internal->glyphs,
        sizeof(st_ScreenRenderer_GlyphInstance) * self->internal->sizeGlyphs);
    free(self->internal->glyphs);
    self->internal->glyphs = newGlyphs;
    self->internal->sizeGlyphs *= 2;
  }

  /* Append the glyph instance data structure to our buffer */
  memcpy(&self->internal->glyphs[self->internal->numGlyphs++], &glyphInstance,
      sizeof(st_ScreenRenderer_GlyphInstance));

//  fprintf(stderr, "Instanced glyph for character: '%c' at location (%d, %d)\n",
//      (char)(*ch),
//      posx,
//      posy);  /* XXX */
}

void st_ScreenRenderer_draw(
    const st_ScreenRenderer *self)
{
  /* Use the glyph shader program */
  glUseProgram(self->internal->glyphShader);
  ASSERT_GL_ERROR();

  /* TODO: Set up the VAO for instanced rendering */
  glBindVertexArray(self->internal->glyphInstanceVAO);
  FORCE_ASSERT_GL_ERROR();
  glBindVertexArray(0);
  FORCE_ASSERT_GL_ERROR();
}
