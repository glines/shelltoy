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

#ifndef TTOY_COLOR_H_
#define TTOY_COLOR_H_

typedef struct ttoy_Color_ {
  uint8_t rgb[3];
} ttoy_Color;

typedef enum ttoy_ColorEnum_ {
  TTOY_COLOR_0 = 0,
  TTOY_COLOR_1,
  TTOY_COLOR_2,
  TTOY_COLOR_3,
  TTOY_COLOR_4,
  TTOY_COLOR_5,
  TTOY_COLOR_6,
  TTOY_COLOR_7,
  TTOY_COLOR_8,
  TTOY_COLOR_9,
  TTOY_COLOR_10,
  TTOY_COLOR_11,
  TTOY_COLOR_12,
  TTOY_COLOR_13,
  TTOY_COLOR_14,
  TTOY_COLOR_15,
  TTOY_COLOR_FOREGROUND,
  TTOY_COLOR_BACKGROUND,
  TTOY_NUM_COLORS,
} ttoy_ColorEnum;

typedef struct ttoy_ColorScheme_ {
  ttoy_Color colors[TTOY_NUM_COLORS];
} ttoy_ColorScheme;

#endif
