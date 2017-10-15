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

#ifndef TTOY_GLYPH_ATLAS_H_
#define TTOY_GLYPH_ATLAS_H_

#include <ft2build.h>
#include FT_FREETYPE_H

#include <GL/glew.h>
#include <inttypes.h>
#include <stddef.h>

#include "boundingBox.h"
#include "glyphRenderer.h"

#define TTOY_GLYPH_ATLAS_MIN_TEXTURE_SIZE 256
#define TTOY_GLYPH_ATLAS_MAX_TEXTURE_SIZE 4096
#define TTOY_GLYPH_ATLAS_INIT_SIZE_GLYPHS 256
#define TTOY_GLYPH_ATLAS_MAX_NUM_TEXTURES 4

struct ttoy_GlyphAtlas_Internal;

/**
 * Class for managing glyphs that have been rendered to a GL texture (or
 * textures) called an atlas.
 *
 * Once a glyph atlas object has been initialized, glyphs can be rendered and
 * added to its GL textures through one of several glyph atlas methods.
 *
 * Glyphs can be added at any time (e.g. at initialization, or when a glyph is
 * encountered for the first time), and the textures associated with this glyph
 * atlas are added and grown accordingly.
 *
 * \todo Change this to an abstract class and move the current functionality to
 * an ttoy_AAGlyphAtlas class and signed distance field functionality to an
 * ttoy_SDFGlyphAtlas class. This refactoring will likely require the
 * implementation of a simple vtable.
 *
 * \todo Determine whether or not on-the-fly glyph loading causes too much
 * latency; if so, we may need to implement asynchronous glyph loading.
 */
typedef struct ttoy_GlyphAtlas_ {
  struct ttoy_GlyphAtlas_Internal *internal;
} ttoy_GlyphAtlas;
typedef struct ttoy_GlyphAtlas_ * ttoy_GlyphAtlas_ptr;

/* Public methods */
/**
 * Initializes the internal structures of the given glyph atlas.
 *
 * Since the glyph atlas may need to allocate buffers in the GL, this method
 * must be called after the GL has been initialized.
 */
void ttoy_GlyphAtlas_init(
    ttoy_GlyphAtlas_ptr self);

void ttoy_GlyphAtlas_destroy(
    ttoy_GlyphAtlas_ptr self);

/**
 * This method renders and adds all of the ASCII glyphs provided by the given
 * FreeType face object.
 *
 * Adding all of the ASCII glyphs from a face is most useful when initializing
 * a glyph atlas with a new font. By adding all ASCII glyphs at once we can
 * pack glyphs into the atlas more intelligently, and we avoid the latency of
 * loading glyphs on-demand.
 *
 * Since adding glyphs might cause the atlas to allocate new texture buffers in
 * the GL, this method must be called after the GL has been initialized.
 */
void ttoy_GlyphAtlas_renderASCIIGlyphs(
    ttoy_GlyphAtlas *self,
    ttoy_GlyphRenderer *glyphRenderer);

void ttoy_GlyphAtlas_addGlyph(
    /* TODO */);

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
    float *glyphHeight);

void ttoy_GlyphAtlas_getTextures(
    const ttoy_GlyphAtlas *self,
    GLuint *textures,
    int *numTextures);

int ttoy_GlyphAtlas_getTextureSize(
    const ttoy_GlyphAtlas *self);

#endif
