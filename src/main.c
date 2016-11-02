#include <SDL.h>
#include <libtsm.h>
#include <stdlib.h>

#include "fonts.h"
#include "terminal.h"

typedef struct st_shell {
  SDL_Window *window;
} st_shell;

void init_sdl() {
  SDL_Init(SDL_INIT_VIDEO);
}

void quit_sdl() {
  SDL_Quit();
}

void st_shell_init(st_shell *self) {
  self->window = SDL_CreateWindow(
      "ShellToy",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      640,
      480,
      SDL_WINDOW_OPENGL
      );
}

void st_shell_destroy(st_shell *self) {
  SDL_DestroyWindow(self->window);
}

/* TODO: Load 'toy' information from JSON files with '.toy' extensions */
/* TODO: Load toys from shared libraries */

int main(int argc, char** argv) {
  st_shell shell;
  st_Terminal terminal;

  st_initFreetype();
  init_sdl();

  /*
  st_loadFontFace();
  st_loadGlyph('~');
  */

  st_Terminal_init(&terminal);
  SDL_Delay(5000);
  st_Terminal_destroy(&terminal);

  st_shell_init(&shell);

  SDL_Delay(3000);

  st_shell_destroy(&shell);

  quit_sdl();

  return EXIT_SUCCESS;
}
