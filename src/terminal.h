#ifndef SHELLTOY_TERMINAL_H_
#define SHELLTOY_TERMINAL_H_

#include <SDL.h>
#include <libtsm.h>

#include "pty.h"

typedef struct {
  SDL_Window *window;
  st_PTY pty;
  struct tsm_screen *screen;
  struct tsm_vte *vte;
} st_Terminal;

void st_Terminal_init(st_Terminal *self);
void st_Terminal_destroy(st_Terminal *self);

#endif
