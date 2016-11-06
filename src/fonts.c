#include <assert.h>
#include <stdlib.h>

#include "fonts.h"

typedef struct {
  st_MonospaceFontFace *monospaceFonts;
  size_t numMonospaceFonts, sizeMonospaceFonts;
  FT_Library ft;
} st_Fonts;

#define ST_FONTS_INIT_MONOSPACE_FONTS_SIZE 4

st_Fonts *st_Fonts_instance() {
  static st_Fonts instance;
  return &instance;
}

void st_Fonts_init() {
  st_Fonts *self = st_Fonts_instance();

  /* Allocate internal buffers in st_Fonts */
  self->monospaceFonts = (st_MonospaceFontFace*)malloc(
      sizeof(void*) * ST_FONTS_INIT_MONOSPACE_FONTS_SIZE);
  self->sizeMonospaceFonts = ST_FONTS_INIT_MONOSPACE_FONTS_SIZE;
  self->numMonospaceFonts = 0;

  st_Fonts_initFreetype();
}

void st_Fonts_initFreetype() {
  st_Fonts *self = st_Fonts_instance();

  /* Freetype library initialization */
  FT_Error error = FT_Init_FreeType(&self->ft);
  if (error != FT_Err_Ok) {
    fprintf(stderr, "Error initializing FreeType 2 library\n");
    /* TODO: Print out an error string for this specific error */
  }
}

/* TODO: Support loading different faces from files that happen to have more
 * than one face */
st_MonospaceFontFace *st_Fonts_loadMonospace(
    int width, int height,
    const char *fontPath)
{
  FT_Face face;
  FT_Error error;

  st_Fonts *self = st_Fonts_instance();

  /* Attempt to load the font from file */
  error = FT_New_Face(
      self->ft,  /* library */
      fontPath,  /* filepathname */
      0,  /* face_index */
      &face  /* aface */
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
  /* FIXME: Do we need to free this font at some point? */

  /* Set the pixel dimensions of the font */
//  error = FT_Set_Pixel_Sizes(
//      face,  /* face */
//      width,  /* pixel_width */
//      height  /* pixel_height */
//      );
  error = FT_Set_Char_Size(
      face,  /* face */
      0,  /* char_width */
      16*64,  /* char_height */
      300,  /* horz_resolution */
      300  /* vert_resolution */
      );

  if (error != FT_Err_Ok) {
    fprintf(stderr,
        "Freetype failed to set dimensions %dx%d for font: %s\n",
        width, height, fontPath);
  }

  /* NOTE: We need to generate an atlas of the glyphs representing printable
   * ASCII characters in this font. Since it is relatively expensive to blit
   * characters into our atlas, we want to determine the placement of glyphs
   * in the atlas prior to blitting them.
   *
   * The heuristic used to efficiently arrange glyphs in the atlas comes from
   * here: <http://gamedev.stackexchange.com/a/2839>
   */

  /* TODO: Iterate over the printable ASCII characters */
  for (unsigned char c = 32; c <= 127; ++c) {
    /* TODO: Estimate the dimensions of the glyph corresponding to this
     * character in this font */
    st_Fonts_loadGlyph(face, c);
  }
}

void st_Fonts_loadGlyph(FT_Face face, char c) {
  FT_Error error;
  int glyph_index;
  st_Fonts *self = st_Fonts_instance();

  /* TODO: Use the charmap in our face to get the actual glyph for this ASCII
   * character code */
  glyph_index = FT_Get_Char_Index(face, c);
  if (glyph_index == 0) {
    fprintf(stderr,
        "Warning: Freetype could not find a glyph for the character '%c'\n",
        c);
  }
  error = FT_Load_Glyph(
      face,  /* face */
      glyph_index,  /* glyph_index */
      FT_LOAD_DEFAULT  /* load_flags */
      );
  if (error != FT_Err_Ok) {
    fprintf(stderr,
        "Freetype encountered an error loading the glyph for '%c'\n",
        c);
  }
  error = FT_Render_Glyph(
      face->glyph,  /* slot */
      FT_RENDER_MODE_MONO  /* render_mode */
      );
  if (error != FT_Err_Ok) {
    fprintf(stderr,
        "Freetype encountered an error rendering the glyph for '%c'\n",
        c);
  }
  /* Print out the glyph bitmap */
  st_printGlyphDebug(&face->glyph->bitmap);
}

void st_printGlyphDebug(const FT_Bitmap *bitmap) {
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

st_MonospaceGlyph *st_MonospaceFontFace_getGlyph(
    st_MonospaceFontFace *self,
    uint32_t ch)
{
}

/* TODO: Generate signed-distance-field textures */
/* TODO: Arrange glyphs into an atlas */
/* TODO: Define a data structure for keeping glyph information */
/* TODO: Write a shader that can transform glyph geometry and draw the glyphs
 * using instanced rendering */
/* TODO: Support multiple font faces */
/* TODO: Support multiple sizes */
