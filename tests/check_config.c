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

#include <check.h>
#include <stdlib.h>

#include "../src/config.h"

START_TEST(test_Config_checkDefaultProfile)
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

Suite *config_test_suite() {
  Suite *s;
  TCase *tc_profiles;

  s = suite_create("Config");

  /* Profiles test case */
  tc_profiles = tcase_create("Profiles");

  tcase_add_test(tc_profiles, test_Config_checkDefaultProfile);
  suite_add_tcase(s, tc_profiles);

  return s;
}

int main(int argc, char **argv) {
  int number_failed;
  Suite *s;
  SRunner *sr;

  s = config_test_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
