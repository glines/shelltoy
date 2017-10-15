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

#ifndef TTOY_FONT_H_
#define TTOY_FONT_H_

#include <ft2build.h>
#include FT_FREETYPE_H

#include <inttypes.h>

#include <ttoy/error.h>

struct ttoy_Font_Internal_;
typedef struct ttoy_Font_Internal_ ttoy_Font_Internal;

typedef struct ttoy_Font_ {
  ttoy_Font_Internal *internal;
} ttoy_Font;

void ttoy_Font_init(
    ttoy_Font *self);

ttoy_ErrorCode
ttoy_Font_load(
    ttoy_Font *self,
    const char *fontPath,
    const char *faceName,
    float fontSize,
    int x_dpi,
    int y_dpi);

int ttoy_Font_isValid(
    const ttoy_Font *self);

void ttoy_Font_destroy(
    ttoy_Font *self);

int
ttoy_Font_hasCharacter(
    ttoy_Font *self,
    uint32_t character);

ttoy_ErrorCode
ttoy_Font_getGlyphDimensions(
    ttoy_Font *self,
    uint32_t character,
    int *width,
    int *height);

ttoy_ErrorCode
ttoy_Font_getGlyphOffset(
    ttoy_Font *self,
    uint32_t character,
    int *x,
    int *y);

ttoy_ErrorCode
ttoy_Font_renderGlyph(
    ttoy_Font *self,
    uint32_t character,
    FT_Bitmap **bitmap);

ttoy_ErrorCode
ttoy_Font_renderGlyph(
    ttoy_Font *self,
    uint32_t character,
    FT_Bitmap **bitmap);

const char *
ttoy_Font_getFaceName(
    const ttoy_Font *self);

const char *
ttoy_Font_getFontPath(
    const ttoy_Font *self);

float
ttoy_Font_getSize(
    const ttoy_Font *self);

ttoy_ErrorCode
ttoy_Font_setSize(
    const ttoy_Font *self,
    float size);

FT_Face
ttoy_Font_getFtFace(
    ttoy_Font *self);

#endif
