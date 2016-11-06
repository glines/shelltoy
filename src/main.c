#include <SDL.h>
#include <libtsm.h>
#include <stdlib.h>

#include "fonts.h"
#include "pty.h"
#include "terminal.h"

typedef struct st_shell {
  SDL_Window *window;
} st_shell;

void st_initSDL() {
  SDL_Init(SDL_INIT_VIDEO);
}

void st_quitSDL() {
  SDL_Quit();
}

void st_dispatchEvents() {
  /* Dispatch all events in the SDL event queue */
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == st_PTY_eventType()) {
      fprintf(stderr, "Recieved PTY event\n");
      /* Instruct the pty to read from the pseudo terminal */
      st_PTY *pty = (st_PTY *)event.user.data1;
      st_PTY_read(pty);
    }
  }
}

/* TODO: Load 'toy' information from JSON files with '.toy' extensions */
/* TODO: Load toys from shared libraries */

/* TODO: Actually get this font path as user input */
const char *FONT_FACE_PATH = "/nix/store/fvwp39z54ka2s7h3gawhfmayrqjnd05a-dejavu-fonts-2.37/share/fonts/truetype/DejaVuSansMono.ttf";
/* TODO: We might even want to read a small font into memory */

int main(int argc, char** argv) {
  st_Terminal terminal;

  st_initSDL();

  st_Fonts_init();
//  st_Render_init();

  st_Fonts_loadMonospace(
      8,  /* width */
      15,  /* height */
      FONT_FACE_PATH  /* fontPath */
      );

  st_Terminal_init(&terminal);
  while (1) {
//    st_dispatchEvents();
    /* TODO: Draw stuff */
    SDL_Delay(50);
  }
  st_Terminal_destroy(&terminal);

  st_quitSDL();

  return EXIT_SUCCESS;
}
