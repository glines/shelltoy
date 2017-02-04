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

#ifndef SHELLTOY_TERMINAL_H_
#define SHELLTOY_TERMINAL_H_

#include <SDL.h>
#include <libtsm.h>

#include "profile.h"
#include "pty.h"

struct st_Terminal_Internal;

typedef struct {
  SDL_Window *window;
  SDL_GLContext glContext;
  st_PTY pty;
  struct tsm_screen *screen;
  struct tsm_vte *vte;
  int width, height;
  int columns, rows;

  struct st_Terminal_Internal *internal;
} st_Terminal;

void st_Terminal_init(
    st_Terminal *self,
    st_Profile *profile,
    int argc,
    char **argv);
void st_Terminal_destroy(st_Terminal *self);

void st_Terminal_windowSizeChanged(
    st_Terminal *self,
    int width,
    int height);

void st_Terminal_updateScreen(st_Terminal *self);

void st_Terminal_draw(st_Terminal *self);

void st_Terminal_textInput(
    st_Terminal *self,
    const char *text);
void st_Terminal_keyInput(
    st_Terminal *self,
    SDL_Keycode keycode,
    uint16_t modifiers);

st_ErrorCode
st_Terminal_increaseFontSize(
    st_Terminal *self);

st_ErrorCode
st_Terminal_decreaseFontSize(
    st_Terminal *self);

#endif
