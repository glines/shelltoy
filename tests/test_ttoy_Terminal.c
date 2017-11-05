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

#include <string.h>

#include "../src/config.h"
#include "../src/fonts.h"
#include "../src/terminal.h"

#include "test_ttoy_Terminal.h"

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

void ttoy_Terminal_oscCallback(
    char *osc_str,
    ttoy_Terminal *self);
START_TEST(ttoy_test_Terminal_oscCallback)
{
  ttoy_Config config;
  ttoy_Profile *defaultProfile;
  ttoy_Terminal terminal;
  char *shell_argv[3];
  char command_buf[256];

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

  /* Set the window title */
  strcpy(command_buf, "2;test window title");
  ttoy_Terminal_oscCallback(
      command_buf,
      &terminal);
  ck_assert_str_eq(
      SDL_GetWindowTitle(terminal.window),
      "test window title");

  /* Ignore missing semicolons */
  strcpy(command_buf, "2");
  ttoy_Terminal_oscCallback(
      command_buf,
      &terminal);
  ck_assert_str_eq(
      SDL_GetWindowTitle(terminal.window),
      "test window title");

  /* Set the window title with semicolons */
  strcpy(command_buf, "2;window;title;with;semicolons");
  ttoy_Terminal_oscCallback(
      command_buf,
      &terminal);
  ck_assert_str_eq(
      SDL_GetWindowTitle(terminal.window),
      "window;title;with;semicolons");

  /* Ignore unknown codes */
  strcpy(command_buf, "42;foo");
  ttoy_Terminal_oscCallback(
      command_buf,
      &terminal);
  mark_point();

  /* Ignore non-integer codes */
  strcpy(command_buf, "foo;bar");
  ttoy_Terminal_oscCallback(
      command_buf,
      &terminal);
  mark_point();

  ttoy_Terminal_destroy(&terminal);
  mark_point();

  ttoy_Config_destroy(&config);
  mark_point();

  ttoy_Fonts_destroy();
}
END_TEST

Suite *ttoy_Terminal_test_suite() {
  Suite *s;
  TCase *tc;

  s = suite_create("ttoy_Terminal");

  tc = tcase_create("init");
  tcase_add_test(tc, ttoy_test_Terminal_init);
  suite_add_tcase(s, tc);

  tc = tcase_create("oscCallback");
  tcase_add_test(tc, ttoy_test_Terminal_oscCallback);
  suite_add_tcase(s, tc);

  return s;
}
