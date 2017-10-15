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
#include "common/glError.h"
#include "fonts.h"
#include "glyphAtlas.h"
#include "naiveCollisionDetection.h"

#define max(a, b) (a) < (b) ? (b) : (a);

/* Private data structures */
typedef struct ttoy_GlyphAtlasEntry_ {
  ttoy_BoundingBox bbox;
  int xOffset, yOffset;
  uint32_t ch;
  int cellWidth, cellHeight;
  int fontIndex;
} ttoy_GlyphAtlasEntry;

struct ttoy_GlyphAtlas_Internal {
  ttoy_GlyphAtlasEntry *glyphs;
  size_t numGlyphs, sizeGlyphs;
  GLuint textureBuffer;
  int textureSize;
};

/* Private method declarations */
void ttoy_GlyphAtlas_blitGlyph(
    const ttoy_GlyphAtlasEntry *glyph,
    const FT_Bitmap *bitmap,
    int padding,
    uint8_t *atlasTexture,
    int textureSize);

void ttoy_GlyphAtlas_init(
    ttoy_GlyphAtlas_ptr self)
{
  /* Allocate memory for internal data structures */
  self->internal = (struct ttoy_GlyphAtlas_Internal *)malloc(
      sizeof(struct ttoy_GlyphAtlas_Internal));
  self->internal->sizeGlyphs = TTOY_GLYPH_ATLAS_INIT_SIZE_GLYPHS;
  self->internal->glyphs = (ttoy_GlyphAtlasEntry *)malloc(
      sizeof(ttoy_GlyphAtlasEntry) * self->internal->sizeGlyphs);
  self->internal->numGlyphs = 0;
  /* Initialize our texture buffer */
  glGenTextures(1, &self->internal->textureBuffer);
  FORCE_ASSERT_GL_ERROR();
}

void ttoy_GlyphAtlas_destroy(
    ttoy_GlyphAtlas_ptr self)
{
  /* Free internal data structures */
  free(self->internal->glyphs);
  free(self->internal);
}

int ttoy_compareGlyphSizes(
    const ttoy_GlyphAtlasEntry *a,
    const ttoy_GlyphAtlasEntry *b)
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

int ttoy_compareGlyphs(
    const ttoy_GlyphAtlasEntry *a,
    const ttoy_GlyphAtlasEntry *b)
{
  if (a->ch < b->ch)
    return -1;
  if (a->ch > b->ch)
    return 1;
  if (a->fontIndex < b->fontIndex)
    return -1;
  if (a->fontIndex > b->fontIndex)
    return 1;
  return 0;
}

void ttoy_GlyphAtlas_renderASCIIGlyphs(
    ttoy_GlyphAtlas *self,
    ttoy_GlyphRenderer *glyphRenderer)
{
#define PRINT_ASCII_FIRST 33
#define PRINT_ASCII_LAST 126
#define NUM_PRINT_ASCII (PRINT_ASCII_LAST - PRINT_ASCII_FIRST + 1)
  ttoy_GlyphAtlasEntry *pendingGlyphs;
  /* TODO: The collision detection structure should be stored inside of the
   * ttoy_GlyphAtlas internal data structure, so that subsequent collision
   * detection can be performed more cheaply. */
  ttoy_NaiveCollisionDetection collisionDetection;
  ttoy_GlyphAtlasEntry *currentGlyph, *collidingGlyph;
  FT_Bitmap *bitmap;
  size_t numPendingGlyphs;
  int done;
  uint8_t *atlasTexture;
  int textureSize;
  const int padding = 2;
  int error;
  int cellWidth, cellHeight;

  pendingGlyphs = (ttoy_GlyphAtlasEntry*)malloc(
      sizeof(ttoy_GlyphAtlasEntry) * NUM_PRINT_ASCII);

  /* We will need to store the cell size with each glyph rendered, for future
   * reference if the cell size ever changes */
  ttoy_GlyphRenderer_getCellSize(glyphRenderer,
      &cellWidth,  /* width */
      &cellHeight  /* height */
      );

  /* Iterate over the printable ASCII characters and gather glyphs (without
   * rendering them) from this face */
  numPendingGlyphs = 0;
  for (uint32_t c = PRINT_ASCII_FIRST;
      c <= PRINT_ASCII_LAST;
      ++c)
  {
    assert(numPendingGlyphs < NUM_PRINT_ASCII);
    currentGlyph = &pendingGlyphs[numPendingGlyphs];
    /* Check for the glyph and get its dimensions */
    error = ttoy_GlyphRenderer_getGlyphDimensions(glyphRenderer,
        c,  /* character */
        0,  /* bold */
        &currentGlyph->bbox.w, &currentGlyph->bbox.h);
    if (error)
      continue;  /* This glyph is not being provided */
    /* Add this glyph to our list of glyphs */
    numPendingGlyphs += 1;
    currentGlyph->ch = c;
    /* Store the glyph offset */
    ttoy_GlyphRenderer_getGlyphOffset(glyphRenderer,
        c,  /* character */
        0,  /* bold */
        &currentGlyph->xOffset,  /* x */
        &currentGlyph->yOffset  /* y */
        );
    /* Add padding to the glyph size */
    currentGlyph->bbox.w += 2 * padding;
    currentGlyph->bbox.h += 2 * padding;
    /* Compensate for the padding in the glyph offset */
    currentGlyph->xOffset -= padding;
    currentGlyph->yOffset -= padding;
    /* Store the cell size for which this glyph was rendered */
    /* FIXME: The same font can be rendered for different cell dimensions, e.g.
     * if a secondary font for asian characters is chosen for different cell
     * dimensions. I think it would be best to give up on glyph "garbage
     * collection" and simply toss the glyph atlas whenever the font changes.
     * We can still get responsive font size changes while the new atlas is
     * being computed. */
    currentGlyph->cellWidth = cellWidth;
    currentGlyph->cellHeight = cellHeight;
    /*
    fprintf(stderr,
        "currentGlyph: '%c'\n"
        "    currentGlyph->bbox.w: %d\n"
        "    currentGlyph->bbox.h: %d\n",
        (char)currentGlyph->ch,
        currentGlyph->bbox.w,
        currentGlyph->bbox.h);
        */
  }
  /* FIXME: I think there's a bug here in case we ever load a font with no
   * glyphs. The atlas seems to grow out of control. */
  /* Sort our list of glyphs by glyph area in pixels */
  qsort(
    pendingGlyphs,  /* ptr */
    numPendingGlyphs,  /* count */
    sizeof(ttoy_GlyphAtlasEntry),  /* size */
    (int(*)(const void *, const void *))ttoy_compareGlyphSizes  /* comp */
    );
  /* We place glyphs in the atlas texture using the heuristic described here:
   * <http://gamedev.stackexchange.com/a/2839> */
  /* Loop over the possible texture sizes, such that if a small texture will
   * not hold all of the glyphs we can try again with a larger texture */
  done = 0;
  for (textureSize = TTOY_GLYPH_ATLAS_MIN_TEXTURE_SIZE; 
      textureSize <= TTOY_GLYPH_ATLAS_MAX_TEXTURE_SIZE && !done;
      textureSize *= 2)
  {
    fprintf(stderr, "Growing atlas texture to %dx%d\n",
        textureSize, textureSize);
    ttoy_NaiveCollisionDetection_init(&collisionDetection);
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
          collidingGlyph = (ttoy_GlyphAtlasEntry*)
            ttoy_NaiveCollisionDetection_checkCollision(
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
            /*
            fprintf(stderr, "%d, %d, %d, %d\n",
                currentGlyph->bbox.x,
                currentGlyph->bbox.y,
                currentGlyph->bbox.x + currentGlyph->bbox.w,
                currentGlyph->bbox.y + currentGlyph->bbox.h);
            */
            ttoy_NaiveCollisionDetection_addEntity(
                &collisionDetection,
                &currentGlyph->bbox,
                (void*)currentGlyph);
            done = 1;
          } else {
            /* TODO: Skip the columns that collidingGlyph occupies to improve
             * performance here */
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
    ttoy_NaiveCollisionDetection_destroy(&collisionDetection);
  }
  assert(done);
  fprintf(stderr, "Ultimate atlas texture size: %dx%d\n",
      textureSize, textureSize);
  self->internal->textureSize = textureSize;
  /* Allocate memory for our atlas texture */
  atlasTexture = (uint8_t*)malloc(textureSize * textureSize);
  memset(atlasTexture, 0 /* XXX */, textureSize * textureSize);
  for (int i = 0; i < numPendingGlyphs; ++i) {
    currentGlyph = &pendingGlyphs[i];
    /* Render each glyph */
    error = ttoy_GlyphRenderer_renderGlyph(glyphRenderer,
        currentGlyph->ch,  /* character */
        0,  /* bold */
        &bitmap,  /* bitmap */
        &currentGlyph->fontIndex  /* fontIndex */
        );
    assert(bitmap->width < currentGlyph->bbox.w);
    assert(bitmap->rows < currentGlyph->bbox.h);
    /* Blit the rendered glyph onto our texture in memory */
    ttoy_GlyphAtlas_blitGlyph(
        currentGlyph,  /* glyph */
        bitmap,  /* bitmap */
        padding,  /* padding */
        atlasTexture,  /* atlasTexture */
        textureSize  /* textureSize */
        );
  }
  /* XXX */
  /*
  for (int i = textureSize; i >= 0; --i) {
    for (int j = 0; j < textureSize / 8; ++j) {
      fprintf(stderr, "%c",
          atlasTexture[i * textureSize + j] == 0 ? '#' : ' ');
    }
    fprintf(stderr, "\n");
  }
  */
  /* TODO: Output the atlas texture to a PNG file for debugging */
  /* Send our atlas texture to the GL */
  glBindTexture(GL_TEXTURE_2D, self->internal->textureBuffer);
  FORCE_ASSERT_GL_ERROR();
  glTexImage2D(
      GL_TEXTURE_2D,  /* target */
      0,  /* level */
      GL_RED,  /* internalFormat */
      textureSize,  /* width */
      textureSize,  /* height */
      0,  /* border */
      GL_RED,  /* format */
      GL_UNSIGNED_BYTE,  /* type */
      atlasTexture  /* data */
      );
  FORCE_ASSERT_GL_ERROR();
  glTexParameteri(
      GL_TEXTURE_2D,  /* target */
      GL_TEXTURE_MIN_FILTER,  /* pname */
      GL_NEAREST  /* param */
      );
  FORCE_ASSERT_GL_ERROR();
  glTexParameteri(
      GL_TEXTURE_2D,  /* target */
      GL_TEXTURE_MAG_FILTER,  /* pname */
      GL_NEAREST  /* param */
      );
  FORCE_ASSERT_GL_ERROR();

  if (self->internal->numGlyphs + numPendingGlyphs > self->internal->sizeGlyphs) {
    ttoy_GlyphAtlasEntry *newGlyphs;
    /* Grow the array of glyphs to hold the new glyphs */
    /* FIXME: This could probably be moved to an ttoy_GlyphAtlas_growGlyphs()
     * private method */
    do {
      self->internal->sizeGlyphs *= 2;
    } while (
        self->internal->numGlyphs + numPendingGlyphs > self->internal->sizeGlyphs);
    newGlyphs = (ttoy_GlyphAtlasEntry *)malloc(
        sizeof(ttoy_GlyphAtlasEntry) * self->internal->sizeGlyphs);
    memcpy(newGlyphs, self->internal->glyphs,
        sizeof(ttoy_GlyphAtlasEntry) * self->internal->numGlyphs);
    free(self->internal->glyphs);
    self->internal->glyphs = newGlyphs;
  }

  /* Copy the pending glyphs to the array of glyphs stored in our internal data
   * structure */
  memcpy(
      &self->internal->glyphs[self->internal->numGlyphs],
      pendingGlyphs,
      sizeof(ttoy_GlyphAtlasEntry) * numPendingGlyphs);
  self->internal->numGlyphs += numPendingGlyphs;
  /* Sort the glyphs by character so that they can be searched */
  qsort(
    self->internal->glyphs,  /* ptr */
    self->internal->numGlyphs,  /* count */
    sizeof(ttoy_GlyphAtlasEntry),  /* size */
    (int(*)(const void *, const void *))ttoy_compareGlyphs  /* comp */
    );

  free(atlasTexture);
  free(pendingGlyphs);
}

ttoy_ErrorCode
ttoy_GlyphAtlas_getGlyph(
    const ttoy_GlyphAtlas *self,
    uint32_t character,
    int fontIndex,
    int cellWidth,
    int cellHeight,
    ttoy_BoundingBox *bbox,
    float *xOffset,
    float *yOffset,
    float *glyphWidth,
    float *glyphHeight)
{
  int a, b, i;
  float widthRatio, heightRatio;
  ttoy_GlyphAtlasEntry *currentGlyph;
  ttoy_GlyphAtlasEntry target;
  int result;
  if (self->internal->numGlyphs == 0)
    return TTOY_ERROR_ATLAS_GLYPH_NOT_FOUND;
  target.ch = character;
  target.fontIndex = fontIndex;
  currentGlyph = &self->internal->glyphs[0];
  /* Binary search for the glyph corresponding to the given character */
  /* FIXME: Check the loop conditions and write some unit tests for this thing;
   * this binary search is probably broken */
  a = 0; b = self->internal->numGlyphs;
  while (a < b) {
    i = (b - a) / 2 + a;
    currentGlyph = &self->internal->glyphs[i];
    result = ttoy_compareGlyphs(&target, currentGlyph);
    if (result < 0) {
      b = i;
    } else if (result > 0) {
      a = i + 1;
    } else {
      /* target == currentGlyph */
      break;
    }
  }
  result = ttoy_compareGlyphs(&target, currentGlyph);
  if (result != 0)
    return TTOY_ERROR_ATLAS_GLYPH_NOT_FOUND;
  memcpy(bbox, &currentGlyph->bbox, sizeof(*bbox));
  /* Calculate the offset and dimensions of the glyph using the ratio of the
   * current cell dimensions over the cell dimensions for which the glyph was
   * rendered at */
  widthRatio = (float)cellWidth / (float)currentGlyph->cellWidth;
  heightRatio = (float)cellHeight / (float)currentGlyph->cellHeight;
  *xOffset = currentGlyph->xOffset * widthRatio;
  *yOffset = currentGlyph->yOffset * heightRatio;
  *glyphWidth = currentGlyph->bbox.dim[0] * widthRatio;
  *glyphHeight = currentGlyph->bbox.dim[1] * heightRatio;
  /* TODO: Set the atlas texture index once we start using more than one atlas
   * texture */
  return TTOY_NO_ERROR;
}

void ttoy_GlyphAtlas_blitGlyph(
    const ttoy_GlyphAtlasEntry *glyph,
    const FT_Bitmap *bitmap,
    int padding,
    uint8_t *atlasTexture,
    int textureSize)
{
  int destIndex;
  /* Iterate over the rows in the bitmap and copy each row to the
   * atlas texture */
  assert(bitmap->pitch >= 0);  /* TODO: Handle negative pitch (i.e. flipped y
                                  coordinates). This will probably work, I just
                                  haven't tested it yet, so this assert is here
                                  for when we encounter a negative pitch value
                                  in the future. */
  for (int row = 0; row < bitmap->rows; ++row) {
    if (bitmap->pitch < 0) {
      destIndex = (glyph->bbox.y + row + padding) * textureSize;
    } else {
      assert(bitmap->pitch != 0);
      /* bitmap->pitch > 0 */
      destIndex = (glyph->bbox.y + bitmap->rows - row + padding) * textureSize;
    }
    destIndex += glyph->bbox.x + padding;
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
//  ttoy_printAntiAliasedGlyphDebug(bitmap);  /* XXX */
//  fprintf(stderr, "\n");  /* XXX */
}

void ttoy_GlyphAtlas_getTextures(
    const ttoy_GlyphAtlas *self,
    GLuint *textures,
    int *numTextures)
{
  /* TODO; Support returning multiple textures */
  textures[0] = self->internal->textureBuffer;
  *numTextures = 1;
}

int ttoy_GlyphAtlas_getTextureSize(
    const ttoy_GlyphAtlas *self)
{
  return self->internal->textureSize;
}
