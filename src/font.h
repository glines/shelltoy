/*
 * Copyright (c) 2017 Jonathan Glines
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

#ifndef SHELLTOY_FONT_H_
#define SHELLTOY_FONT_H_

#include <ft2build.h>
#include FT_FREETYPE_H

#include <inttypes.h>

#include <shelltoy/error.h>

struct st_Font_Internal_;
typedef struct st_Font_Internal_ st_Font_Internal;

typedef struct st_Font_ {
  st_Font_Internal *internal;
} st_Font;

void st_Font_init(
    st_Font *self);

st_ErrorCode
st_Font_load(
    st_Font *self,
    const char *fontPath,
    const char *faceName,
    float fontSize,
    int x_dpi,
    int y_dpi);

int st_Font_isValid(
    const st_Font *self);

void st_Font_destroy(
    st_Font *self);

int
st_Font_hasCharacter(
    st_Font *self,
    uint32_t character);

st_ErrorCode
st_Font_getGlyphDimensions(
    st_Font *self,
    uint32_t character,
    int *width,
    int *height);

st_ErrorCode
st_Font_getGlyphOffset(
    st_Font *self,
    uint32_t character,
    int *x,
    int *y);

st_ErrorCode
st_Font_renderGlyph(
    st_Font *self,
    uint32_t character,
    FT_Bitmap **bitmap);

st_ErrorCode
st_Font_renderGlyph(
    st_Font *self,
    uint32_t character,
    FT_Bitmap **bitmap);

const char *
st_Font_getFaceName(
    const st_Font *self);

const char *
st_Font_getFontPath(
    const st_Font *self);

float
st_Font_getSize(
    const st_Font *self);

st_ErrorCode
st_Font_setSize(
    const st_Font *self,
    float size);

FT_Face
st_Font_getFtFace(
    st_Font *self);

#endif
