#ifndef SHELLTOY_TERMINAL_H_
#define SHELLTOY_TERMINAL_H_

#include <SDL.h>
#include <libtsm.h>

#include "pty.h"
#include "screenRenderer.h"

struct st_Terminal_Internal;

typedef struct {
  SDL_Window *window;
  SDL_GLContext glContext;
  st_PTY pty;
  struct tsm_screen *screen;
  struct tsm_vte *vte;
  int width, height;

  struct st_Terminal_Internal *internal;
} st_Terminal;

void st_Terminal_init(st_Terminal *self);
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

#endif
