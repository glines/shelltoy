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

#include "../src/config.h"
#include "../src/fonts.h"
#include "../src/terminal.h"

START_TEST(ttoy_test_Terminal_init)
{
  ttoy_Config config;
  ttoy_Profile *defaultProfile;
  ttoy_Terminal terminal;
  char *shell_argv[3];

  ttoy_Fonts_init();
  mark_point();

  ttoy_Config_init(&config);
  mark_point();

  ttoy_Config_getDefaultProfile(&config,
      &defaultProfile);
  ck_assert(defaultProfile != NULL);

  shell_argv[0] = "/usr/bin/env";
  shell_argv[1] = "true";
  shell_argv[2] = NULL;
  ttoy_Terminal_init(&terminal,
      defaultProfile,  /* profile */
      3,  /* argc */
      shell_argv  /* argv */
      );
  mark_point();

  ttoy_Terminal_destroy(&terminal);
  mark_point();

  ttoy_Config_destroy(&config);
  mark_point();

  ttoy_Fonts_destroy();
}
END_TEST

Suite *terminal_test_suite() {
  Suite *s;
  TCase *tc_init;

  s = suite_create("ttoy_Terminal");

  tc_init = tcase_create("init");

  tcase_add_test(tc_init, ttoy_test_Terminal_init);
  suite_add_tcase(s, tc_init);

  return s;
}

int main(int argc, char **argv) {
  int number_failed;
  Suite *s;
  SRunner *sr;

  s = terminal_test_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
