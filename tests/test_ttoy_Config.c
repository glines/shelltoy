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

#include "../src/config.h"

#include "test_ttoy_Config.h"

START_TEST(ttoy_test_Config_defaultProfile)
{
  ttoy_Config config;
  ttoy_Profile *defaultProfile;

  ttoy_Config_init(&config);
  mark_point();
  ttoy_Config_getDefaultProfile(&config,
      &defaultProfile);

  ck_assert(defaultProfile != NULL);

  ttoy_Config_destroy(&config);
}
END_TEST

Suite *ttoy_Config_test_suite() {
  Suite *s;
  TCase *tc;

  s = suite_create("ttoy_Config");

  tc = tcase_create("defaultProfile");
  tcase_add_test(tc, ttoy_test_Config_defaultProfile);
  suite_add_tcase(s, tc);

  return s;
}
