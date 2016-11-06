#include "common/shaders.h"
#include "render.h"

/* Private methods */
void st_RenderContext_initShaders(st_RenderContext *self);
void st_RenderContext_initVAO(st_RenderContext *self);

#define ST_RENDER_CONTEXT_INIT_SIZE_GLYPHS (80 * 24)

void st_RenderContext_init(st_RenderContext *self) {
  self->sizeGlyphs = ST_RENDER_CONTEXT_INIT_SIZE_GLYPHS;
  self->glyphs = (st_Render_GlyphInstance*)malloc(
      sizeof(st_Render_GlyphInstance) * self->sizeGlyphs);
  self->numGlyphs = 0;
  self->rows = 0;
  self->cols = 0;
  st_RenderContext_initShaders(self);
  st_RenderContext_initVAO(self);
}

void st_RenderContext_initShaders(st_RenderContext *self) {
  self->glyphShader = st_Shaders_glyphShader();
}

void st_RenderContext_initVAO(st_RenderContext *self) {
  GLuint glyphLocation;

  /* TODO: Check OpenGL error output */
  glGenVertexArrays(1, &self->glyphVAO);
  glBindVertexArray(self->glyphVAO);

  glyphLocation = glGetAttribLocation(self->glyphShader, "glyph");
}

void st_RenderContext_glyphDrawCallback(
  struct tsm_screen *con,
  uint32_t id,
  const uint32_t *ch,
  size_t len,
  unsigned int width,
  unsigned int posx,
  unsigned int posy,
  const struct tsm_screen_attr *attr,
  tsm_age_t age,
  st_RenderContext *self)
{
  st_MonospaceGlyph *glyph;
  st_Render_GlyphInstance glyphInstance;

  /* Retrieve the glyph from our font */
  glyph = st_MonospaceFontFace_getGlyph(self->font, *ch);

  /* Skip missing glyphs */
  if (glyph == NULL) {
    fprintf(stderr, "Could not find glyph for '0x%08x'", *ch);
    return;
  }

  if (self->numGlyphs + 1 > self->sizeGlyphs) {
    st_Render_GlyphInstance *newGlyphs;

    /* Double the number of glyphs to ensure we have enough space */
    newGlyphs = (st_Render_GlyphInstance*)malloc(
        sizeof(st_Render_GlyphInstance) * self->sizeGlyphs * 2);
    memcpy(newGlyphs, self->glyphs,
        sizeof(st_Render_GlyphInstance) * self->sizeGlyphs);
    free(self->glyphs);
    self->glyphs = newGlyphs;
    self->sizeGlyphs *= 2;
  }

  /* Set up the glyph instance data structure */
  glyphInstance.row = posy;
  glyphInstance.col = posx;
  glyphInstance.xOffset = glyph->xOffset;
  glyphInstance.yOffset = glyph->yOffset;
  glyphInstance.aaPos = glyph->aaPos;
  glyphInstance.sdfPos = glyph->sdfPos;
  /* TODO: What to set for the aaAtlasSampler and sdfAtlasSampler? */

  /* Append the glyph instance data structure to our buffer */
  memcpy(self->glyphs + self->numGlyphs, &glyphInstance,
      sizeof(st_Render_GlyphInstance));
  self->numGlyphs += 1;

  /* TODO: Update the OpenGL buffers backing the instance rendered text */
  fprintf(stderr, "Printing character: '%c' at location (%d, %d)\n",
      (char)(*ch),
      posx,
      posy);
}

void st_RenderContext_updateTerminalText(
    st_RenderContext *self,
    struct tsm_screen *screen)
{
  /* TODO: Make a glyph instance buffer to send to the GL */
  self->rows = tsm_screen_get_height(screen);
  self->cols = tsm_screen_get_width(screen);
  self->numGlyphs = 0;
  /* FIXME: How do we account for glyphs that take up two cells? */
  /* Fill the glyph instance buffer with the latest screen contents */
  tsm_screen_draw(
      screen,  /* con */
      (tsm_screen_draw_cb)st_RenderContext_glyphDrawCallback,  /* draw_cb */
      self  /* data */
      );
  /* TODO: Check for GL errors */
  /* Send the glyph instance buffer to the GL */
  glBindBuffer(GL_ARRAY_BUFFER, self->glyphInstanceBuffer);
  glBufferData(
      GL_ARRAY_BUFFER,  /* target */
      sizeof(st_Render_GlyphInstance) * self->numGlyphs,  /* size */
      self->glyphs,  /* data */
      GL_DYNAMIC_DRAW  /* usage */
      );
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}
