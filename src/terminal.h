#ifndef SHELLTOY_TERMINAL_H_
#define SHELLTOY_TERMINAL_H_

#include <SDL.h>
#include <libtsm.h>

#include "pty.h"
#include "screenRenderer.h"

typedef struct {
  SDL_Window *window;
  SDL_GLContext glContext;
  st_PTY pty;
  st_ScreenRenderer screenRenderer;
  struct tsm_screen *screen;
  struct tsm_vte *vte;
} st_Terminal;

void st_Terminal_init(st_Terminal *self);
void st_Terminal_destroy(st_Terminal *self);

void st_Terminal_updateScreen(st_Terminal *self);

void st_Terminal_draw(st_Terminal *self);

#endif
