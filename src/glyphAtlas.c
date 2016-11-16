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
typedef struct st_GlyphAtlasEntry_ {
  st_BoundingBox bbox;
  uint32_t ch;
} st_GlyphAtlasEntry;

struct st_GlyphAtlas_Internal {
  st_GlyphAtlasEntry *glyphs;
  size_t numGlyphs, sizeGlyphs;
  GLuint textureBuffer;
};

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
  /* Allocate memory for internal data structures */
  self->internal = (struct st_GlyphAtlas_Internal *)malloc(
      sizeof(struct st_GlyphAtlas_Internal));
  self->internal->sizeGlyphs = ST_GLYPH_ATLAS_INIT_SIZE_GLYPHS;
  self->internal->glyphs = (st_GlyphAtlasEntry *)malloc(
      sizeof(st_GlyphAtlasEntry) * self->internal->sizeGlyphs);
  self->internal->numGlyphs = 0;
  /* Initialize our texture buffer */
  glGenTextures(1, &self->internal->textureBuffer);
  FORCE_ASSERT_GL_ERROR();
}

void st_GlyphAtlas_destroy(
    st_GlyphAtlas_ptr self)
{
  /* Free internal data structures */
  free(self->internal->glyphs);
  free(self->internal);
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

int st_compareGlyphCharacters(
    const st_GlyphAtlasEntry *a,
    const st_GlyphAtlasEntry *b)
{
  if (a->ch < b->ch)
    return -1;
  if (a->ch > b->ch)
    return 1;
  return 0;
}

void st_GlyphAtlas_renderASCIIGlyphs(
    st_GlyphAtlas *self,
    st_GlyphRenderer *glyphRenderer)
{
#define PRINT_ASCII_FIRST 33
#define PRINT_ASCII_LAST 126
#define NUM_PRINT_ASCII (PRINT_ASCII_LAST - PRINT_ASCII_FIRST + 1)
  st_GlyphAtlasEntry *pendingGlyphs;
  /* TODO: The collision detection structure should be stored inside of the
   * st_GlyphAtlas internal data structure, so that subsequent collision
   * detection can be performed more cheaply. */
  st_NaiveCollisionDetection collisionDetection;
  st_GlyphAtlasEntry *currentGlyph, *collidingGlyph;
  FT_Bitmap *bitmap;
  size_t numPendingGlyphs, numPlacedGlyphs;
  int done;
  uint8_t *atlasTexture;
  size_t textureSize;
  const int padding = 2;
  int error;

  pendingGlyphs = (st_GlyphAtlasEntry*)malloc(
      sizeof(st_GlyphAtlasEntry) * NUM_PRINT_ASCII);

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
    /* FIXME: I don't really like that the
     * st_GlyphRenderer_getGlyphDimensions() method does two things, but it
     * does avoid a few FreeType calls */
    error = st_GlyphRenderer_getGlyphDimensions(glyphRenderer, c,
        &currentGlyph->bbox.w, &currentGlyph->bbox.h);
    if (error)
      continue;  /* This glyph is not being provided */
    /* Add this glyph to our list of glyphs */
    numPendingGlyphs += 1;
    currentGlyph->ch = c;
    /* Add padding to the glyph size */
    currentGlyph->bbox.w += padding * 2;
    currentGlyph->bbox.h += padding * 2;
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
            /*
            fprintf(stderr, "%d, %d, %d, %d\n",
                currentGlyph->bbox.x,
                currentGlyph->bbox.y,
                currentGlyph->bbox.x + currentGlyph->bbox.w,
                currentGlyph->bbox.y + currentGlyph->bbox.h);
            */
            st_NaiveCollisionDetection_addEntity(
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
    st_NaiveCollisionDetection_destroy(&collisionDetection);
  }
  assert(done);
  /* Allocate memory for our atlas texture */
  atlasTexture = (uint8_t*)malloc(textureSize * textureSize);
  memset(atlasTexture, 0 /* XXX */, textureSize * textureSize);
  for (int i = 0; i < numPendingGlyphs; ++i) {
    currentGlyph = &pendingGlyphs[i];
    /* Render each glyph */
    bitmap = st_GlyphRenderer_renderGlyph(
        glyphRenderer,
        currentGlyph->ch);
    assert(bitmap->width < currentGlyph->bbox.w);
    assert(bitmap->rows < currentGlyph->bbox.h);
    /* Blit the rendered glyph onto our texture in memory */
    st_GlyphAtlas_blitGlyph(
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
    st_GlyphAtlasEntry *newGlyphs;
    /* Grow the array of glyphs to hold the new glyphs */
    /* FIXME: This could probably be moved to an st_GlyphAtlas_growGlyphs()
     * private method */
    do {
      self->internal->sizeGlyphs *= 2;
    } while (
        self->internal->numGlyphs + numPendingGlyphs > self->internal->sizeGlyphs);
    newGlyphs = (st_GlyphAtlasEntry *)malloc(
        sizeof(st_GlyphAtlasEntry) * self->internal->sizeGlyphs);
    memcpy(newGlyphs, self->internal->glyphs,
        sizeof(st_GlyphAtlasEntry) * self->internal->numGlyphs);
    free(self->internal->glyphs);
    self->internal->glyphs = newGlyphs;
  }

  /* Copy the pending glyphs to the array of glyphs stored in our internal data
   * structure */
  memcpy(
      &self->internal->glyphs[self->internal->numGlyphs],
      pendingGlyphs,
      sizeof(st_GlyphAtlasEntry) * numPendingGlyphs);
  self->internal->numGlyphs += numPendingGlyphs;
  /* Sort the glyphs by character so that they can be searched */
  qsort(
    self->internal->glyphs,  /* ptr */
    self->internal->numGlyphs,  /* count */
    sizeof(st_GlyphAtlasEntry),  /* size */
    (int(*)(const void *, const void *))st_compareGlyphCharacters  /* comp */
    );

  free(atlasTexture);
  free(pendingGlyphs);
}

int st_GlyphAtlas_getGlyph(
    const st_GlyphAtlas *self,
    uint32_t character,
    st_BoundingBox *bbox,
    int *atlasTextureIndex)
{
  int a, b, i;
  st_GlyphAtlasEntry *currentGlyph;
  assert(self->internal->numGlyphs > 0);  /* XXX */
  if (self->internal->numGlyphs == 0)
    return 1;
  /* Binary search for the glyph corresponding to the given character */
  /* FIXME: Check the loop conditions and write some unit tests for this thing;
   * this binary search is probably broken */
  a = 0; b = self->internal->numGlyphs;
  while (a < b) {
    i = (b - a) / 2 + a;
    currentGlyph = &self->internal->glyphs[i];
    if (character < currentGlyph->ch) {
      b = i;
    } else if (character > currentGlyph->ch) {
      a = i + 1;
    } else {
      /* character == currentGlyph->ch */
      break;
    }
  }
  if (currentGlyph->ch != character)
    return 1;
  memcpy(bbox, &currentGlyph->bbox, sizeof(*bbox));
  /* FIXME: Set the atlas texture index once we start using more than one atlas
   * texture */
  *atlasTextureIndex = 0;
  return 0;
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
//  st_printAntiAliasedGlyphDebug(bitmap);  /* XXX */
//  fprintf(stderr, "\n");  /* XXX */
}

void st_GlyphAtlas_getTextures(
    const st_GlyphAtlas *self,
    GLuint *textures,
    int *numTextures)
{
  /* TODO; Support returning multiple textures */
  textures[0] = self->internal->textureBuffer;
  *numTextures = 1;
}

