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
  float atlasPos[2];
  float glyphSize[2];
  float offset[2];
  int cell[2];
  uint8_t fgColor[3];
} st_ScreenRenderer_GlyphInstance;

typedef struct st_ScreenRenderer_BackgroundInstance_ {
  int cell[2];
  uint8_t bgColor[4];
} st_ScreenRenderer_BackgroundInstance;

typedef struct st_ScreenRenderer_ScreenDrawCallbackData_ {
  st_ScreenRenderer *self;
  st_GlyphRenderer *glyphRenderer;
} st_ScreenRenderer_ScreenDrawCallbackData;

struct st_ScreenRenderer_Internal {
  st_ScreenRenderer_GlyphInstance *glyphs;
  size_t numGlyphs, sizeGlyphs;
  st_ScreenRenderer_BackgroundInstance *backgroundCells;
  size_t numBackgroundCells, sizeBackgroundCells;
  GLuint quadVertexBuffer, quadIndexBuffer;
  GLuint glyphInstanceBuffer, glyphInstanceVAO;
  GLuint backgroundInstanceBuffer, backgroundInstanceVAO;
  GLuint glyphShader, backgroundShader;
};

/* Private method declarations */
void st_ScreenRenderer_initShaders(
    st_ScreenRenderer *self);
void st_ScreenRenderer_initBuffers(
    st_ScreenRenderer *self);
void st_ScreenRenderer_initVAO(
    st_ScreenRenderer *self);
void st_ScreenRenderer_initGlyphInstanceVAO(
    st_ScreenRenderer *self);
void st_ScreenRenderer_initBackgroundInstanceVAO(
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
    st_ScreenRenderer_ScreenDrawCallbackData *data);
void st_ScreenRenderer_addBackgroundCellInstance(
    st_ScreenRenderer *self,
    int posx,
    int posy,
    const struct tsm_screen_attr *attr);
void st_ScreenRenderer_addGlyphInstance(
    st_ScreenRenderer *self,
    st_GlyphRenderer *glyphRenderer,
    uint32_t ch,
    int posx,
    int posy,
    const struct tsm_screen_attr *attr);
int st_ScreenRenderer_colorCodeToRGB(
    const st_ScreenRenderer *self,
    int8_t code,
    uint8_t *rgb);
void st_ScreenRenderer_drawBackgroundCells(
    const st_ScreenRenderer *self,
    int cellWidth, int cellHeight,
    int viewportWidth, int viewportHeight);

void st_ScreenRenderer_init(
    st_ScreenRenderer *self,
    st_GlyphRenderer *glyphRenderer)
{
  /* Allocate memory for internal data structures */
  self->internal = (struct st_ScreenRenderer_Internal*)malloc(
      sizeof(struct st_ScreenRenderer_Internal));
  self->internal->sizeGlyphs = ST_SCREEN_RENDERER_INIT_SIZE_GLYPHS;
  self->internal->glyphs = (st_ScreenRenderer_GlyphInstance *)malloc(
      sizeof(st_ScreenRenderer_GlyphInstance) * self->internal->sizeGlyphs);
  self->internal->numGlyphs = 0;
  self->internal->sizeBackgroundCells =
    ST_SCREEN_RENDERER_INIT_SIZE_BACKGROUND_CELLS;
  self->internal->backgroundCells =
    (st_ScreenRenderer_BackgroundInstance*)malloc(
      sizeof(st_ScreenRenderer_BackgroundInstance)
      * self->internal->sizeBackgroundCells);
  self->internal->numBackgroundCells = 0;
  st_ScreenRenderer_initShaders(self);
  st_ScreenRenderer_initBuffers(self);
  st_ScreenRenderer_initVAO(self);
  /* Initialize the glyph atlas */
  st_GlyphAtlas_init(&self->atlas);
  /* Render glyphs to the atlas representative of ASCII terminals */
  st_GlyphAtlas_renderASCIIGlyphs(&self->atlas, glyphRenderer);
}

void st_ScreenRenderer_initShaders(
    st_ScreenRenderer *self)
{
  self->internal->glyphShader = st_Shaders_glyphShader();
  self->internal->backgroundShader = st_Shaders_backgroundShader();
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
}

void st_ScreenRenderer_initVAO(
    st_ScreenRenderer *self)
{
  st_ScreenRenderer_initGlyphInstanceVAO(self);
  st_ScreenRenderer_initBackgroundInstanceVAO(self);
}

void st_ScreenRenderer_initGlyphInstanceVAO(
    st_ScreenRenderer *self)
{
  GLuint vertPosLocation, atlasIndexLocation, atlasPosLocation,
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
      sizeof(st_ScreenRenderer_QuadVertex),  /* stride */
      ((st_ScreenRenderer_QuadVertex*)0)->pos  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();

  /* Configure the vertex attributes from the glyph instance buffer */
  glBindBuffer(GL_ARRAY_BUFFER, self->internal->glyphInstanceBuffer);
  FORCE_ASSERT_GL_ERROR();
  /* Configure atlasIndex */
//  atlasIndexLocation = glGetAttribLocation(
//      self->internal->glyphShader,
//      "atlasIndex");
//  FORCE_ASSERT_GL_ERROR();
//  glEnableVertexAttribArray(atlasIndexLocation);
//  FORCE_ASSERT_GL_ERROR();
//  glVertexAttribPointer(
//      atlasIndexLocation,  /* index */
//      1,  /* size */
//      GL_INT,  /* type */
//      0,  /* normalized */
//      sizeof(st_ScreenRenderer_GlyphInstance),  /* stride */
//      &((st_ScreenRenderer_GlyphInstance*)0)->atlasIndex  /* pointer */
//      );
//  FORCE_ASSERT_GL_ERROR();
//  glVertexAttribDivisor(atlasIndexLocation, 1);
//  FORCE_ASSERT_GL_ERROR();
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
      sizeof(st_ScreenRenderer_GlyphInstance),  /* stride */
      ((st_ScreenRenderer_GlyphInstance*)0)->atlasPos  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribDivisor(atlasPosLocation, 1);
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
      sizeof(st_ScreenRenderer_GlyphInstance),  /* stride */
      ((st_ScreenRenderer_GlyphInstance*)0)->glyphSize  /* pointer */
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
      sizeof(st_ScreenRenderer_GlyphInstance),  /* stride */
      ((st_ScreenRenderer_GlyphInstance*)0)->offset  /* pointer */
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
      sizeof(st_ScreenRenderer_GlyphInstance),  /* stride */
      ((st_ScreenRenderer_GlyphInstance*)0)->cell  /* pointer */
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
      sizeof(st_ScreenRenderer_GlyphInstance),  /* stride */
      ((st_ScreenRenderer_GlyphInstance*)0)->fgColor  /* pointer */
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

void st_ScreenRenderer_initBackgroundInstanceVAO(
    st_ScreenRenderer *self)
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
      sizeof(st_ScreenRenderer_QuadVertex),  /* stride */
      ((st_ScreenRenderer_QuadVertex*)0)->pos  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();

  /* Configure the vertex attributes from the glyph instance buffer */
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
      sizeof(st_ScreenRenderer_BackgroundInstance),  /* stride */
      ((st_ScreenRenderer_BackgroundInstance*)0)->cell  /* pointer */
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
      sizeof(st_ScreenRenderer_BackgroundInstance),  /* stride */
      ((st_ScreenRenderer_BackgroundInstance*)0)->bgColor  /* pointer */
      );
  FORCE_ASSERT_GL_ERROR();
  glVertexAttribDivisor(bgColorLocation, 1);
  FORCE_ASSERT_GL_ERROR();

  /* Configure the IBO for drawing glyph quads */
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->internal->quadIndexBuffer);
  FORCE_ASSERT_GL_ERROR();

  glBindVertexArray(0);
  FORCE_ASSERT_GL_ERROR();
}

void st_ScreenRenderer_destroy(
    st_ScreenRenderer *self)
{
  /* Destroy the glyph atlas */
  st_GlyphAtlas_destroy(&self->atlas);
  /* Free internal data structures */
  free(self->internal->backgroundCells);
  free(self->internal->glyphs);
  free(self->internal);
}

void st_ScreenRenderer_updateScreen(
    st_ScreenRenderer *self,
    struct tsm_screen *screen,
    st_GlyphRenderer *glyphRenderer)
{
  st_ScreenRenderer_ScreenDrawCallbackData data;

  /* Disown the old buffers to avoid synchronization cost. See:
   * <https://www.opengl.org/wiki/Buffer_Object_Streaming> */
  /* FIXME: We could be even nicer to the GL driver by always requesting
   * sufficiently large buffer sizes. */
  glBufferData(
      GL_ARRAY_BUFFER,  /* target */
      self->internal->numGlyphs
      * sizeof(st_ScreenRenderer_GlyphInstance),  /* size */
      NULL,  /* data */
      GL_STREAM_DRAW  /* usage */
      );
  ASSERT_GL_ERROR();
  glBufferData(
      GL_ARRAY_BUFFER,  /* target */
      self->internal->numBackgroundCells
      * sizeof(st_ScreenRenderer_BackgroundInstance),  /* size */
      NULL,  /* data */
      GL_STREAM_DRAW  /* usage */
      );
  ASSERT_GL_ERROR();

  /* Fill the background and glyph instance buffers with the latest screen
   * contents */
  data.self = self;
  data.glyphRenderer = glyphRenderer;
  self->internal->numGlyphs = 0;
  self->internal->numBackgroundCells = 0;
  tsm_screen_draw(
      screen,  /* con */
      (tsm_screen_draw_cb)st_ScreenRenderer_screenDrawCallback,  /* draw_cb */
      &data  /* data */
      );

  /* Send the recently updated glyph instance buffer to the GL */
  glBindBuffer(GL_ARRAY_BUFFER, self->internal->glyphInstanceBuffer);
  ASSERT_GL_ERROR();
  glBufferData(
      GL_ARRAY_BUFFER,  /* target */
      self->internal->numGlyphs
      * sizeof(st_ScreenRenderer_GlyphInstance),  /* size */
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
      * sizeof(st_ScreenRenderer_BackgroundInstance),  /* size */
      self->internal->backgroundCells,  /* data */
      GL_STREAM_DRAW  /* usage */
      );
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
  st_ScreenRenderer_ScreenDrawCallbackData *data)
{
  st_ScreenRenderer *self;
  st_GlyphRenderer *glyphRenderer;

  self = data->self;
  glyphRenderer = data->glyphRenderer;

  /* Add background instances for cells with a background color */
  /* FIXME: I'm not sure how to check for no background color. It might be
   * color codes 16 or 17. */
  if (attr->bccode != 17 || attr->inverse) {
    st_ScreenRenderer_addBackgroundCellInstance(self,
        posx,  /* posx */
        posy,  /* posy */
        attr  /* attr */
        );
  }

  /* Add glyph instances for non-whitespace characters */
  if ((*ch) && (*ch != 0x20)) {
    st_ScreenRenderer_addGlyphInstance(self,
        glyphRenderer,  /* glyphRenderer */
        *ch,  /* ch */
        posx,  /* posx */
        posy,  /* posx */
        attr  /* attr */
        );
  }
}

void st_ScreenRenderer_addBackgroundCellInstance(
    st_ScreenRenderer *self,
    int posx,
    int posy,
    const struct tsm_screen_attr *attr)
{
  st_ScreenRenderer_BackgroundInstance backgroundInstance;
  int result;

  /* Set up the background instance data structure */
  backgroundInstance.cell[0] = posx;
  backgroundInstance.cell[1] = posy;

  /* Determine the background color */
  backgroundInstance.bgColor[3] = 255;  /* Assume background alpha of one */
  if (attr->bccode < 0) {
    /* Use RGB for the background color */
    backgroundInstance.bgColor[0] = attr->br;
    backgroundInstance.bgColor[1] = attr->bg;
    backgroundInstance.bgColor[2] = attr->bb;
  } else {
    result = st_ScreenRenderer_colorCodeToRGB(self,
        attr->bccode,  /* code */
        backgroundInstance.bgColor  /* rgb */
        );
    if (!result) {
      /* Use the default background color */
      // assert(0);  /* XXX: We should never actually reach here */
      /* Set the color to magenta for easier debugging */
      backgroundInstance.bgColor[0] = 255;
      backgroundInstance.bgColor[1] = 0;
      backgroundInstance.bgColor[2] = 255;
      backgroundInstance.bgColor[3] = 255;
    }
  }

  /* Make sure we have memory allocated for the new background instance */
  if (self->internal->numBackgroundCells + 1 > self->internal->sizeBackgroundCells) {
    st_ScreenRenderer_BackgroundInstance *newBackgroundCells;

    /* Double the number of background cells to ensure we have enough space */
    newBackgroundCells = (st_ScreenRenderer_BackgroundInstance*)malloc(
        sizeof(st_ScreenRenderer_BackgroundInstance)
        * self->internal->sizeBackgroundCells * 2);
    memcpy(newBackgroundCells, self->internal->backgroundCells,
        sizeof(st_ScreenRenderer_BackgroundInstance)
        * self->internal->sizeBackgroundCells);
    free(self->internal->backgroundCells);
    self->internal->backgroundCells = newBackgroundCells;
    self->internal->sizeBackgroundCells *= 2;
  }

  /* Append the background instance data structure to our buffer */
  memcpy(
      &self->internal->backgroundCells[self->internal->numBackgroundCells++],
      &backgroundInstance,
      sizeof(st_ScreenRenderer_BackgroundInstance));
}

void st_ScreenRenderer_addGlyphInstance(
    st_ScreenRenderer *self,
    st_GlyphRenderer *glyphRenderer,
    uint32_t ch,
    int posx,
    int posy,
    const struct tsm_screen_attr *attr)
{
  st_ScreenRenderer_GlyphInstance glyphInstance;
  st_BoundingBox bbox;
  int atlasIndex, xOffset, yOffset;
  int error, result;

  /* Look for this glyph in our atlas */
  error = st_GlyphAtlas_getGlyph(&self->atlas,
      ch,  /* character */
      &bbox,  /* bbox */
      &xOffset,  /* xOffset */
      &yOffset,  /* yOffset */
      &atlasIndex  /* atlasTextureIndex */
      );
  if (error) {
    /* A glyph for the given character could not be found in the atlas */
    /* TODO: Try to add a glyph for this character */
    fprintf(stderr, "Could not find glyph for '0x%08x'\n", ch);
    return;
  }

  /* Set up the glyph instance data structure */
  glyphInstance.glyphSize[0] = (float)bbox.w;
  glyphInstance.glyphSize[1] = (float)bbox.h;
  glyphInstance.atlasPos[0] = (float)bbox.x;
  glyphInstance.atlasPos[1] = (float)bbox.y;
  glyphInstance.cell[0] = posx;
  glyphInstance.cell[1] = posy;
  glyphInstance.offset[0] = (float)xOffset;
  glyphInstance.offset[1] = (float)yOffset;
  /* FIXME: We need to determine which samplers are assigned for which atlas
   * textures */
//  glyphInstance.atlasIndex = 0;

  /* Determine the foreground color */
  if (attr->fccode < 0) {
    /* Use RGB for the foreground color */
    glyphInstance.fgColor[0] = attr->fr;
    glyphInstance.fgColor[1] = attr->fg;
    glyphInstance.fgColor[2] = attr->fb;
  } else {
    result = st_ScreenRenderer_colorCodeToRGB(self,
        attr->fccode,  /* code */
        glyphInstance.fgColor  /* rgb */
        );
    if (!result) {
      /* Use the default foreground color */
      /* TODO: Allow the user to configure the default foreground color */
      glyphInstance.fgColor[0] = 255;
      glyphInstance.fgColor[1] = 255;
      glyphInstance.fgColor[2] = 255;
    }
  }

  /* TODO: Implement bold glyph attributes */
  /* TODO: Implement inverse colors */

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
}

int st_ScreenRenderer_colorCodeToRGB(
    const st_ScreenRenderer *self,
    int8_t code,
    uint8_t *rgb)
{
  /* TODO: Allow the user to configure the color scheme */
  switch (code) {
#define CODE_RGB(code,r,g,b) \
    case code: \
      rgb[0] = r; \
      rgb[1] = g; \
      rgb[2] = b; \
      return 1;
    CODE_RGB(0, 0, 0, 0)  /* black */
    CODE_RGB(1, 255, 0, 0)  /* red */
    CODE_RGB(2, 0, 255, 0)  /* green */
    CODE_RGB(3, 255, 255, 0)  /* yellow */
    CODE_RGB(4, 0, 0, 255)  /* blue */
    CODE_RGB(5, 255, 0, 255)  /* magenta */
    CODE_RGB(6, 0, 255, 255)  /* cyan */
    CODE_RGB(7, 255, 255, 255)  /* white */
    default:
      /* TODO: Fail gracefully */
      /* FIXME: What are color codes 16 and 17? */
      fprintf(stderr, "Unknown color code: '%d'\n", code);
  }
  return 0;
}

void st_ScreenRenderer_drawBackgroundCells(
    const st_ScreenRenderer *self,
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

void st_ScreenRenderer_drawGlyphs(
    const st_ScreenRenderer *self,
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
  assert(ST_GLYPH_ATLAS_MAX_NUM_TEXTURES <= 16);
  GLuint atlasTextures[ST_GLYPH_ATLAS_MAX_NUM_TEXTURES];
  int numAtlasTextures, atlasSize;
  GLuint atlasLocation, cellSizeLocation, viewportSizeLocation,
         atlasSizeLocation;

  /* Use the glyph shader program */
  glUseProgram(self->internal->glyphShader);
  ASSERT_GL_ERROR();

  /* Configure the texture samplers */
  st_GlyphAtlas_getTextures(&self->atlas,
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
  atlasSize = st_GlyphAtlas_getTextureSize(&self->atlas);
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

void st_ScreenRenderer_draw(
    const st_ScreenRenderer *self,
    const st_GlyphRenderer *glyphRenderer,
    int viewportWidth, int viewportHeight)
{
  int cellSize[2];

  /* Get cell size from our glyph renderer */
  /* FIXME: The need for this call seems a little strange. Our glyphs have long
   * since been rendered to the glyph atlas. We should have simply stored the
   * cell size when the atlas was updated. */
  st_GlyphRenderer_getCellSize(glyphRenderer,
      &cellSize[0],  /* width */
      &cellSize[1]  /* height */
      );

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
  st_ScreenRenderer_drawBackgroundCells(self,
      cellSize[0],  /* cellWidth */
      cellSize[1],  /* cellHeight */
      viewportWidth,  /* viewportWidth */
      viewportHeight  /* viewportHeight */
      );

  /* Draw the glyphs on top of the background cells */
  st_ScreenRenderer_drawGlyphs(self,
      cellSize[0],  /* cellWidth */
      cellSize[1],  /* cellHeight */
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
