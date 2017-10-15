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

#include <math.h>

#include "fonts.h"
#include "logging.h"

#include "font.h"

/* Convert the fixed-point 26.6 format (a.k.a. 1/64th pixel format) to an
 * integer value in pixels, rounding up */
#define TWENTY_SIX_SIX_TO_PIXELS(value) ( \
    ((value) >> 6) + ((value) & ((1 << 6) - 1) ? 1 : 0))

/* Convert a floating point value to the fixed-point 26.6 format */
#define FLOAT_TO_TWENTY_SIX_SIX(value) ( \
    ((int)(trunc(value)) << 6) \
    + (int)trunc(((value) - trunc(value)) * (float)(1 << 6)))

const char *
ttoy_FreeTypeErrorString(
    FT_Error error)
{
#undef __FTERRORS_H__
#define FT_ERROR_START_LIST switch(error) {
#define FT_ERRORDEF(e, v, s) case e: return s;
#define FT_ERROR_END_LIST }
#include FT_ERRORS_H
  return "Unknown FreeType Error";
}

struct ttoy_Font_Internal_ {
  /* TODO: Somewhere in the FreeType documentation they suggest disposing of
   * FT_Face objects whenever possible. We tend to keep them for as long as the
   * font does not change. It might be best not to keep these objects around,
   * but I don't know. */
  FT_Face face;
  char *faceName, *fontPath;
  float size;
};

void
ttoy_Font_init(
    ttoy_Font *self)
{
  /* Allocate memory for internal structures */
  self->internal = (ttoy_Font_Internal *)malloc(sizeof(ttoy_Font_Internal));
  /* Fonts are not valid until FreeType font face has been loaded */
  self->internal->face = NULL;
  self->internal->faceName = NULL;
  self->internal->size = -1.0f;
}

ttoy_ErrorCode
ttoy_Font_load(
    ttoy_Font *self,
    const char *fontPath,
    const char *faceName,
    float fontSize,
    int x_dpi,
    int y_dpi)
{
  FT_Error ftError;
  FT_Library ft;

  ft = ttoy_Fonts_getFreeTypeInstance();

  /* Load the FreeType font face from file */
  ftError = FT_New_Face(
      ft,  /* library */
      fontPath,  /* filepathname */
      0,  /* face_index */
      &self->internal->face  /* aface */
      );
  if (ftError != FT_Err_Ok) {
    TTOY_LOG_ERROR("Freetype encountered an error reading file '%s': %s",
        fontPath,
        ttoy_FreeTypeErrorString(ftError));
    return TTOY_ERROR_FAILED_TO_LOAD_FONT;
  }

  /* Attempt to set the font size */
  ftError = FT_Set_Char_Size(
      self->internal->face,  /* face */
      0,  /* char_width */
      FLOAT_TO_TWENTY_SIX_SIX(fontSize),  /* char_height */
      x_dpi,  /* horz_resolution */
      y_dpi  /* vert_resolution */
      );
  if (ftError != FT_Err_Ok) {
    TTOY_LOG_ERROR("Freetype failed to set character size for font: %s",
        fontPath);
    /* TODO: Print the specific error message from Freetype */
    /* NOTE: Not a fatal error */
  }

  /* Store the size */
  self->internal->size = fontSize;
  /* Store the face name */
  self->internal->faceName = (char *)malloc(strlen(faceName) + 1);
  if (self->internal->faceName == NULL) {
    return TTOY_ERROR_OUT_OF_MEMORY;
  }
  strcpy(self->internal->faceName, faceName);

  return TTOY_NO_ERROR;
}

int
ttoy_Font_isValid(
    const ttoy_Font *self)
{
  /* Without a face loaded, the font is invalid */
  return self->internal->face != NULL;
}

void
ttoy_Font_destroy(
    ttoy_Font *self)
{
  if (self->internal->face != NULL) {
    /* Release the FreeType font face */
    FT_Done_Face(self->internal->face);
  }
  /* Free allocated memory */
  free(self->internal->faceName);
  free(self->internal);
}

int
ttoy_Font_hasCharacter(
    ttoy_Font *self,
    uint32_t character)
{
  if (self->internal->face == NULL)
    return 0;
  /* Look for the character in our FreeType font face */
  return FT_Get_Char_Index(
      self->internal->face,  /* face */
      character  /* charcode */
      ) != 0;
}

ttoy_ErrorCode
ttoy_Font_getGlyphDimensions(
    ttoy_Font *self,
    uint32_t character,
    int *width,
    int *height)
{
  int glyph_index;
  FT_Error ftError;

  /* Look for the glyph that corresponds with the given character code */
  glyph_index = FT_Get_Char_Index(self->internal->face, character);
  if (!glyph_index)
    return TTOY_ERROR_FONT_GLYPH_NOT_FOUND;  /* Error; could not find the glyph */

  /* Load the glyph */
  ftError = FT_Load_Glyph(
      self->internal->face,  /* face */
      glyph_index,  /* glyph_index */
      FT_LOAD_DEFAULT  /* load_flags */
      );
  if (ftError != FT_Err_Ok) {
    /* TODO: Print out the specific FreeType error */
    TTOY_LOG_ERROR(
        "Freetype error loading glyph for the character '0x%08x': %s",
        character,
        ttoy_FreeTypeErrorString(ftError));
    return TTOY_ERROR_FREETYPE_ERROR;
  }

  /* Calculate the pixel dimensions of this glyph, as we would render it */
  *width = 
    TWENTY_SIX_SIX_TO_PIXELS(self->internal->face->glyph->metrics.width);
  *height = 
    TWENTY_SIX_SIX_TO_PIXELS(self->internal->face->glyph->metrics.height);

  return TTOY_NO_ERROR;
}

ttoy_ErrorCode
ttoy_Font_getGlyphOffset(
    ttoy_Font *self,
    uint32_t character,
    int *x,
    int *y)
{
  FT_UInt glyph_index;
  FT_Face face;
  FT_Error ftError;

  face = self->internal->face;

  /* Look for the glyph that corresponds with the given character code */
  glyph_index = FT_Get_Char_Index(self->internal->face, character);
  if (!glyph_index)
    return TTOY_ERROR_FONT_GLYPH_NOT_FOUND;  /* Error; could not find the glyph */

  /* Load the glyph */
  ftError = FT_Load_Glyph(
      self->internal->face,  /* face */
      glyph_index,  /* glyph_index */
      FT_LOAD_DEFAULT  /* load_flags */
      );
  if (ftError != FT_Err_Ok) {
    /* TODO: Print out the specific FreeType error */
    TTOY_LOG_ERROR(
        "Freetype error loading glyph for the character '0x%08x'",
        character);
    return TTOY_ERROR_FREETYPE_ERROR;
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

  return TTOY_NO_ERROR;
}

ttoy_ErrorCode
ttoy_Font_renderGlyph(
    ttoy_Font *self,
    uint32_t character,
    FT_Bitmap **bitmap)
{
  int glyph_index;
  FT_Error ftError;

  /* Look for the glyph that corresponds with the given character code */
  glyph_index = FT_Get_Char_Index(self->internal->face, character);
  if (!glyph_index)
    return TTOY_ERROR_FONT_GLYPH_NOT_FOUND;  /* Error; could not find the glyph */

  /* Load the glyph */
  ftError = FT_Load_Glyph(
      self->internal->face,  /* face */
      glyph_index,  /* glyph_index */
      FT_LOAD_DEFAULT  /* load_flags */
      );
  if (ftError != FT_Err_Ok) {
    /* TODO: Print out the specific FreeType error */
    TTOY_LOG_ERROR(
        "Freetype error loading glyph for the character '0x%08x'",
        character);
    return TTOY_ERROR_FREETYPE_ERROR;
  }

  /* Render the glyph */
  ftError = FT_Render_Glyph(
      self->internal->face->glyph,
      FT_RENDER_MODE_NORMAL);
  if (ftError != FT_Err_Ok) {
    /* TODO: Print out the specific FreeType error */
    TTOY_LOG_ERROR("Freetype error rendering the glyph for '0x%08x'\n",
        character);
    return TTOY_ERROR_FREETYPE_ERROR;
  }

  *bitmap = &self->internal->face->glyph->bitmap;

  return TTOY_NO_ERROR;
}

const char *
ttoy_Font_getFaceName(
    const ttoy_Font *self)
{
  return (const char *)self->internal->faceName;
}

const char *
ttoy_Font_getFontPath(
    const ttoy_Font *self)
{
  return (const char *)self->internal->fontPath;
}

float
ttoy_Font_getSize(
    const ttoy_Font *self)
{
  return self->internal->size;
}

ttoy_ErrorCode
ttoy_Font_setSize(
    const ttoy_Font *self,
    float size)
{
  FT_Error ftError;

  /* Attempt to set the FreeType font size */
  ftError = FT_Set_Char_Size(
      self->internal->face,  /* face */
      0,  /* char_width */
      FLOAT_TO_TWENTY_SIX_SIX(size),  /* char_height */
      /* FIXME: Actually pass the DPI somehow */
      144,  /* horz_resolution */
      144  /* vert_resolution */
      );
  if (ftError != FT_Err_Ok) {
    TTOY_LOG_ERROR("Freetype failed to set character size for font '%s': %s",
        self->internal->fontPath,
        ttoy_FreeTypeErrorString(ftError));
    return TTOY_ERROR_FREETYPE_ERROR;
  }

  /* Store the new font size */
  self->internal->size = size;

  return TTOY_NO_ERROR;
}

FT_Face
ttoy_Font_getFtFace(
    ttoy_Font *self)
{
  return self->internal->face;
}
