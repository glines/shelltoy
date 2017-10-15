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

#ifndef TTOY_TERMINAL_H_
#define TTOY_TERMINAL_H_

#include <SDL.h>
#include <libtsm.h>

#include "profile.h"
#include "pty.h"

struct ttoy_Terminal_Internal;

typedef struct {
  SDL_Window *window;
  SDL_GLContext glContext;
  ttoy_PTY pty;
  struct tsm_screen *screen;
  struct tsm_vte *vte;
  int width, height;
  int cellWidth, cellHeight;
  int columns, rows;

  struct ttoy_Terminal_Internal *internal;
} ttoy_Terminal;

void ttoy_Terminal_init(
    ttoy_Terminal *self,
    ttoy_Profile *profile,
    int argc,
    char **argv);
void ttoy_Terminal_destroy(ttoy_Terminal *self);

void ttoy_Terminal_windowSizeChanged(
    ttoy_Terminal *self,
    int width,
    int height);

void ttoy_Terminal_updateScreen(ttoy_Terminal *self);

void ttoy_Terminal_draw(ttoy_Terminal *self);

void ttoy_Terminal_textInput(
    ttoy_Terminal *self,
    const char *text);
void ttoy_Terminal_keyInput(
    ttoy_Terminal *self,
    SDL_Keycode keycode,
    uint16_t modifiers);

void ttoy_Terminal_mouseButton(
    ttoy_Terminal *self,
    const SDL_MouseButtonEvent *event);
void ttoy_Terminal_mouseMotion(
    ttoy_Terminal *self,
    const SDL_MouseMotionEvent *event);

ttoy_ErrorCode
ttoy_Terminal_increaseFontSize(
    ttoy_Terminal *self);

ttoy_ErrorCode
ttoy_Terminal_decreaseFontSize(
    ttoy_Terminal *self);

ttoy_ErrorCode
ttoy_Terminal_setSelectionClipboard(
    ttoy_Terminal *self,
    const char *selection);

#endif
