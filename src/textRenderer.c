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

#include "boundingBox.h"
#include "common/glError.h"
#include "common/shaders.h"
#include "logging.h"

#include "textRenderer.h"

#define TTOY_TEXT_RENDERER_INIT_SIZE_GLYPHS (80 * 24 * 2)
#define TTOY_TEXT_RENDERER_INIT_SIZE_BACKGROUND_CELLS (80 * 24 * 2)
#define TTOY_TEXT_RENDERER_INIT_SIZE_UNDERLINES (128)

/* Private internal structures */
typedef struct ttoy_TextRenderer_QuadVertex_ {
  float pos[2];
} ttoy_TextRenderer_QuadVertex;

typedef struct ttoy_TextRenderer_GlyphInstance_ {
  float atlasPos[2];
  float atlasGlyphSize[2];
  float glyphSize[2];
  float offset[2];
  int cell[2];
  uint8_t fgColor[3];
} ttoy_TextRenderer_GlyphInstance;

typedef struct ttoy_TextRenderer_BackgroundInstance_ {
  int cell[2];
  uint8_t bgColor[4];
} ttoy_TextRenderer_BackgroundInstance;

typedef struct ttoy_TextRenderer_UnderlineInstance_ {
  int cell[2];
  uint8_t fgColor[3];
} ttoy_TextRenderer_UnderlineInstance;

typedef struct ttoy_TextRenderer_ScreenDrawCallbackData_ {
  ttoy_TextRenderer *self;
  int cellWidth, cellHeight;
} ttoy_TextRenderer_ScreenDrawCallbackData;

struct ttoy_TextRenderer_Internal {
  ttoy_TextToy *textToy;
  ttoy_GlyphRendererRef *glyphRenderer;
  ttoy_GlyphAtlas *atlas;
  ttoy_Profile *profile;
  ttoy_TextRenderer_GlyphInstance *glyphs;
  size_t numGlyphs, sizeGlyphs;
  ttoy_TextRenderer_BackgroundInstance *backgroundCells;
  size_t numBackgroundCells, sizeBackgroundCells;
  ttoy_TextRenderer_UnderlineInstance *underlines;
  size_t numUnderlines, sizeUnderlines;
  GLuint quadVertexBuffer, quadIndexBuffer;
  GLuint glyphInstanceBuffer, glyphInstanceVAO;
  GLuint backgroundInstanceBuffer, backgroundInstanceVAO;
  GLuint underlineInstanceBuffer, underlineInstanceVAO;
  GLuint glyphShader, backgroundShader, underlineShader;
  int cellWidth, cellHeight;
};

/* Private method declarations */
void ttoy_TextRenderer_initShaders(
    ttoy_TextRenderer *self);
void ttoy_TextRenderer_initBuffers(
    ttoy_TextRenderer *self);
void ttoy_TextRenderer_initVAO(
    ttoy_TextRenderer *self);
void ttoy_TextRenderer_initGlyphInstanceVAO(
    ttoy_TextRenderer *self);
void ttoy_TextRenderer_initBackgroundInstanceVAO(
    ttoy_TextRenderer *self);
void ttoy_TextRenderer_initUnderlineInstanceVAO(
    ttoy_TextRenderer *self);
void ttoy_TextRenderer_screenDrawCallback(
    struct tsm_screen *con,
    uint32_t id,
    const uint32_t *ch,
    size_t len,
    unsigned int width,
    unsigned int posx,
    unsigned int posy,
    const struct tsm_screen_attr *attr,
    tsm_age_t age,
    ttoy_TextRenderer_ScreenDrawCallbackData *data);
void ttoy_TextRenderer_addBackgroundCellInstance(
    ttoy_TextRenderer *self,
    int posx,
    int posy,
    const struct tsm_screen_attr *attr);
void ttoy_TextRenderer_addGlyphInstance(
    ttoy_TextRenderer *self,
    uint32_t ch,
    int posx,
    int posy,
    const struct tsm_screen_attr *attr,
    int cellWidth,
    int cellHeight);
void ttoy_TextRenderer_addUnderlineInstance(
    ttoy_TextRenderer *self,
    int posx,
    int posy,
    const struct tsm_screen_attr *attr);
ttoy_ErrorCode
ttoy_TextRenderer_colorCodeToRGB(
    const ttoy_TextRenderer *self,
    int8_t code,
    uint8_t *rgb);

void ttoy_TextRenderer_drawBackgroundCells(
    const ttoy_TextRenderer *self,
    int cellWidth, int cellHeight,
    int viewportWidth, int viewportHeight);
void ttoy_TextRenderer_drawUnderlines(
    const ttoy_TextRenderer *self,
    int cellWidth, int cellHeight,
    int underlineOffset,
    int viewportWidth, int viewportHeight);
void ttoy_TextRenderer_drawGlyphs(
    const ttoy_TextRenderer *self,
    int cellWidth, int cellHeight,
    int viewportWidth, int viewportHeight);

void ttoy_TextRenderer_init(
    ttoy_TextRenderer *self,
    ttoy_GlyphRendererRef *glyphRenderer,
    ttoy_Profile *profile)
{
  /* Allocate memory for internal data structures */
  self->internal = (struct ttoy_TextRenderer_Internal*)malloc(
      sizeof(struct ttoy_TextRenderer_Internal));
  /* TODO: Consolidate initialization of these dynamic arrays */
  self->internal->sizeGlyphs = TTOY_TEXT_RENDERER_INIT_SIZE_GLYPHS;
  self->internal->glyphs = (ttoy_TextRenderer_GlyphInstance *)malloc(
      sizeof(ttoy_TextRenderer_GlyphInstance) * self->internal->sizeGlyphs);
  self->internal->numGlyphs = 0;
  self->internal->sizeBackgroundCells =
    TTOY_TEXT_RENDERER_INIT_SIZE_BACKGROUND_CELLS;
  self->internal->backgroundCells =
    (ttoy_TextRenderer_BackgroundInstance*)malloc(
      sizeof(ttoy_TextRenderer_BackgroundInstance)
      * self->internal->sizeBackgroundCells);
  self->internal->numBackgroundCells = 0;
  self->internal->sizeUnderlines = TTOY_TEXT_RENDERER_INIT_SIZE_UNDERLINES;
  self->internal->underlines = (ttoy_TextRenderer_UnderlineInstance *)malloc(
      sizeof(ttoy_TextRenderer_UnderlineInstance) * self->internal->sizeUnderlines);
  self->internal->numUnderlines = 0;
  /* Store a reference to the glyph renderer */
  self->internal->glyphRenderer = glyphRenderer;
  ttoy_GlyphRendererRef_increment(glyphRenderer);
  /* Store a pointer to the profile */
  self->internal->profile = profile;
  /* Store a pointer to the text toy, for fancy text rendering */
  self->internal->textToy = ttoy_Profile_getTextToy(profile);
  /* Initialize our GL resources */
  ttoy_TextRenderer_initShaders(self);
  ttoy_TextRenderer_initBuffers(self);
  ttoy_TextRenderer_initVAO(self);
  /* Initialize the glyph atlas */
  self->internal->atlas = (ttoy_GlyphAtlas *)malloc(sizeof(ttoy_GlyphAtlas));
  ttoy_GlyphAtlas_init(self->internal->atlas);
  /* Render glyphs to the atlas representative of ASCII terminals */
  ttoy_GlyphAtlas_renderASCIIGlyphs(self->internal->atlas,
      ttoy_GlyphRendererRef_get(
        self->internal->glyphRenderer)  /* glyphRenderer */
      );
}

void ttoy_TextRenderer_initShaders(
    ttoy_TextRenderer *self)
{
  self->internal->glyphShader = ttoy_Shaders_glyphShader();
  self->internal->backgroundShader = ttoy_Shaders_backgroundShader();
  self->internal->underlineShader = ttoy_Shaders_underlineShader();
}

void ttoy_TextRenderer_initBuffers(
    ttoy_TextRenderer *self)
{
  static const ttoy_TextRenderer_QuadVertex quadVertices[] = {
    { .pos = { 0.0f, 0.0f } },
    { .pos = { 1.0f, 0.0f } },
    { .pos = { 0.0f, 1.0f } },
    { .pos = { 1.0f, 1.0f } },
  };
  static const GLuint quadIndices[] = {
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

  /* Initialize the background instance buffer */
  glGenBuffers(1, &self->internal->backgroundInstanceBuffer);
  FORCE_ASSERT_GL_ERROR();

  /* Initialize the underline instance buffer */
  glGenBuffers(1, &self->internal->underlineInstanceBuffer);
  FORCE_ASSERT_GL_ERROR();
}

void ttoy_TextRenderer_initVAO(
    ttoy_TextRenderer *self)
{
  ttoy_TextRenderer_initGlyphInstanceVAO(self);
  ttoy_TextRenderer_initBackgroundInstanceVAO(self);
  ttoy_TextRenderer_initUnderlineInstanceVAO(self);
}

void ttoy_TextRenderer_initGlyphInstanceVAO(
    ttoy_TextRenderer *self)
{
  GLuint vertPosLocation, atlasPosLocation, atlasGlyphSizeLocation,
         glyphSizeLocation, offsetLocation, cellLocation, fgColorLocation;

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
      2,  /* size */
      GL_FLOAT,  /* type */
      GL_FALSE,  /* normalized */
      sizeof(ttoy_TextRenderer_QuadVertex),  /* stride */
      (void *)offsetof(ttoy_TextRenderer_QuadVertex, pos)  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();

  /* Configure the vertex attributes from the glyph instance buffer */
  glBindBuffer(GL_ARRAY_BUFFER, self->internal->glyphInstanceBuffer);
  FORCE_ASSERT_GL_ERROR();
  /* Configure atlasPos */
  atlasPosLocation = glGetAttribLocation(
      self->internal->glyphShader,
      "atlasPos");
  FORCE_ASSERT_GL_ERROR();
  glEnableVertexAttribArray(atlasPosLocation);
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribPointer(
      atlasPosLocation,  /* index */
      2,  /* size */
      GL_FLOAT,  /* type */
      0,  /* normalized */
      sizeof(ttoy_TextRenderer_GlyphInstance),  /* stride */
      (void *)offsetof(ttoy_TextRenderer_GlyphInstance, atlasPos)  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribDivisor(atlasPosLocation, 1);
  FORCE_ASSERT_GL_ERROR();
  /* Configure atlasGlyphSize */
  atlasGlyphSizeLocation = glGetAttribLocation(
      self->internal->glyphShader,
      "atlasGlyphSize");
  FORCE_ASSERT_GL_ERROR();
  glEnableVertexAttribArray(atlasGlyphSizeLocation);
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribPointer(
      atlasGlyphSizeLocation,  /* index */
      2,  /* size */
      GL_FLOAT,  /* type */
      0,  /* normalized */
      sizeof(ttoy_TextRenderer_GlyphInstance),  /* stride */
      (void *)offsetof(ttoy_TextRenderer_GlyphInstance, atlasGlyphSize)  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribDivisor(atlasGlyphSizeLocation, 1);
  FORCE_ASSERT_GL_ERROR();
  /* Configure glyphSize */
  glyphSizeLocation = glGetAttribLocation(
      self->internal->glyphShader,
      "glyphSize");
  FORCE_ASSERT_GL_ERROR();
  glEnableVertexAttribArray(glyphSizeLocation);
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribPointer(
      glyphSizeLocation,  /* index */
      2,  /* size */
      GL_FLOAT,  /* type */
      0,  /* normalized */
      sizeof(ttoy_TextRenderer_GlyphInstance),  /* stride */
      (void *)offsetof(ttoy_TextRenderer_GlyphInstance, glyphSize)  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribDivisor(glyphSizeLocation, 1);
  FORCE_ASSERT_GL_ERROR();
  /* Configure glyphOffset */
  offsetLocation = glGetAttribLocation(
      self->internal->glyphShader,
      "offset");
  FORCE_ASSERT_GL_ERROR();
  glEnableVertexAttribArray(offsetLocation);
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribPointer(
      offsetLocation,  /* index */
      2,  /* size */
      GL_FLOAT,  /* type */
      0,  /* normalized */
      sizeof(ttoy_TextRenderer_GlyphInstance),  /* stride */
      (void *)offsetof(ttoy_TextRenderer_GlyphInstance, offset)  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribDivisor(offsetLocation, 1);
  FORCE_ASSERT_GL_ERROR();
  /* Configure cell */
  cellLocation = glGetAttribLocation(
      self->internal->glyphShader,
      "cell");
  FORCE_ASSERT_GL_ERROR();
  glEnableVertexAttribArray(cellLocation);
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribIPointer(
      cellLocation,  /* index */
      2,  /* size */
      GL_INT,  /* type */
      sizeof(ttoy_TextRenderer_GlyphInstance),  /* stride */
      (void *)offsetof(ttoy_TextRenderer_GlyphInstance, cell)  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribDivisor(cellLocation, 1);
  FORCE_ASSERT_GL_ERROR();
  /* Configure fgColor */
  fgColorLocation = glGetAttribLocation(
      self->internal->glyphShader,
      "fgColor");
  FORCE_ASSERT_GL_ERROR();
  glEnableVertexAttribArray(fgColorLocation);
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribPointer(
      fgColorLocation,  /* index */
      3,  /* size */
      GL_UNSIGNED_BYTE,  /* type */
      1,  /* normalized */
      sizeof(ttoy_TextRenderer_GlyphInstance),  /* stride */
      (void *)offsetof(ttoy_TextRenderer_GlyphInstance, fgColor)  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribDivisor(fgColorLocation, 1);
  FORCE_ASSERT_GL_ERROR();

  /* Configure the IBO for drawing glyph quads */
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->internal->quadIndexBuffer);
  FORCE_ASSERT_GL_ERROR();

  glBindVertexArray(0);
  FORCE_ASSERT_GL_ERROR();
}

void ttoy_TextRenderer_initBackgroundInstanceVAO(
    ttoy_TextRenderer *self)
{
  GLuint vertPosLocation, cellLocation, bgColorLocation;

  glGenVertexArrays(1, &self->internal->backgroundInstanceVAO);
  FORCE_ASSERT_GL_ERROR();
  glBindVertexArray(self->internal->backgroundInstanceVAO);
  FORCE_ASSERT_GL_ERROR();

  /* Configure the vertex attributes from the quad buffer */
  glBindBuffer(GL_ARRAY_BUFFER, self->internal->quadVertexBuffer);
  FORCE_ASSERT_GL_ERROR();
  /* Configure vertPos */
  vertPosLocation = glGetAttribLocation(
      self->internal->backgroundShader,
      "vertPos");
  FORCE_ASSERT_GL_ERROR();
  glEnableVertexAttribArray(vertPosLocation);
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribPointer(
      vertPosLocation,  /* index */
      2,  /* size */
      GL_FLOAT,  /* type */
      GL_FALSE,  /* normalized */
      sizeof(ttoy_TextRenderer_QuadVertex),  /* stride */
      (void *)offsetof(ttoy_TextRenderer_QuadVertex, pos)  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();

  /* Configure the vertex attributes from the background instance buffer */
  glBindBuffer(GL_ARRAY_BUFFER, self->internal->backgroundInstanceBuffer);
  FORCE_ASSERT_GL_ERROR();
  /* Configure cell */
  cellLocation = glGetAttribLocation(
      self->internal->backgroundShader,
      "cell");
  FORCE_ASSERT_GL_ERROR();
  glEnableVertexAttribArray(cellLocation);
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribIPointer(
      cellLocation,  /* index */
      2,  /* size */
      GL_INT,  /* type */
      sizeof(ttoy_TextRenderer_BackgroundInstance),  /* stride */
      (void *)offsetof(ttoy_TextRenderer_BackgroundInstance, cell)  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribDivisor(cellLocation, 1);
  FORCE_ASSERT_GL_ERROR();
  /* Configure bgColor */
  bgColorLocation = glGetAttribLocation(
      self->internal->backgroundShader,
      "bgColor");
  FORCE_ASSERT_GL_ERROR();
  glEnableVertexAttribArray(bgColorLocation);
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribPointer(
      bgColorLocation,  /* index */
      4,  /* size */
      GL_UNSIGNED_BYTE,  /* type */
      1,  /* normalized */
      sizeof(ttoy_TextRenderer_BackgroundInstance),  /* stride */
      (void *)offsetof(ttoy_TextRenderer_BackgroundInstance, bgColor)  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribDivisor(bgColorLocation, 1);
  FORCE_ASSERT_GL_ERROR();

  /* Configure the IBO for drawing background cell quads */
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->internal->quadIndexBuffer);
  FORCE_ASSERT_GL_ERROR();

  glBindVertexArray(0);
  FORCE_ASSERT_GL_ERROR();
}

void ttoy_TextRenderer_initUnderlineInstanceVAO(
    ttoy_TextRenderer *self)
{
  GLuint vertPosLocation, cellLocation, fgColorLocation;

  glGenVertexArrays(1, &self->internal->underlineInstanceVAO);
  FORCE_ASSERT_GL_ERROR();
  glBindVertexArray(self->internal->underlineInstanceVAO);
  FORCE_ASSERT_GL_ERROR();

  /* Configure the vertex attributes from the quad buffer */
  glBindBuffer(GL_ARRAY_BUFFER, self->internal->quadVertexBuffer);
  FORCE_ASSERT_GL_ERROR();
  /* Configure vertPos */
  vertPosLocation = glGetAttribLocation(
      self->internal->underlineShader,
      "vertPos");
  FORCE_ASSERT_GL_ERROR();
  glEnableVertexAttribArray(vertPosLocation);
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribPointer(
      vertPosLocation,  /* index */
      2,  /* size */
      GL_FLOAT,  /* type */
      GL_FALSE,  /* normalized */
      sizeof(ttoy_TextRenderer_QuadVertex),  /* stride */
      (void *)offsetof(ttoy_TextRenderer_QuadVertex, pos)  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();

  /* Configure the vertex attributes from the underline instance buffer */
  glBindBuffer(GL_ARRAY_BUFFER, self->internal->underlineInstanceBuffer);
  FORCE_ASSERT_GL_ERROR();
  /* Configure cell */
  cellLocation = glGetAttribLocation(
      self->internal->underlineShader,
      "cell");
  FORCE_ASSERT_GL_ERROR();
  glEnableVertexAttribArray(cellLocation);
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribIPointer(
      cellLocation,  /* index */
      2,  /* size */
      GL_INT,  /* type */
      sizeof(ttoy_TextRenderer_UnderlineInstance),  /* stride */
      (void *)offsetof(ttoy_TextRenderer_UnderlineInstance, cell)  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribDivisor(cellLocation, 1);
  FORCE_ASSERT_GL_ERROR();
  /* Configure fgColor */
  fgColorLocation = glGetAttribLocation(
      self->internal->underlineShader,
      "fgColor");
  FORCE_ASSERT_GL_ERROR();
  glEnableVertexAttribArray(fgColorLocation);
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribPointer(
      fgColorLocation,  /* index */
      3,  /* size */
      GL_UNSIGNED_BYTE,  /* type */
      1,  /* normalized */
      sizeof(ttoy_TextRenderer_UnderlineInstance),  /* stride */
      (void *)offsetof(ttoy_TextRenderer_UnderlineInstance, fgColor)  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribDivisor(fgColorLocation, 1);
  FORCE_ASSERT_GL_ERROR();

  /* Configure the IBO for drawing underline quads */
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->internal->quadIndexBuffer);
  FORCE_ASSERT_GL_ERROR();

  glBindVertexArray(0);
  FORCE_ASSERT_GL_ERROR();
}

void ttoy_TextRenderer_destroy(
    ttoy_TextRenderer *self)
{
  /* Destroy the glyph atlas */
  ttoy_GlyphAtlas_destroy(self->internal->atlas);
  free(self->internal->atlas);
  /* Disown our glyph renderer */
  ttoy_GlyphRendererRef_decrement(self->internal->glyphRenderer);
  /* Free internal data structures */
  free(self->internal->underlines);
  free(self->internal->backgroundCells);
  free(self->internal->glyphs);
  free(self->internal);
}

void ttoy_TextRenderer_setGlyphRenderer(
    ttoy_TextRenderer *self,
    ttoy_GlyphRendererRef *glyphRenderer)
{
  ttoy_GlyphAtlas *newAtlas;

  /* TODO: Convert this serial implementation into a parallel implementation
   * with a thread to prepare a stagingAtlas with a
   * stagingGlyphRenderer. */
  /* Construct a new glyph atlas with the given glyph renderer */
  newAtlas = (ttoy_GlyphAtlas *)malloc(sizeof(ttoy_GlyphAtlas));
  ttoy_GlyphAtlas_init(newAtlas);
  ttoy_GlyphAtlas_renderASCIIGlyphs(newAtlas,
      ttoy_GlyphRendererRef_get(glyphRenderer)  /* glyphRenderer */
      );

  /* Destroy our old atlas and replace it with the new one */
  ttoy_GlyphAtlas_destroy(self->internal->atlas);
  free(self->internal->atlas);
  self->internal->atlas = newAtlas;

  /* Disown our old glyph renderer and use the new one */
  ttoy_GlyphRendererRef_decrement(self->internal->glyphRenderer);
  self->internal->glyphRenderer = glyphRenderer;
}

void ttoy_TextRenderer_updateScreen(
    ttoy_TextRenderer *self,
    struct tsm_screen *screen,
    int cellWidth,
    int cellHeight)
{
  ttoy_TextRenderer_ScreenDrawCallbackData data;

  /* Disown the old buffers to avoid synchronization cost. See:
   * <https://www.opengl.org/wiki/Buffer_Object_Streaming> */
  /* FIXME: We could be even nicer to the GL driver by always requesting
   * sufficiently large buffer sizes. */
  glBindBuffer(GL_ARRAY_BUFFER, self->internal->glyphInstanceBuffer);
  ASSERT_GL_ERROR();
  glBufferData(
      GL_ARRAY_BUFFER,  /* target */
      self->internal->numGlyphs
      * sizeof(ttoy_TextRenderer_GlyphInstance),  /* size */
      NULL,  /* data */
      GL_STREAM_DRAW  /* usage */
      );
  ASSERT_GL_ERROR();
  glBindBuffer(GL_ARRAY_BUFFER, self->internal->backgroundInstanceBuffer);
  ASSERT_GL_ERROR();
  glBufferData(
      GL_ARRAY_BUFFER,  /* target */
      self->internal->numBackgroundCells
      * sizeof(ttoy_TextRenderer_BackgroundInstance),  /* size */
      NULL,  /* data */
      GL_STREAM_DRAW  /* usage */
      );
  ASSERT_GL_ERROR();

  /* Fill the background and glyph instance buffers with the latest screen
   * contents */
  data.self = self;
  data.cellWidth = cellWidth;
  data.cellHeight = cellHeight;
  self->internal->numGlyphs = 0;
  self->internal->numBackgroundCells = 0;
  self->internal->numUnderlines = 0;
  tsm_screen_draw(
      screen,  /* con */
      (tsm_screen_draw_cb)ttoy_TextRenderer_screenDrawCallback,  /* draw_cb */
      &data  /* data */
      );

  /* Send the recently updated glyph instance buffer to the GL */
  glBindBuffer(GL_ARRAY_BUFFER, self->internal->glyphInstanceBuffer);
  ASSERT_GL_ERROR();
  glBufferData(
      GL_ARRAY_BUFFER,  /* target */
      self->internal->numGlyphs
      * sizeof(ttoy_TextRenderer_GlyphInstance),  /* size */
      self->internal->glyphs,  /* data */
      GL_STREAM_DRAW  /* usage */
      );
  ASSERT_GL_ERROR();

  /* Send the recently updated background instance buffer to the GL */
  glBindBuffer(GL_ARRAY_BUFFER, self->internal->backgroundInstanceBuffer);
  ASSERT_GL_ERROR();
  glBufferData(
      GL_ARRAY_BUFFER,  /* target */
      self->internal->numBackgroundCells
      * sizeof(ttoy_TextRenderer_BackgroundInstance),  /* size */
      self->internal->backgroundCells,  /* data */
      GL_STREAM_DRAW  /* usage */
      );
  ASSERT_GL_ERROR();

  /* Send the recently updated underline instance buffer to the GL */
  glBindBuffer(GL_ARRAY_BUFFER, self->internal->underlineInstanceBuffer);
  ASSERT_GL_ERROR();
  glBufferData(
      GL_ARRAY_BUFFER,  /* target */
      self->internal->numUnderlines
      * sizeof(ttoy_TextRenderer_UnderlineInstance),  /* size */
      self->internal->underlines,  /* data */
      GL_STREAM_DRAW  /* usage */
      );
  ASSERT_GL_ERROR();
}

/** This routine "draws" the each glyph by adding an instance of the glyph to
 * our buffer of glyph instances. The glyphs are not actually drawn
 * immediately, but the GL glyph instances are updated. */
void ttoy_TextRenderer_screenDrawCallback(
  struct tsm_screen *con,
  uint32_t id,
  const uint32_t *ch,
  size_t len,
  unsigned int width,
  unsigned int posx,
  unsigned int posy,
  const struct tsm_screen_attr *attr,
  tsm_age_t age,
  ttoy_TextRenderer_ScreenDrawCallbackData *data)
{
  ttoy_TextRenderer *self;

  self = data->self;

  /* Add background instances for cells with a background color */
  /* FIXME: I'm not sure how to check for no background color. It might be
   * color codes 16 or 17. */
  /* FIXME: Support background on inverse colors? */
  if (attr->bccode != 17 || attr->inverse) {
    ttoy_TextRenderer_addBackgroundCellInstance(self,
        posx,  /* posx */
        posy,  /* posy */
        attr  /* attr */
        );
  }

  /* Add glyph instances for non-whitespace characters */
  /* FIXME: Better detection for whitespace characters here? */
  if ((*ch) && (*ch != 0x20)) {
    ttoy_TextRenderer_addGlyphInstance(self,
        *ch,  /* ch */
        posx,  /* posx */
        posy,  /* posx */
        attr,  /* attr */
        data->cellWidth,  /* cellWidth */
        data->cellHeight  /* cellWidth */
        );
  }

  /* Add an underline instance for characters with underlines */
  /* FIXME: Support full-width underlines */
  if (attr->underline) {
    ttoy_TextRenderer_addUnderlineInstance(self,
        posx,  /* posx */
        posy,  /* posy */
        attr  /* attr */
        );
  }
}

void ttoy_TextRenderer_addBackgroundCellInstance(
    ttoy_TextRenderer *self,
    int posx,
    int posy,
    const struct tsm_screen_attr *attr)
{
  ttoy_TextRenderer_BackgroundInstance backgroundInstance;
  ttoy_ErrorCode result;
  int8_t code;

  /* Set up the background instance data structure */
  backgroundInstance.cell[0] = posx;
  backgroundInstance.cell[1] = posy;

  /* Determine the background color */
  code = attr->inverse ? attr->fccode : attr->bccode;
  if (code == TTOY_COLOR_BACKGROUND)
    return;  /* transparent background */
  backgroundInstance.bgColor[3] = 255;  /* Assume background alpha of one */
  if (code < 0) {
    /* Use RGB for the background color */
    backgroundInstance.bgColor[0] = attr->inverse ? attr->fr : attr->br;
    backgroundInstance.bgColor[1] = attr->inverse ? attr->fg : attr->bg;
    backgroundInstance.bgColor[2] = attr->inverse ? attr->fb : attr->bb;
  } else {
    result = ttoy_TextRenderer_colorCodeToRGB(self,
        code,  /* code */
        backgroundInstance.bgColor  /* rgb */
        );
    if (result != TTOY_NO_ERROR) {
      /* Set the color to magenta for debugging */
      backgroundInstance.bgColor[0] = 255;
      backgroundInstance.bgColor[1] = 0;
      backgroundInstance.bgColor[2] = 255;
      backgroundInstance.bgColor[3] = 255;
    }
  }

  /* TODO: Consolidate array appending code for the background cell instances */
  /* Make sure we have memory allocated for the new background instance */
  if (self->internal->numBackgroundCells + 1 > self->internal->sizeBackgroundCells) {
    ttoy_TextRenderer_BackgroundInstance *newBackgroundCells;

    /* Double the number of background cells to ensure we have enough space */
    newBackgroundCells = (ttoy_TextRenderer_BackgroundInstance*)malloc(
        sizeof(ttoy_TextRenderer_BackgroundInstance)
        * self->internal->sizeBackgroundCells * 2);
    memcpy(newBackgroundCells, self->internal->backgroundCells,
        sizeof(ttoy_TextRenderer_BackgroundInstance)
        * self->internal->sizeBackgroundCells);
    free(self->internal->backgroundCells);
    self->internal->backgroundCells = newBackgroundCells;
    self->internal->sizeBackgroundCells *= 2;
  }

  /* Append the background instance data structure to our buffer */
  memcpy(
      &self->internal->backgroundCells[self->internal->numBackgroundCells++],
      &backgroundInstance,
      sizeof(ttoy_TextRenderer_BackgroundInstance));
}

void ttoy_TextRenderer_addGlyphInstance(
    ttoy_TextRenderer *self,
    uint32_t ch,
    int posx,
    int posy,
    const struct tsm_screen_attr *attr,
    int cellWidth,
    int cellHeight)
{
  ttoy_GlyphRenderer *glyphRenderer;
  ttoy_TextRenderer_GlyphInstance glyphInstance;
  ttoy_BoundingBox bbox;
  float xOffset, yOffset, glyphWidth, glyphHeight;
  int result;
  int fontIndex;
  int8_t code;
  ttoy_ErrorCode error;

  glyphRenderer = ttoy_GlyphRendererRef_get(self->internal->glyphRenderer);
  error = ttoy_GlyphRenderer_getFontIndex(glyphRenderer,
      ch,  /* character */
      attr->bold,  /* bold */
      &fontIndex  /* fontIndex */
      );
  if (error != TTOY_NO_ERROR) {
    /* No fonts provide a glyph for this character code */
    return;
  }

  /* Look for this glyph in our atlas */
  error = ttoy_GlyphAtlas_getGlyph(self->internal->atlas,
      ch,  /* character */
      fontIndex,  /* fontIndex */
      cellWidth,  /* cellWidth */
      cellHeight,  /* cellHeight */
      &bbox,  /* bbox */
      &xOffset,  /* xOffset */
      &yOffset,  /* yOffset */
      &glyphWidth,  /* glyphWidth */
      &glyphHeight  /* glyphHeight */
      );
  if (error != TTOY_NO_ERROR) {
    /* A glyph for the given character could not be found in the atlas */
    /* TODO: Try to add a glyph for this character */
    /* fprintf(stderr, "Could not find glyph for '0x%08x'\n", ch); */
    /* FIXME: Log one error per missing glyph without repeating errors */
    return;
  }

  /* Set up the glyph instance data structure */
  glyphInstance.atlasGlyphSize[0] = (float)bbox.w;
  glyphInstance.atlasGlyphSize[1] = (float)bbox.h;
  glyphInstance.glyphSize[0] = glyphWidth;
  glyphInstance.glyphSize[1] = glyphHeight;
  glyphInstance.atlasPos[0] = (float)bbox.x;
  glyphInstance.atlasPos[1] = (float)bbox.y;
  glyphInstance.cell[0] = posx;
  glyphInstance.cell[1] = posy;
  glyphInstance.offset[0] = xOffset;
  glyphInstance.offset[1] = yOffset;

  /* Determine the foreground color */
  code = attr->inverse ? attr->bccode : attr->fccode;
  if (code < 0) {
    /* Use RGB for the foreground color */
    glyphInstance.fgColor[0] = attr->inverse ? attr->br : attr->fr;
    glyphInstance.fgColor[1] = attr->inverse ? attr->bg : attr->fg;
    glyphInstance.fgColor[2] = attr->inverse ? attr->bb : attr->fb;
  } else {
    result = ttoy_TextRenderer_colorCodeToRGB(self,
        code,  /* code */
        glyphInstance.fgColor  /* rgb */
        );
    if (result != TTOY_NO_ERROR) {
      /* Set the color to cyan for debugging */
      glyphInstance.fgColor[0] = 0;
      glyphInstance.fgColor[1] = 255;
      glyphInstance.fgColor[2] = 255;
    }
  }

  /* TODO: Implement bold glyph attributes */
  /* TODO: Implement inverse colors */

  /* TODO: Consolidate array appending code for the glyph instances */
  /* Make sure we have memory allocated for the new glyph instance */
  if (self->internal->numGlyphs + 1 > self->internal->sizeGlyphs) {
    ttoy_TextRenderer_GlyphInstance *newGlyphs;

    /* Double the number of glyphs to ensure we have enough space */
    newGlyphs = (ttoy_TextRenderer_GlyphInstance*)malloc(
        sizeof(ttoy_TextRenderer_GlyphInstance) * self->internal->sizeGlyphs * 2);
    memcpy(newGlyphs, self->internal->glyphs,
        sizeof(ttoy_TextRenderer_GlyphInstance) * self->internal->sizeGlyphs);
    free(self->internal->glyphs);
    self->internal->glyphs = newGlyphs;
    self->internal->sizeGlyphs *= 2;
  }

  /* Append the glyph instance data structure to our buffer */
  memcpy(&self->internal->glyphs[self->internal->numGlyphs++], &glyphInstance,
      sizeof(ttoy_TextRenderer_GlyphInstance));
}

void ttoy_TextRenderer_addUnderlineInstance(
    ttoy_TextRenderer *self,
    int posx,
    int posy,
    const struct tsm_screen_attr *attr)
{
  ttoy_TextRenderer_UnderlineInstance underlineInstance;
  ttoy_ErrorCode result;
  int8_t code;

  /* Set up the underline instance data structure */
  underlineInstance.cell[0] = posx;
  underlineInstance.cell[1] = posy;

  /* Determine the foreground color */
  code = attr->inverse ? attr->bccode : attr->fccode;
  if (code < 0) {
    /* Use RGB for the foreground color */
    underlineInstance.fgColor[0] = attr->inverse ? attr->br : attr->fr;
    underlineInstance.fgColor[1] = attr->inverse ? attr->bg : attr->fg;
    underlineInstance.fgColor[2] = attr->inverse ? attr->bb : attr->fb;
  } else {
    result = ttoy_TextRenderer_colorCodeToRGB(self,
        code,  /* code */
        underlineInstance.fgColor  /* rgb */
        );
    if (result != TTOY_NO_ERROR) {
      /* Set the color to cyan for debugging */
      underlineInstance.fgColor[0] = 0;
      underlineInstance.fgColor[1] = 255;
      underlineInstance.fgColor[2] = 255;
    }
  }

  /* TODO: Consolidate array appending code for the underline instances */
  /* Make sure we have memory allocated for the new underline instance */
  if (self->internal->numUnderlines + 1 > self->internal->sizeUnderlines) {
    ttoy_TextRenderer_UnderlineInstance *newUnderlines;

    /* Double the number of underlines to ensure we have enough space */
    newUnderlines = (ttoy_TextRenderer_UnderlineInstance*)malloc(
        sizeof(ttoy_TextRenderer_UnderlineInstance) * self->internal->sizeUnderlines * 2);
    memcpy(newUnderlines, self->internal->underlines,
        sizeof(ttoy_TextRenderer_UnderlineInstance) * self->internal->sizeUnderlines);
    free(self->internal->underlines);
    self->internal->underlines = newUnderlines;
    self->internal->sizeUnderlines *= 2;
  }

  /* Append the underline instance data structure to our buffer */
  memcpy(&self->internal->underlines[self->internal->numUnderlines++], &underlineInstance,
      sizeof(ttoy_TextRenderer_UnderlineInstance));
}

ttoy_ErrorCode
ttoy_TextRenderer_colorCodeToRGB(
    const ttoy_TextRenderer *self,
    int8_t code,
    uint8_t *rgb)
{
  /* Retrieve the RGB color from our color scheme */
  switch (code) {
    case TTOY_COLOR_0:
    case TTOY_COLOR_1:
    case TTOY_COLOR_2:
    case TTOY_COLOR_3:
    case TTOY_COLOR_4:
    case TTOY_COLOR_5:
    case TTOY_COLOR_6:
    case TTOY_COLOR_7:
    case TTOY_COLOR_8:
    case TTOY_COLOR_9:
    case TTOY_COLOR_10:
    case TTOY_COLOR_11:
    case TTOY_COLOR_12:
    case TTOY_COLOR_13:
    case TTOY_COLOR_14:
    case TTOY_COLOR_15:
    case TTOY_COLOR_FOREGROUND:
    case TTOY_COLOR_BACKGROUND:
      memcpy(
          rgb,
          &self->internal->profile->colorScheme.colors[code],
          sizeof(ttoy_Color));
      return TTOY_NO_ERROR;
    default:
      TTOY_LOG_ERROR("Unknown color code: '%d'\n", code);
  }
  return TTOY_ERROR_UNKNOWN_COLOR_CODE;
}

void ttoy_TextRenderer_drawBackgroundCells(
    const ttoy_TextRenderer *self,
    int cellWidth, int cellHeight,
    int viewportWidth, int viewportHeight)
{
  GLuint cellSizeLocation, viewportSizeLocation;

  /* Use the background cell shader program */
  glUseProgram(self->internal->backgroundShader);
  ASSERT_GL_ERROR();

  /* Use our VAO for instanced background cell rendering */
  glBindVertexArray(self->internal->backgroundInstanceVAO);
  ASSERT_GL_ERROR();

  /* Configure the uniform values */
  /* Configure the cellSize uniform */
  /* FIXME: Store the cellSize attribute location */
  cellSizeLocation = glGetUniformLocation(
      self->internal->backgroundShader,
      "cellSize");
  ASSERT_GL_ERROR();
  glUniform2i(cellSizeLocation,
      cellWidth,
      cellHeight);
  ASSERT_GL_ERROR();
  /* Configure the viewportSize uniform */
  /* FIXME: Store the viewportSize attribute location */
  viewportSizeLocation = glGetUniformLocation(
      self->internal->backgroundShader,
      "viewportSize");
  ASSERT_GL_ERROR();
  glUniform2i(viewportSizeLocation,
      viewportWidth,
      viewportHeight);
  ASSERT_GL_ERROR();

  /* Draw the instanced background cells, which are quads */
  glDrawElementsInstanced(
      /* FIXME: This might be better as GL_TRIANGLE_STRIP or GL_TRIANGLE_FAN */
      GL_TRIANGLES,  /* mode */
      6,  /* count */
      GL_UNSIGNED_INT,  /* mode */
      0,  /* indices */
      self->internal->numBackgroundCells  /* primcount */
      );
  ASSERT_GL_ERROR();

  glBindVertexArray(0);
  ASSERT_GL_ERROR();
}

void ttoy_TextRenderer_drawUnderlines(
    const ttoy_TextRenderer *self,
    int cellWidth, int cellHeight,
    int underlineOffset,
    int viewportWidth, int viewportHeight)
{
  GLuint cellSizeLocation, underlineOffsetLocation, viewportSizeLocation;

  /* Use the underline shader program */
  glUseProgram(self->internal->underlineShader);
  ASSERT_GL_ERROR();

  /* Use our VAO for instanced underline rendering */
  glBindVertexArray(self->internal->underlineInstanceVAO);
  ASSERT_GL_ERROR();

  /* Configure the uniform values */
  /* Configure the cellSize uniform */
  /* FIXME: Store the cellSize attribute location */
  cellSizeLocation = glGetUniformLocation(
      self->internal->underlineShader,
      "cellSize");
  ASSERT_GL_ERROR();
  glUniform2i(cellSizeLocation,
      cellWidth,
      cellHeight);
  ASSERT_GL_ERROR();
  /* Configure the underlineOffset uniform */
  /* FIXME: Store the underlineOffset attribute location */
  underlineOffsetLocation = glGetUniformLocation(
      self->internal->underlineShader,
      "underlineOffset");
  ASSERT_GL_ERROR();
  glUniform1i(underlineOffsetLocation,
      underlineOffset);
  ASSERT_GL_ERROR();
  /* Configure the viewportSize uniform */
  /* FIXME: Store the viewportSize attribute location */
  viewportSizeLocation = glGetUniformLocation(
      self->internal->underlineShader,
      "viewportSize");
  ASSERT_GL_ERROR();
  glUniform2i(viewportSizeLocation,
      viewportWidth,
      viewportHeight);
  ASSERT_GL_ERROR();

  /* Draw the instanced underlines, which are quads */
  glDrawElementsInstanced(
      /* FIXME: This might be better as GL_TRIANGLE_STRIP or GL_TRIANGLE_FAN */
      GL_TRIANGLES,  /* mode */
      6,  /* count */
      GL_UNSIGNED_INT,  /* mode */
      0,  /* indices */
      self->internal->numUnderlines  /* primcount */
      );
  ASSERT_GL_ERROR();

  glBindVertexArray(0);
  ASSERT_GL_ERROR();
}

void ttoy_TextRenderer_drawGlyphs(
    const ttoy_TextRenderer *self,
    int cellWidth, int cellHeight,
    int viewportWidth, int viewportHeight)
{
  static const GLuint atlasSamplers[] = {
    GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3, GL_TEXTURE4,
    GL_TEXTURE5, GL_TEXTURE6, GL_TEXTURE7, GL_TEXTURE8, GL_TEXTURE9,
    GL_TEXTURE10, GL_TEXTURE11, GL_TEXTURE12, GL_TEXTURE13, GL_TEXTURE14,
    GL_TEXTURE15,
  };
  /* Note that OpenGL 3.x specifies that at least 16 texture units must be
   * available. */
  assert(TTOY_GLYPH_ATLAS_MAX_NUM_TEXTURES <= 16);
  GLuint atlasTextures[TTOY_GLYPH_ATLAS_MAX_NUM_TEXTURES];
  int numAtlasTextures, atlasSize;
  GLuint atlasLocation, cellSizeLocation, viewportSizeLocation,
         atlasSizeLocation;

  /* Use the glyph shader program */
  glUseProgram(self->internal->glyphShader);
  ASSERT_GL_ERROR();

  /* Configure the texture samplers */
  ttoy_GlyphAtlas_getTextures(self->internal->atlas,
      atlasTextures,  /* textures */
      &numAtlasTextures  /* numTextures */
      );
  for (int i = 0; i < numAtlasTextures; ++i) {
    glActiveTexture(atlasSamplers[i]);
    ASSERT_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, atlasTextures[i]);
    ASSERT_GL_ERROR();
  }

  /* Use our VAO for instanced glyph rendering */
  glBindVertexArray(self->internal->glyphInstanceVAO);
  ASSERT_GL_ERROR();

  /* Configure the uniform values */
  /* FIXME: We should store these uniform locations and avoid looking for them
   * at every draw call */
  /* Configure the atlas texture sampler uniform */
  atlasLocation = glGetUniformLocation(
      self->internal->glyphShader,
      "atlas");
  ASSERT_GL_ERROR();
  /* FIXME: The GL doesn't like it when we don't bind all of the textures */
/*  glUniform1iv(atlasLocation, numAtlasTextures, atlasSamplers); */
  glUniform1i(atlasLocation, 0);
  ASSERT_GL_ERROR();
  /* Configure the cellSize uniform */
  /* FIXME: Store the cellSize attribute location */
  cellSizeLocation = glGetUniformLocation(
      self->internal->glyphShader,
      "cellSize");
  ASSERT_GL_ERROR();
  glUniform2i(cellSizeLocation,
      cellWidth,
      cellHeight);
  ASSERT_GL_ERROR();
  /* Configure the viewportSize uniform */
  /* FIXME: Store the viewportSize attribute location */
  viewportSizeLocation = glGetUniformLocation(
      self->internal->glyphShader,
      "viewportSize");
  ASSERT_GL_ERROR();
  glUniform2i(viewportSizeLocation,
      viewportWidth,
      viewportHeight);
  ASSERT_GL_ERROR();
  /* Configure the atlasSize uniform */
  atlasSizeLocation = glGetUniformLocation(
      self->internal->glyphShader,
      "atlasSize");
  ASSERT_GL_ERROR();
  atlasSize = ttoy_GlyphAtlas_getTextureSize(self->internal->atlas);
  glUniform1i(atlasSizeLocation,
      atlasSize);
  ASSERT_GL_ERROR();

  /* Draw the instanced glyph quads */
  glDrawElementsInstanced(
      /* FIXME: This might be better as GL_TRIANGLE_STRIP or GL_TRIANGLE_FAN */
      GL_TRIANGLES,  /* mode */
      6,  /* count */
      GL_UNSIGNED_INT,  /* type */
      0,  /* indices */
      self->internal->numGlyphs  /* primcount */
      );
  ASSERT_GL_ERROR();

  glBindVertexArray(0);
  ASSERT_GL_ERROR();
}

void ttoy_TextRenderer_draw(
    const ttoy_TextRenderer *self,
    int cellWidth, int cellHeight,
    int viewportWidth, int viewportHeight)
{
  /* Configure blending mode */
  glEnable(GL_BLEND);
  ASSERT_GL_ERROR();
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  ASSERT_GL_ERROR();
  /* Disable depth test */
  /* NOTE: We assume no glyphs occlude any other glyphs. This obviates the need
   * to sort glyphs by depth to avoid having them fail the depth test. */
  glDisable(GL_DEPTH_TEST);
  ASSERT_GL_ERROR();

  /* Draw the background cells */
  ttoy_TextRenderer_drawBackgroundCells(self,
      cellWidth,  /* cellWidth */
      cellHeight,  /* cellHeight */
      viewportWidth,  /* viewportWidth */
      viewportHeight  /* viewportHeight */
      );

  /* Draw the underlines on top of the background cells */
  int underlineOffset = 2;  /* FIXME: The glyph renderer needs to calculate the
                               underline offset */
  ttoy_TextRenderer_drawUnderlines(self,
      cellWidth,  /* cellWidth */
      cellHeight,  /* cellHeight */
      underlineOffset,  /* underlineOffset */
      viewportWidth,  /* viewportWidth */
      viewportHeight  /* viewportHeight */
      );

  /* Draw the glyphs on top of everything */
  ttoy_TextRenderer_drawGlyphs(self,
      cellWidth,  /* cellWidth */
      cellHeight,  /* cellHeight */
      viewportWidth,  /* viewportWidth */
      viewportHeight  /* viewportHeight */
      );

  /* Restore depth test */
  glEnable(GL_DEPTH_TEST);
  ASSERT_GL_ERROR();
  /* Restore blending mode */
  glDisable(GL_BLEND);
  ASSERT_GL_ERROR();
}
