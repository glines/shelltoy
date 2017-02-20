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

#ifndef SHELLTOY_ERROR_H_
#define SHELLTOY_ERROR_H_

/* Create an enum type for all of our error codes */
#define ST_START_ERROR_CODES \
  typedef enum st_ErrorCode_ {
#define ST_DECLARE_ERROR_CODE(code,string) \
    code,
#define ST_END_ERROR_CODES \
  } st_ErrorCode;
#undef SHELLTOY_ERROR_CODES_H_
#include <shelltoy/errorCodes.h>
#undef ST_START_ERROR_CODES
#undef ST_DECLARE_ERROR_CODE
#undef ST_END_ERROR_CODES

const char *st_ErrorString(
    st_ErrorCode error);

#endif
