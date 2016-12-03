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

#include <assert.h>
#include <fontconfig/fontconfig.h>
#include <math.h>

#include "fonts.h"

#include "glyphRenderer.h"

/* We convert the fixed-point 26.6 format (a.k.a. 1/64th pixel format) to an
 * integer value in pixels, rounding up */
#define TWENTY_SIX_SIX_TO_PIXELS(value) ( \
    ((value) >> 6) + ((value) & ((1 << 6) - 1) ? 1 : 0))

/* Private methods */
void st_GlyphRenderer_buildFontPattern(
    st_GlyphRenderer *self,
    const char *fontFace,
    float fontSize);
void st_GlyphRenderer_updateCellSize(
    st_GlyphRenderer *self);
void st_GlyphRenderer_calculateCellSize(
    const FT_Face face,
    int *width, int *height);

struct st_GlyphRenderer_Internal {
  /* TODO: Support multiple font faces in this structure */
  /* TODO: Somewhere in the FreeType documentation they suggest disposing of
   * FT_Face objects whenever possible. It might be best not to keep these
   * objects around, but I don't know. */
  FT_Face face;
  int cellSize[2];
};

void st_GlyphRenderer_init(
    st_GlyphRenderer *self,
    const char *fontFace,
    float fontSize)
{
  /* FIXME: Get rid of this path */
#define FONT_FACE_PATH "/nix/store/fvwp39z54ka2s7h3gawhfmayrqjnd05a-dejavu-fonts-2.37/share/fonts/truetype/DejaVuSansMono.ttf"

  FT_Error error;
  FT_Library ft;
  const char *fontPath = FONT_FACE_PATH;

  /* TODO: Determine the font path using fontconfig */
  st_GlyphRenderer_buildFontPattern(self,
      fontFace,
      fontSize);

  ft = st_Fonts_getFreeTypeInstance();

  /* Allocate internal data structures */
  self->internal = (struct st_GlyphRenderer_Internal*)malloc(
      sizeof(struct st_GlyphRenderer_Internal));

  /* TODO: Support configuration of the font face to load. This might require
   * an st_GlyphRenderer_addFace() method. */
  error = FT_New_Face(
      ft,  /* library */
      fontPath,  /* filepathname */
      0,  /* face_index */
      &self->internal->face  /* aface */
      );
  if (error == FT_Err_Unknown_File_Format) {
    fprintf(stderr, "Freetype encountered an unknown file format: %s\n",
        fontPath);
    /* TODO: Print the specific error message from Freetype */
  } else if (error != FT_Err_Ok) {
    fprintf(stderr, "Freetype encountered an error reading file: %s\n",
        fontPath);
    /* TODO: Print the specific error message from Freetype */
  }
  /* TODO: Fail gracefully? */
  /* TODO: Support setting different font sizes */
  /* TODO: Support differentiation between bitmap and vector fonts here */
  /* TODO: Try out using FT_SIZE_REQUEST_TYPE_CELL, since it is obviously
   * intended for terminal emulator applications. */
  error = FT_Set_Char_Size(
      self->internal->face,  /* face */
      0,  /* char_width */
      4 << 6,  /* char_height */
      300,  /* horz_resolution */
      300  /* vert_resolution */
      );
  if (error != FT_Err_Ok) {
    fprintf(stderr,
        "Freetype failed to set dimensions for font: %s\n",
        fontPath);
    /* TODO: Fail gracefully? */
  }
  st_GlyphRenderer_updateCellSize(self);
}

void st_GlyphRenderer_buildFontPattern(
    st_GlyphRenderer *self,
    const char *fontFace,
    float fontSize)
{
  FcPattern *pattern;
  FcFontSet *sourceFontSets[2];
  FcFontSet *resultFontSet;
  FcConfig *config;
  FcResult result;
  FcValue value;
  char *fontFilePath;

  config = st_Fonts_getFontconfigInstance();

  sourceFontSets[0] = FcConfigGetFonts(config, FcSetSystem);
  sourceFontSets[1] = FcConfigGetFonts(config, FcSetApplication);

  pattern = FcPatternBuild(
      NULL,  /* pattern */
      FC_SPACING, FcTypeInteger, FC_MONO,  /* Look for monospace fonts */
      (char *) NULL  /* terminator */
      );

  /* Generate a list of all fonts matching our criteria */
  resultFontSet = FcFontSetList(
      config,  /* config */
      sourceFontSets,  /* sets */
      2,  /* nsets */
      pattern,  /* pattern */
      /* FIXME: Fontconfig documentation is not clear on the utility of
       * object_set */
      NULL  /* object_set */
      );

  /* XXX: Print out all of the matching fonts */
  for (int i = 0; i < resultFontSet->nfont; ++i) {
    FcPattern *font;
    font = resultFontSet->fonts[i];
    FcPatternPrint(font);
  }

  if (resultFontSet->nfont <= 0) {
    fprintf(stderr, "Error: Fontconfig could not find any suitable fonts.\n");
    /* TODO: Fail gracefully */
    assert(0);
  }

  /* Get the file path of the matching font */
  result = FcPatternGet(resultFontSet->fonts[0], FC_FILE, 0, &value);
  assert(value.u.s != NULL);
  fontFilePath = (char *)malloc(strlen(value.u.s) + 1);
  strcpy(fontFilePath, value.u.s);
  fprintf(stderr, "Fontconfig found suitable font: '%s'\n",
      fontFilePath);
  free(fontFilePath);

  FcPatternDestroy(pattern);
}

void st_GlyphRenderer_updateCellSize(
    st_GlyphRenderer *self)
{
  FT_Face face;

  face = self->internal->face;

  /* Calculate the cell size for the current base font */
  st_GlyphRenderer_calculateCellSize(
      face,  /* face */
      &self->internal->cellSize[0],  /* width */
      &self->internal->cellSize[1]  /* height */
      );
}

/** Each FreeType font face defines a "bounding box" that encloses all glyphs.
 * The atlas does not use this information, since it packs glyphs as tightly as
 * possible, but we do need to compute bounding box information for the
 * terminal. */
void st_GlyphRenderer_calculateCellSize(
    const FT_Face face,
    int *width, int *height)
{
  double x_pixels_per_unit, y_pixels_per_unit, units_per_em;
  FT_UInt glyph_index;
  FT_Error error;

  /* Compute a scaling factor to convert from font units to pixels, as
   * described in
   * <https://www.freetype.org/freetype2/docs/tutorial/step2.html> */
  units_per_em = (double)face->units_per_EM;
  x_pixels_per_unit = face->size->metrics.x_ppem / units_per_em;
  y_pixels_per_unit = face->size->metrics.y_ppem / units_per_em;

  /* Horizontal distance between glyphs is estimated by looking at the glyph
   * for 'M'.  bbox is not appropriate here since some monospace fonts contain
   * characters far wider than expected. */
  glyph_index = FT_Get_Char_Index(face, (FT_ULong)'M');
  if (glyph_index == 0) {
    fprintf(stderr, "Font does not contain the character 'M'\n");
    /* TODO: Fail gracefully */
    assert(0);
  }
  error = FT_Load_Glyph(
      face,  /* face */
      glyph_index,  /* glyph_index */
      FT_LOAD_DEFAULT  /* load_flags */
      );
  if (error != FT_Err_Ok) {
    fprintf(stderr,
        "Freetype error loading the glyph for ASCII character 'M'\n");
    /* TODO: Fail gracefully */
    assert(0);
  }
  *width = TWENTY_SIX_SIX_TO_PIXELS(face->glyph->metrics.horiAdvance);

  /* Distance between baselines is given by the height member of FT_FaceRec */
  *height = (int)ceil((double)face->height * y_pixels_per_unit);

  /* NOTE: The following calculation also seems to work well */
/*  *height = TWENTY_SIX_SIX_TO_PIXELS(face->size->metrics.height); */
}

void st_GlyphRenderer_destroy(
    st_GlyphRenderer *self)
{
  /* Free internal data structures */
  FT_Done_Face(self->internal->face);
  free(self->internal);
}

void st_GlyphRenderer_getCellSize(
    const st_GlyphRenderer *self,
    int *width, int *height)
{
  *width = self->internal->cellSize[0];
  *height = self->internal->cellSize[1];
}

int st_GlyphRenderer_getGlyphDimensions(
    st_GlyphRenderer *self,
    uint32_t character,
    int *width, int *height)
{
  int glyph_index;
  FT_Error error;
  /* TODO: Look for a face that provides this glyph (once we add support for
   * multiple faces) */
  /* Check for a corresponding glyph provided by this face */
  glyph_index = FT_Get_Char_Index(self->internal->face, character);
  if (!glyph_index)
    return 1;  /* Error; could not find the glyph */
  /* Load the glyph */
  error = FT_Load_Glyph(
      self->internal->face,  /* face */
      glyph_index,  /* glyph_index */
      FT_LOAD_DEFAULT  /* load_flags */
      );
  if (error != FT_Err_Ok) {
    fprintf(stderr,
        "Freetype error loading the glyph for ASCII character '%c'\n",
        (char)character);
  }
  /* Calculate the pixel dimensions of this glyph, as we would render it */
  *width = 
    TWENTY_SIX_SIX_TO_PIXELS(self->internal->face->glyph->metrics.width);
  *height = 
    TWENTY_SIX_SIX_TO_PIXELS(self->internal->face->glyph->metrics.height);
  return 0;
}

FT_Bitmap *st_GlyphRenderer_renderGlyph(
    st_GlyphRenderer *self,
    uint32_t character)
{
  int glyph_index;
  FT_Error error;

  /* Render the glyph for this character */
  glyph_index = FT_Get_Char_Index(self->internal->face, character);
  assert(glyph_index != 0);
  error = FT_Load_Glyph(
      self->internal->face,  /* face */
      glyph_index,  /* glyph_index */
      FT_LOAD_DEFAULT  /* load_flags */
      );
  assert(error == FT_Err_Ok);
  error = FT_Render_Glyph(
      self->internal->face->glyph,
      FT_RENDER_MODE_NORMAL);
  if (error != FT_Err_Ok) {
    fprintf(stderr,
        "Freetype encountered an error rendering the glyph for '0x%08x'\n",
        character);
    /* TODO: Fail gracefully */
    assert(0);
  }
  return &self->internal->face->glyph->bitmap;
}

void st_GlyphRenderer_getGlyphOffset(
    st_GlyphRenderer *self,
    uint32_t character,
    int *x, int *y)
{
  double x_pixels_per_unit, y_pixels_per_unit, units_per_em;
  FT_UInt glyph_index;
  FT_Face face;
  FT_Error error;

  face = self->internal->face;

  /* Compute a scaling factor to convert from font units to pixels, as
   * described in
   * <https://www.freetype.org/freetype2/docs/tutorial/step2.html> */
  /* TODO: Cache this result */
  units_per_em = (double)face->units_per_EM;
  x_pixels_per_unit = face->size->metrics.x_ppem / units_per_em;
  y_pixels_per_unit = face->size->metrics.y_ppem / units_per_em;

  /* TODO: Look for a face that provides this glyph (once we add support for
   * multiple faces) */
  glyph_index = FT_Get_Char_Index(face, character);
  assert(glyph_index);
  /* Load the glyph */
  error = FT_Load_Glyph(
      face,  /* face */
      glyph_index,  /* glyph_index */
      FT_LOAD_DEFAULT  /* load_flags */
      );
  if (error != FT_Err_Ok) {
    fprintf(stderr,
        "Freetype error loading the glyph for ASCII character '%c'\n",
        (char)character);
    /* TODO: Fail gracefully */
    assert(0);
  }
  /* Calculate the horizontal offset of this glyph */
  *x = TWENTY_SIX_SIX_TO_PIXELS(face->glyph->metrics.horiBearingX);
  /* Calculate the vertical offset of this glyph */
  FT_Pos linegap = face->size->metrics.height
    - face->size->metrics.ascender
    + face->size->metrics.descender;
  *y = TWENTY_SIX_SIX_TO_PIXELS(
      face->glyph->metrics.horiBearingY
      - face->size->metrics.descender
      - face->glyph->metrics.height
      - linegap / 2);  /* Distribute the linegap above and below the glyph */
}
