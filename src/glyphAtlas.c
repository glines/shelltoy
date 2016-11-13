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

#include "collisionDetection.h"
#include "fonts.h"
#include "glyphAtlas.h"
#include "naiveCollisionDetection.h"

#define max(a, b) (a) < (b) ? (b) : (a);

/* Private method declarations */
void st_GlyphAtlas_blitGlyph(
    const st_GlyphAtlasEntry *glyph,
    const FT_Bitmap *bitmap,
    int padding,
    uint8_t *atlasTexture,
    int textureSize);

void st_GlyphAtlas_init(
    st_GlyphAtlas_ptr self)
{
  /* TODO: */
}

int st_compareGlyphSizes(
    const st_GlyphAtlasEntry *a,
    const st_GlyphAtlasEntry *b)
{
  int size_a, size_b;
  size_a = a->bbox.w * a->bbox.h;
  size_b = b->bbox.w * b->bbox.h;
  if (size_a < size_b)
    return -1;
  if (size_a > size_b)
    return 1;
  return 0;
}

typedef struct st_SortAndSweepGlyph_ {
  st_GlyphAtlasEntry *glyph;
  int pos, isEnd;
} st_SortAndSweepGlyph;

int st_compareSortAndSweepGlyph(
    const st_SortAndSweepGlyph *a,
    const st_SortAndSweepGlyph *b)
{
  if (a->pos < b->pos)
    return -1;
  if (a->pos > b->pos)
    return 1;
  return 0;
}

void st_GlyphAtlas_addASCIIGlyphsFromFace(
    FT_Face face)
{
#define PRINT_ASCII_FIRST 33
#define PRINT_ASCII_LAST 126
#define NUM_PRINT_ASCII (PRINT_ASCII_LAST - PRINT_ASCII_FIRST + 1)
  int glyph_index;
  st_GlyphAtlasEntry *pendingGlyphs;
  st_NaiveCollisionDetection collisionDetection;
  st_GlyphAtlasEntry *currentGlyph, *collidingGlyph;
  size_t numPendingGlyphs, numPlacedGlyphs;
  int maxGlyphWidth, maxGlyphHeight;
  int done;
  uint8_t *atlasTexture;
  size_t textureSize;
  FT_Error error;
  const int padding = 2;

  pendingGlyphs = (st_GlyphAtlasEntry*)malloc(
      sizeof(st_GlyphAtlasEntry) * NUM_PRINT_ASCII);

  /* Iterate over the printable ASCII characters and gather glyphs (without
   * rendering them) from this face */
  maxGlyphWidth = 0;
  maxGlyphHeight = 0;
  numPendingGlyphs = 0;
  for (uint32_t c = PRINT_ASCII_FIRST;
      c <= PRINT_ASCII_LAST;
      ++c)
  {
    /* Check for a corresponding glyph provided by this face */
    glyph_index = FT_Get_Char_Index(face, c);
    if (!glyph_index)
      continue;
    /* Load the glyph */
    error = FT_Load_Glyph(
        face,  /* face */
        glyph_index,  /* glyph_index */
        FT_LOAD_DEFAULT  /* load_flags */
        );
    if (error != FT_Err_Ok) {
      fprintf(stderr,
          "Freetype error loading the glyph for ASCII character '%c'\n",
          (char)c);
    }
    /* Add this glyph to our list of glyphs */
    assert(numPendingGlyphs < NUM_PRINT_ASCII);
    currentGlyph = &pendingGlyphs[numPendingGlyphs++];
    currentGlyph->ch = c;
    fprintf(stderr,
        "face->glyph->metrics.width: %ld\n"
        "face->glyph->metrics.height: %ld\n",
        face->glyph->metrics.width,
        face->glyph->metrics.height);
    /* Calculate the pixel dimensions of the (soon to be rendered) glyph */
    /* We convert the fixed-point 26.6 format (a.k.a. 1/64th pixel format) to
     * an integer value in pixels, rounding up */
#define TWENTY_SIX_SIX_TO_PIXELS(value) ( \
    ((value) >> 6) + ((value) & ((1 << 6) - 1) ? 1 : 0))
    currentGlyph->bbox.w =
      TWENTY_SIX_SIX_TO_PIXELS(face->glyph->metrics.width) + padding * 2;
    currentGlyph->bbox.h =
      TWENTY_SIX_SIX_TO_PIXELS(face->glyph->metrics.height) + padding * 2;
    fprintf(stderr,
        "currentGlyph->bbox.w: %d\n"
        "currentGlyph->bbox.h: %d\n",
        currentGlyph->bbox.w,
        currentGlyph->bbox.h);
    /* Calculate maximum glyph width and height to aid in broad-phase collision
     * detection later */
    maxGlyphWidth = max(maxGlyphWidth, currentGlyph->bbox.w);
    maxGlyphHeight = max(maxGlyphHeight, currentGlyph->bbox.h);
  }
  /* Sort our list of glyphs by glyph area in pixels */
  qsort(
    pendingGlyphs,  /* ptr */
    numPendingGlyphs,  /* count */
    sizeof(st_GlyphAtlasEntry),  /* size */
    (int(*)(const void *, const void *))st_compareGlyphSizes  /* comp */
    );
  /* We place glyphs in the atlas texture using the heuristic described here:
   * <http://gamedev.stackexchange.com/a/2839> */
  /* Loop over the possible texture sizes, such that if a small texture will
   * not hold all of the glyphs we can try again with a larger texture */
  done = 0;
  for (textureSize = ST_GLYPH_ATLAS_MIN_TEXTURE_SIZE; 
      textureSize <= ST_GLYPH_ATLAS_MAX_TEXTURE_SIZE && !done;
      textureSize *= 2)
  {
    fprintf(stderr, "Growing atlas texture to %ldx%ld\n",
        textureSize, textureSize);
    st_NaiveCollisionDetection_init(&collisionDetection);
    /* Position glyphs in the texture, starting with the largest glyphs */
    for (int i = numPendingGlyphs - 1; i >= 0; --i) {
      currentGlyph = &pendingGlyphs[i];
      done = 0;
      /* Start from the first scanline, which is located at the bottom of the
       * texture */
      for (size_t scanline = 0; (scanline < textureSize) && !done; ++scanline) {
        currentGlyph->bbox.y = scanline;
        if (currentGlyph->bbox.y + currentGlyph->bbox.h >= textureSize) {
          /* The glyph reaches beyond the atlas; break out of this loop to grow
           * the atlas texture */
          break;
        }
        /* Try positioning the glyph at each column. Columns are skipped as we
         * encounter glyphs occupying that space. */
        for (size_t column = 0; (column < textureSize) && !done; ++column) {
          currentGlyph->bbox.x = column;
          if (currentGlyph->bbox.x + currentGlyph->bbox.w >= textureSize) {
            /* The glyph reaches beyond the atlas; break out of this loop to go
             * to the next scanline */
            break;
          }
          /* TODO: Replace all of these NaiveCollisionDetection calls with
           * virtual calls */
          collidingGlyph = (st_GlyphAtlasEntry*)
            st_NaiveCollisionDetection_checkCollision(
                &collisionDetection,
                &currentGlyph->bbox);
          if (collidingGlyph == NULL) {
            /* We found a suitable position for this glyph */
            /*
            fprintf(stderr, "Placed glyph '%c' at: (%d, %d)\n",
                (char)currentGlyph->ch,
                currentGlyph->bbox.x,
                currentGlyph->bbox.y);
                */
            fprintf(stderr, "%d, %d, %d, %d\n",
                currentGlyph->bbox.x,
                currentGlyph->bbox.y,
                currentGlyph->bbox.x + currentGlyph->bbox.w,
                currentGlyph->bbox.y + currentGlyph->bbox.h);
            st_NaiveCollisionDetection_addEntity(
                &collisionDetection,
                &currentGlyph->bbox,
                (void*)currentGlyph);
            done = 1;
          }
        }
      }
      if (!done) {
        /* We were unable to place this glyph in the atlas; break out of this
         * loop to grow the atlas texture */
        fprintf(stderr, "Could not place glyph '%c'\n",
            (char)currentGlyph->ch);
        break;
      }
    }
    /* FIXME: Without providing a "clear" method for NaiveCollisionDetection,
     * this is not very efficient */
    st_NaiveCollisionDetection_destroy(&collisionDetection);
  }
  assert(done);
  /* Allocate memory for our atlas texture */
  atlasTexture = (uint8_t*)malloc(textureSize * textureSize);
  memset(atlasTexture, 0, textureSize * textureSize);
  for (int i = 0; i < numPendingGlyphs; ++i) {
    currentGlyph = &pendingGlyphs[i];
    /* Render each glyph */
    glyph_index = FT_Get_Char_Index(face, currentGlyph->ch);
    assert(glyph_index != 0);
    error = FT_Load_Glyph(
        face,  /* face */
        glyph_index,  /* glyph_index */
        FT_LOAD_DEFAULT  /* load_flags */
        );
    assert(error == FT_Err_Ok);
    error = FT_Render_Glyph(
        face->glyph,
        FT_RENDER_MODE_NORMAL);
    if (error != FT_Err_Ok) {
      fprintf(stderr,
          "Freetype encountered an error rendering the glyph for '%c'\n",
          (char)currentGlyph->ch);
    }
    assert(face->glyph->bitmap.width < currentGlyph->bbox.w);
    assert(face->glyph->bitmap.rows < currentGlyph->bbox.h);
    /* Blit the rendered glyph onto our texture in memory */
    st_GlyphAtlas_blitGlyph(
        currentGlyph,  /* glyph */
        &face->glyph->bitmap,  /* bitmap */
        padding,  /* padding */
        atlasTexture,  /* atlasTexture */
        textureSize  /* textureSize */
        );
  }
  for (int i = textureSize; i >= 0; --i) {
    for (int j = 0; j < textureSize / 8; ++j) {
      fprintf(stderr, "%c",
          atlasTexture[i * textureSize + j] == 0 ? '#' : ' ');
    }
    fprintf(stderr, "\n");
  }
  /* TODO: Output the atlas texture to a PNG file for debugging */
  /* TODO: Send our atlas texture to the GL */
  free(atlasTexture);
  free(pendingGlyphs);
}

void st_GlyphAtlas_blitGlyph(
    const st_GlyphAtlasEntry *glyph,
    const FT_Bitmap *bitmap,
    int padding,
    uint8_t *atlasTexture,
    int textureSize)
{
  int destIndex;
  /* Iterate over the rows in the bitmap and copy each row to the
   * atlas texture */
  for (int row = 0; row < bitmap->rows; ++row) {
    destIndex = (glyph->bbox.y + row + padding) * textureSize + glyph->bbox.x + padding;
#ifndef NDEBUG
    for (int i = 0; i < bitmap->width; ++i) {
      /* Make sure we don't blit over any existing glyphs */
      assert(atlasTexture[destIndex + i] == 0);
    }
#endif
    memcpy(
        &atlasTexture[destIndex],
        &bitmap->buffer[row * abs(bitmap->pitch)],
        bitmap->width);
  }
  st_printAntiAliasedGlyphDebug(bitmap);
  fprintf(stderr, "\n");
}
