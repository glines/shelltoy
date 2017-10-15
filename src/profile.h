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

#ifndef TTOY_PROFILE_H_
#define TTOY_PROFILE_H_

#include <inttypes.h>

#include <ttoy/backgroundToy.h>
#include <ttoy/error.h>
#include <ttoy/textToy.h>

#include "color.h"
#include "fontRefArray.h"

struct ttoy_Profile_Internal_;
typedef struct ttoy_Profile_Internal_ ttoy_Profile_Internal;

typedef enum ttoy_Profile_Flag_ {
  TTOY_PROFILE_ANTIALIAS_FONT = 1 << 0,
  TTOY_PROFILE_BRIGHT_IS_BOLD = 1 << 1,
} ttoy_Profile_Flag;

typedef struct ttoy_Profile_ {
  char *name;
  float fontSize;
  uint32_t flags;
  ttoy_ColorScheme colorScheme;
  ttoy_Profile_Internal *internal;
} ttoy_Profile;

void ttoy_Profile_init(
    ttoy_Profile *self,
    const char *name);

void ttoy_Profile_destroy(
    ttoy_Profile *self);

void ttoy_Profile_setFlags(
    ttoy_Profile *self,
    uint32_t flags);

void
ttoy_Profile_clearFonts(
    ttoy_Profile *self);

ttoy_Font *
ttoy_Profile_getPrimaryFont(
    ttoy_Profile *self);

ttoy_ErrorCode
ttoy_Profile_setPrimaryFont(
    ttoy_Profile *self,
    const char *fontFace,
    float fontSize);

ttoy_ErrorCode
ttoy_Profile_addFallbackFont(
    ttoy_Profile *self,
    const char *fontFace);

float
ttoy_Profile_getFontSize(
    ttoy_Profile *self);

ttoy_ErrorCode
ttoy_Profile_setFontSize(
    ttoy_Profile *self,
    float fontSize);

void ttoy_Profile_getFonts(
    ttoy_Profile *self,
    ttoy_FontRefArray *fonts,
    ttoy_FontRefArray *boldFonts);

void ttoy_Profile_setBackgroundToy(
    ttoy_Profile *self,
    ttoy_BackgroundToy *backgroundToy);

ttoy_BackgroundToy *
ttoy_Profile_getBackgroundToy(
    ttoy_Profile *self);

void ttoy_Profile_setTextToy(
    ttoy_Profile *self,
    ttoy_TextToy *textToy);

ttoy_TextToy *
ttoy_Profile_getTextToy(
    ttoy_Profile *self);


#endif
