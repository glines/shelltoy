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
#include <math.h>

#include "fontRefArray.h"
#include "fonts.h"
#include "logging.h"

#include "glyphRenderer.h"

/* Convert the fixed-point 26.6 format (a.k.a. 1/64th pixel format) to an
 * integer value in pixels, rounding up */
#define TWENTY_SIX_SIX_TO_PIXELS(value) ( \
    ((value) >> 6) + ((value) & ((1 << 6) - 1) ? 1 : 0))

/* Private methods */
ttoy_ErrorCode
ttoy_GlyphRenderer_calculateCellSize(
    ttoy_GlyphRenderer *self,
    int *width, int *height);
ttoy_ErrorCode
ttoy_GlyphRenderer_calculateUnderlineOffset(
    ttoy_GlyphRenderer *self,
    int *offset);

struct ttoy_GlyphRenderer_Internal {
  ttoy_FontRefArray fonts;
  ttoy_FontRefArray boldFonts;
  int cellSize[2];
  int underlineOffset;
  float fontSize;
};

void ttoy_GlyphRenderer_init(
    ttoy_GlyphRenderer *self,
    ttoy_Profile *profile)
{
  ttoy_ErrorCode error;

  /* Allocate internal data structures */
  self->internal = (struct ttoy_GlyphRenderer_Internal*)malloc(
      sizeof(struct ttoy_GlyphRenderer_Internal));
  ttoy_FontRefArray_init(&self->internal->fonts);
  ttoy_FontRefArray_init(&self->internal->boldFonts);

  /* Get the current font lists from the profile */
  ttoy_Profile_getFonts(profile,
      &self->internal->fonts,  /* fonts */
      &self->internal->boldFonts  /* boldFonts */
      );

  /* Calculate the cell size for the given fonts */
  error = ttoy_GlyphRenderer_calculateCellSize(self,
      &self->internal->cellSize[0],  /* width */
      &self->internal->cellSize[1]  /* height */
      );
  if (error != TTOY_NO_ERROR) {
    TTOY_LOG_ERROR_CODE(error);
  }

  /* Calculate the underline offset for the given fonts */
  error = ttoy_GlyphRenderer_calculateUnderlineOffset(self,
      &self->internal->underlineOffset  /* offset */
      );
  if (error != TTOY_NO_ERROR) {
    TTOY_LOG_ERROR_CODE(error);
  }

  /* Store the font size */
  self->internal->fontSize = profile->fontSize;
}

/** Each FreeType font face defines a "bounding box" that encloses all glyphs.
 * The atlas does not use this information, since it packs glyphs as tightly as
 * possible, but we do need to compute bounding box information for the
 * terminal. */
ttoy_ErrorCode
ttoy_GlyphRenderer_calculateCellSize(
    ttoy_GlyphRenderer *self,
    int *width, int *height)
{
  double y_pixels_per_unit, units_per_em;
  FT_UInt glyph_index;
  FT_Error error;
  ttoy_Font *font;
  FT_Face face;

  /* Use the character 'M' for approximating the cell dimensions */
#define CELL_REFERENCE_CHAR 'M'

  /* Get the font providing our reference character */
  error = ttoy_GlyphRenderer_getFont(self,
      CELL_REFERENCE_CHAR,  /* character */
      0,  /* bold */
      &font,  /* font */
      NULL  /* fondIndex */
      );
  if (error != TTOY_NO_ERROR) {
    return error;
  }

  /* Get the FreeType face handle */
  face = ttoy_Font_getFtFace(font);

  /* Compute a scaling factor to convert from font units to pixels, as
   * described in
   * <https://www.freetype.org/freetype2/docs/tutorial/step2.html> */
  units_per_em = (double)face->units_per_EM;
  y_pixels_per_unit = face->size->metrics.y_ppem / units_per_em;

  /* Horizontal distance between glyphs is estimated by looking at the glyph
   * for our reference character.  bbox is not appropriate here since some
   * monospace fonts contain characters far wider than expected. */
  glyph_index = FT_Get_Char_Index(
      face,  /* face */
      (FT_ULong)CELL_REFERENCE_CHAR  /* charcode */
      );
  if (glyph_index == 0) {
    /* This should never happen... */
    TTOY_LOG_ERROR("No fonts provide the character '%c'",
        CELL_REFERENCE_CHAR);
    return TTOY_ERROR_MISSING_FONT_FOR_CHARACTER_CODE;
  }
  error = FT_Load_Glyph(
      face,  /* face */
      glyph_index,  /* glyph_index */
      FT_LOAD_DEFAULT  /* load_flags */
      );
  if (error != FT_Err_Ok) {
    TTOY_LOG_ERROR(
        "Freetype error loading the glyph for ASCII character '%c'",
        CELL_REFERENCE_CHAR);
    return TTOY_ERROR_FREETYPE_ERROR;
  }
  *width = TWENTY_SIX_SIX_TO_PIXELS(face->glyph->metrics.horiAdvance);

  /* Distance between baselines is given by the height member of FT_FaceRec */
  *height = (int)ceil((double)face->height * y_pixels_per_unit);

  /* NOTE: The following calculation also seems to work well */
/*  *height = TWENTY_SIX_SIX_TO_PIXELS(face->size->metrics.height); */

  return TTOY_NO_ERROR;
}

ttoy_ErrorCode
ttoy_GlyphRenderer_calculateUnderlineOffset(
    ttoy_GlyphRenderer *self,
    int *offset)
{
  /* TODO: Actually calculate the underline offset */
  *offset = 2;

  return TTOY_NO_ERROR;
}

void ttoy_GlyphRenderer_destroy(
    ttoy_GlyphRenderer *self)
{
  /* Decrement all font references */
#define DEC_FONT_REFS(ARRAY) \
  for (size_t i = 0; \
      i < ttoy_FontRefArray_size(&self->internal->ARRAY); \
      ++i) \
  { \
    ttoy_FontRef *fontRef; \
    fontRef = ttoy_FontRefArray_get(&self->internal->ARRAY, i); \
    ttoy_FontRef_decrement(fontRef); \
  }
  DEC_FONT_REFS(fonts)
  DEC_FONT_REFS(boldFonts)
  /* Destroy font arrays */
  ttoy_FontRefArray_destroy(&self->internal->fonts);
  ttoy_FontRefArray_destroy(&self->internal->boldFonts);
  /* Free internal data structures */
  free(self->internal);
}

void ttoy_GlyphRenderer_getCellSize(
    const ttoy_GlyphRenderer *self,
    int *width, int *height)
{
  *width = self->internal->cellSize[0];
  *height = self->internal->cellSize[1];
}

void ttoy_GlyphRenderer_getUnderlineOffset(
    ttoy_GlyphRenderer *self,
    int *offset)
{
  *offset = self->internal->underlineOffset;
}

ttoy_ErrorCode
ttoy_GlyphRenderer_getFont(
    ttoy_GlyphRenderer *self,
    uint32_t character,
    int bold,
    ttoy_Font **font,
    int *fontIndex)
{
  ttoy_FontRefArray *fonts;
  int firstTry = 1;
  int foo;

  /* TODO: I don't know how well FcCharSet or FC_Face performs. We might be
   * able to improve performance here by caching these results */

  if (fontIndex == NULL) {
    /* Discard font index result */
    fontIndex = &foo;
  }

  if (bold) {
    /* Start looking for bold fonts */
    fonts = &self->internal->boldFonts;
    /* Start the font index past the ordinary fonts */
    *fontIndex = ttoy_FontRefArray_size(&self->internal->fonts);
  } else {
    /* Start looking for ordinary fonts */
    fonts = &self->internal->fonts;
    /* Start the font index at zero */
    *fontIndex = 0;
  }

  do {
    /* Iterate over our list of fonts, looking for the first font that provides
     * the given character */
    for (size_t i = 0;
        i < ttoy_FontRefArray_size(fonts);
        ++i)
    {
      *font = ttoy_FontRef_get(ttoy_FontRefArray_get(fonts, i));
      if (ttoy_Font_hasCharacter(*font, character)) {
        /* Found a suitable font */
        /* Adjust the font index by the index into this font array */
        *fontIndex += i;
        return TTOY_NO_ERROR;
      }
    }

    if (bold) {
      /* Look for ordinary fonts out of desperation */
      fonts = &self->internal->fonts;
      /* Start the font index at zero */
      *fontIndex = 0;
    } else {
      /* Look for bold fonts out of desperation */
      fonts = &self->internal->boldFonts;
      /* Start the font index past the ordinary fonts */
      *fontIndex = ttoy_FontRefArray_size(&self->internal->fonts);
    }
  } while (firstTry--);

  /* No suitable font was found */
  *font = NULL;
  *fontIndex = -1;

  return TTOY_ERROR_MISSING_FONT_FOR_CHARACTER_CODE;
}

ttoy_ErrorCode
ttoy_GlyphRenderer_getFontIndex(
    ttoy_GlyphRenderer *self,
    uint32_t character,
    int bold,
    int *fontIndex)
{
  ttoy_Font *font;
  ttoy_ErrorCode error;

  error = ttoy_GlyphRenderer_getFont(self,
      character,  /* character */
      bold,  /* bold */
      &font,  /* font */
      fontIndex  /* fontIndex */
      );

  return error;
}

ttoy_ErrorCode
ttoy_GlyphRenderer_getGlyphDimensions(
    ttoy_GlyphRenderer *self,
    uint32_t character,
    int bold,
    int *width,
    int *height)
{
  ttoy_ErrorCode error;
  ttoy_Font *font;

  /* Get the font that provides this character's glyph */
  error = ttoy_GlyphRenderer_getFont(self,
      character,  /* character */
      bold,  /* bold */
      &font,  /* font */
      NULL  /* fontIndex */
      );
  if (error != TTOY_NO_ERROR) {
    return error;
  }

  /* Get the dimensions of the glyph from our font */
  error = ttoy_Font_getGlyphDimensions(font,
      character,  /* character */
      width,  /* width */
      height  /* height */
      );
  if (error != TTOY_NO_ERROR) {
    return error;
  }

  return TTOY_NO_ERROR;
}

ttoy_ErrorCode
ttoy_GlyphRenderer_getGlyphOffset(
    ttoy_GlyphRenderer *self,
    uint32_t character,
    int bold,
    int *x,
    int *y)
{
  ttoy_Font *font;
  ttoy_ErrorCode error;

  /* Get the font that provides this character's glyph */
  error = ttoy_GlyphRenderer_getFont(self,
      character,  /* character */
      bold,  /* bold */
      &font,  /* font */
      NULL  /* fontIndex */
      );
  if (error != TTOY_NO_ERROR) {
    return error;
  }

  /* Get the offset of the glyph from our font */
  error = ttoy_Font_getGlyphOffset(font,
      character,  /* character */
      x,  /* x */
      y  /* y */
      );
  if (error != TTOY_NO_ERROR) {
    return error;
  }

  return TTOY_NO_ERROR;
}

ttoy_ErrorCode
ttoy_GlyphRenderer_renderGlyph(
    ttoy_GlyphRenderer *self,
    uint32_t character,
    int bold,
    FT_Bitmap **bitmap,
    int *fontIndex)
{
  ttoy_Font *font;
  ttoy_ErrorCode error;

  /* Determine which font provides the glyph for this character code */
  error = ttoy_GlyphRenderer_getFont(self,
      character,  /* character */
      bold,  /* bold */
      &font,  /* font */
      fontIndex  /* fontIndex */
      );
  if (error != TTOY_NO_ERROR) {
    return error;
  }

  /* Render the glyph for this character code with our font */
  error = ttoy_Font_renderGlyph(font,
      character,  /* character */
      bitmap  /* bitmap */
      );
  if (error != TTOY_NO_ERROR) {
    return error;
  }

  return TTOY_NO_ERROR;
}
