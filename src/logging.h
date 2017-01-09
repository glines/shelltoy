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

#ifndef SHELLTOY_LOGGING_H_
#define SHELLTOY_LOGGING_H_

#include <assert.h>

#include "error.h"

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__ )

#define ST_LOG_ERROR(format, ...) \
  st_logError(__FILENAME__, __LINE__, \
      format, ##__VA_ARGS__)

#define ST_LOG_ERROR_CODE(error) \
  ST_LOG_ERROR("%s", st_ErrorString(error))

#define ST_ASSERT_ERROR_CODE(error) \
  do { \
    ST_LOG_ERROR("%s", st_ErrorString(error)); \
    assert(error == ST_NO_ERROR); \
  } while (0)

void st_logError(
  const char *file,
  int line,
  const char *format,
  ...);

#endif
