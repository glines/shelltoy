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

#ifndef SHELLTOY_PROFILE_H_
#define SHELLTOY_PROFILE_H_

#include <inttypes.h>

#include <shelltoy/backgroundToy.h>
#include <shelltoy/error.h>
#include <shelltoy/textToy.h>

#include "color.h"

struct st_Profile_Internal_;
typedef struct st_Profile_Internal_ st_Profile_Internal;

typedef enum st_Profile_Flag_ {
  ST_PROFILE_ANTIALIAS_FONT = 1 << 0,
  ST_PROFILE_BRIGHT_IS_BOLD = 1 << 1,
} st_Profile_Flag;

typedef struct st_Profile_ {
  char *name, *fontFace, *fontPath;
  float fontSize;
  uint32_t flags;
  st_ColorScheme colorScheme;
  st_Profile_Internal *internal;
} st_Profile;

void st_Profile_init(
    st_Profile *self,
    const char *name);

void st_Profile_destroy(
    st_Profile *self);

void st_Profile_setFlags(
    st_Profile *self,
    uint32_t flags);

st_ErrorCode st_Profile_setFont(
    st_Profile *self,
    const char *fontFace,
    float fontSize);

void st_Profile_setBackgroundToy(
    st_Profile *self,
    st_BackgroundToy *backgroundToy);

st_BackgroundToy *
st_Profile_getBackgroundToy(
    st_Profile *self);

void st_Profile_setTextToy(
    st_Profile *self,
    st_TextToy *textToy);

st_TextToy *
st_Profile_getTextToy(
    st_Profile *self);


#endif
