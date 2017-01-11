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

#ifndef SHELLTOY_COLOR_H_
#define SHELLTOY_COLOR_H_

typedef struct st_Color_ {
  uint8_t rgb[3];
} st_Color;

typedef enum st_ColorEnum_ {
  ST_COLOR_0 = 0,
  ST_COLOR_1,
  ST_COLOR_2,
  ST_COLOR_3,
  ST_COLOR_4,
  ST_COLOR_5,
  ST_COLOR_6,
  ST_COLOR_7,
  ST_COLOR_8,
  ST_COLOR_9,
  ST_COLOR_10,
  ST_COLOR_11,
  ST_COLOR_12,
  ST_COLOR_13,
  ST_COLOR_14,
  ST_COLOR_15,
  ST_COLOR_FOREGROUND,
  ST_COLOR_BACKGROUND,
  ST_NUM_COLORS,
} st_ColorEnum;

typedef struct st_ColorScheme_ {
  st_Color colors[ST_NUM_COLORS];
} st_ColorScheme;

#endif
