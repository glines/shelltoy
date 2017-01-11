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

#ifndef SHELLTOY_ERROR_H_
#define SHELLTOY_ERROR_H_

typedef enum st_ErrorCode_ {
  ST_NO_ERROR = 0,
  ST_ERROR_CONFIG,
  ST_ERROR_CONFIG_FILE_FORMAT,
  ST_ERROR_CONFIG_FILE_NOT_FOUND,
  ST_ERROR_CONFIG_FILE_PATH_NOT_SET,
  ST_ERROR_CONFIG_FILE_READ,
  ST_ERROR_FONT_NOT_FOUND,
  ST_ERROR_OUT_OF_MEMORY,
  ST_ERROR_PROFILE_NOT_FOUND,
  ST_ERROR_UNKNOWN_COLOR_CODE,
} st_ErrorCode;

const char *st_ErrorString(
    st_ErrorCode error);

#endif
