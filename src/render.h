#ifndef SHELLTOY_RENDER_H_
#define SHELLTOY_RENDER_H_

#include <GL/glew.h>
#include <libtsm.h>

#include "fonts.h"

typedef struct {
  st_AtlasPos aaPos, sdfPos;
  GLuint aaAtlasSampler, sdfAtlasSampler;
  int xOffset, yOffset;
  int row, col;
} st_Render_GlyphInstance;

typedef struct {
  st_Render_GlyphInstance *glyphs;
  size_t numGlyphs, sizeGlyphs;
  GLuint glyphInstanceBuffer, glyphVAO;
  st_MonospaceFontFace *font;
  unsigned int rows, cols;
  GLuint glyphShader;
} st_RenderContext;

void st_RenderContext_init(st_RenderContext *self);

void st_RenderContext_updateTerminalText(
    st_RenderContext *self,
    struct tsm_screen *screen);

#endif
