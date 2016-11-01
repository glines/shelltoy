#include <assert.h>
#include <stdlib.h>

#include "fonts.h"

/* TODO: Actually get this font path as user input */
const char *FONT_FACE_PATH = "/nix/store/fvwp39z54ka2s7h3gawhfmayrqjnd05a-dejavu-fonts-2.37/share/fonts/truetype/DejaVuSansMono.ttf";
/* TODO: We might even want to read a small font into memory */

/* Freetype library initialization */
FT_Library ft;
void st_initFreetype() {
  FT_Error error = FT_Init_FreeType(&ft);
  if (error != FT_Err_Ok) {
    fprintf(stderr, "Error initializing FreeType 2 library\n");
    /* TODO: Print out an error string for this specific error */
  }
}

FT_Face face;
void st_loadFontFace() {
  FT_Error error;
  /* TODO: Load a font face from file */
  error = FT_New_Face(
      ft,  /* library */
      FONT_FACE_PATH,  /* filepathname */
      0,  /* face_index */
      &face  /* aface */
      );
  if (error == FT_Err_Unknown_File_Format) {
    fprintf(stderr, "Freetype encountered an unknown file format: %s\n",
        FONT_FACE_PATH);
    /* TODO: Print the specific error message from Freetype */
  } else if (error != FT_Err_Ok) {
    fprintf(stderr, "Freetype encountered an error reading file: %s\n",
        FONT_FACE_PATH);
    /* TODO: Print the specific error message from Freetype */
  }
  /* TODO: Do we need to free this font? */

  /* TODO: We need to render this font to a texture for OpenGL to use */
  /* NOTE: I should probably try using FT_Set_Pixel_Sizes() here to get exact
   * pixel sizes in the font, which is more useful for terminals */
  error = FT_Set_Char_Size(
      face,  /* face */
      0,  /* char_width */
      16*64,  /* char_height */
      300,  /* horz_resolution */
      300  /* vert_resolution */
      );
  if (error != FT_Err_Ok) {
    fprintf(stderr, "Freetype encountered an error setting char size");
    assert(0);
  }
}

void st_loadGlyph(char c) {
  FT_Error error;
  int glyph_index;
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

/* TODO: Generate signed-distance-field textures */
/* TODO: Arrange glyphs into an atlas */
/* TODO: Define a data structure for keeping glyph information */
/* TODO: Write a shader that can transform glyph geometry and draw the glyphs
 * using instanced rendering */
/* TODO: Support multiple font faces */
/* TODO: Support multiple sizes */
