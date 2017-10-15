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

#include <assert.h>
#include <fontconfig/fontconfig.h>
#include <math.h>
#include <stdlib.h>

#include "error.h"
#include "fonts.h"

/* Private methods */
void ttoy_Fonts_initFreetype();
void ttoy_Fonts_destroyFreetype();
void ttoy_Fonts_initFontconfig();
void ttoy_Fonts_destroyFontconfig();

typedef struct ttoy_Fonts_ {
  ttoy_MonospaceFontFace *monospaceFonts;
  size_t numMonospaceFonts, sizeMonospaceFonts;
  FT_Library ft;
  FcConfig *fcConfig;
} ttoy_Fonts;

#define TTOY_FONTS_INIT_MONOSPACE_FONTS_SIZE 4

ttoy_Fonts *ttoy_Fonts_instance() {
  static ttoy_Fonts instance;
  return &instance;
}

void ttoy_Fonts_init() {
  ttoy_Fonts *self = ttoy_Fonts_instance();

  /* Allocate internal buffers in ttoy_Fonts */
  self->monospaceFonts = (ttoy_MonospaceFontFace*)malloc(
      sizeof(void*) * TTOY_FONTS_INIT_MONOSPACE_FONTS_SIZE);
  self->sizeMonospaceFonts = TTOY_FONTS_INIT_MONOSPACE_FONTS_SIZE;
  self->numMonospaceFonts = 0;

  ttoy_Fonts_initFreetype();
  ttoy_Fonts_initFontconfig();
}

void ttoy_Fonts_destroy() {
  ttoy_Fonts_destroyFontconfig();
  ttoy_Fonts_destroyFreetype();
}

void ttoy_Fonts_initFreetype() {
  ttoy_Fonts *self = ttoy_Fonts_instance();

  /* Freetype library initialization */
  FT_Error error = FT_Init_FreeType(&self->ft);
  if (error != FT_Err_Ok) {
    fprintf(stderr, "Error initializing FreeType 2 library\n");
    /* TODO: Print out an error string for this specific error */
  }
}

void ttoy_Fonts_destroyFreetype() {
  ttoy_Fonts *self = ttoy_Fonts_instance();

  FT_Error error = FT_Done_FreeType(self->ft);
  if (error != FT_Err_Ok) {
    fprintf(stderr, "Error closing FreeType 2 library\n");
    /* TODO: Print out an error string for this specific error */
  }
}

void ttoy_Fonts_initFontconfig() {
  ttoy_Fonts *self = ttoy_Fonts_instance();

  self->fcConfig = FcInitLoadConfig();
  /* TODO: We might want to re-enable Fontconfig's cache in the future, but for
   * now disabling it avoids an assert in FcFini() */
  FcConfigBuildFonts(self->fcConfig);
}

void ttoy_Fonts_destroyFontconfig() {
  /* Finalize fontconfig library */
  FcFini();
}

FT_Library ttoy_Fonts_getFreeTypeInstance() {
  ttoy_Fonts *self = ttoy_Fonts_instance();

  return self->ft;
}

FcConfig *ttoy_Fonts_getFontconfigInstance() {
  ttoy_Fonts *self = ttoy_Fonts_instance();

  return self->fcConfig;
}

void ttoy_Fonts_calculateFacePixelBBox(
    const FT_Face face,
    int *width, int *height)
{
  double x_pixels_per_unit, y_pixels_per_unit, units_per_em;
  /* Compute a scaling factor to convert from font units to pixels, as
   * described in
   * <https://www.freetype.org/freetype2/docs/tutorial/step2.html> */
  units_per_em = (double)face->units_per_EM;
  x_pixels_per_unit = face->size->metrics.x_ppem / units_per_em;
  y_pixels_per_unit = face->size->metrics.y_ppem / units_per_em;
  *width = (int)ceil(
      (double)(face->bbox.xMax - face->bbox.xMin) * x_pixels_per_unit);
  *height = (int)ceil(
      (double)(face->bbox.yMax - face->bbox.yMin) * y_pixels_per_unit);
}

#define TTOY_MONOSPACE_FONT_FACE_INIT_SIZE_GLYPHS 256

void ttoy_MonospaceFontFace_init(ttoy_MonospaceFontFace *self) {
  self->sizeGlyphs = TTOY_MONOSPACE_FONT_FACE_INIT_SIZE_GLYPHS;
  self->glyphs = (ttoy_MonospaceGlyph*)malloc(
      sizeof(ttoy_MonospaceGlyph) * self->sizeGlyphs);
  self->numGlyphs = 0;
}

void ttoy_MonospaceFontFace_loadGlyph(
    ttoy_MonospaceFontFace *self,
    FT_Face face,
    uint32_t ch)
{
  FT_Error error;
  int glyph_index;

  /* Use the charmap in our face to get the actual glyph for this ASCII
   * character code */
  glyph_index = FT_Get_Char_Index(face, ch);
  if (glyph_index == 0) {
    fprintf(stderr,
        "Warning: Freetype could not find a glyph for the character '%c'\n",
        (char)ch);
  }
  error = FT_Load_Glyph(
      face,  /* face */
      glyph_index,  /* glyph_index */
      FT_LOAD_DEFAULT  /* load_flags */
      );
  if (error != FT_Err_Ok) {
    fprintf(stderr,
        "Freetype encountered an error loading the glyph for '%c'\n",
        (char)ch);
  }
  error = FT_Render_Glyph(
      face->glyph,  /* slot */
      FT_RENDER_MODE_MONO  /* render_mode */
      );
  if (error != FT_Err_Ok) {
    fprintf(stderr,
        "Freetype encountered an error rendering the glyph for '%c'\n",
        (char)ch);
  }
  /* Print out the glyph bitmap */
  ttoy_printGlyphDebug(&face->glyph->bitmap);
}

void ttoy_printGlyphDebug(const FT_Bitmap *bitmap) {
  /* Print out the bitmap as ascii text */
  for (int row = 0; row < bitmap->rows; ++row) {
    for (int column = 0; column < bitmap->width; ++column) {
      int buffer_index = row * abs(bitmap->pitch) + column / 8;
      int bit_index = 7 - (column % 8);
      fprintf(stderr, "%c",
          bitmap->buffer[buffer_index] & (1 << bit_index) ? '#' : ' ');
    }
    fprintf(stderr, "\n");
  }
}

void ttoy_printAntiAliasedGlyphDebug(const FT_Bitmap *bitmap) {
  /* Print out the bitmap as ascii text */
  for (int row = 0; row < bitmap->rows; ++row) {
    for (int column = 0; column < bitmap->width; ++column) {
      int buffer_index = row * abs(bitmap->pitch) + column;
      fprintf(stderr, "%c",
          bitmap->buffer[buffer_index] == 0 ? '#' : ' ');
    }
    fprintf(stderr, "\n");
  }
}

ttoy_MonospaceGlyph *ttoy_MonospaceFontFace_getGlyph(
    ttoy_MonospaceFontFace *self,
    uint32_t ch)
{
  size_t i, a, b;

  /* Binary search for the glyph */
  a = 0; b = self->numGlyphs;
  i = self->numGlyphs / 2;
  while (a != b) {
    if (ch < self->glyphs[i].ch) {
      b = i;
      i = a + (b - a) / 2;
    } else if (ch > self->glyphs[i].ch) {
      a = i;
      i = a + (b - a) / 2;
    } else {
      break;
    }
  }
  if ((i >= self->numGlyphs) || (ch != self->glyphs[i].ch)) {
    /* Couldn't find this glyph */
    /* TODO: We need to try to render this glyph from our fonts */
    return NULL;
  }

  return &self->glyphs[i];
}

/* TODO: Generate signed-distance-field textures */
/* TODO: Arrange glyphs into an atlas */
/* TODO: Define a data structure for keeping glyph information */
/* TODO: Write a shader that can transform glyph geometry and draw the glyphs
 * using instanced rendering */
/* TODO: Support multiple font faces */
/* TODO: Support multiple sizes */
