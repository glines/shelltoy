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

#include <SDL.h>
#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <libtsm.h>
#include <stdlib.h>

#include "config.h"
#include "fonts.h"
#include "logging.h"
#include "terminal.h"

struct st_Shelltoy {
  st_Config config;
  st_Terminal terminal;
} shelltoy;

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
    switch (event.type) {
      case SDL_WINDOWEVENT:
        switch (event.window.event) {
          case SDL_WINDOWEVENT_SIZE_CHANGED:
            /* TODO: Support event dispatch to multiple terminals here */
            /* Inform the terminal of the new window size */
            st_Terminal_windowSizeChanged(&shelltoy.terminal,
                event.window.data1,  /* width */
                event.window.data2  /* height */
                );
            break;
        }
        break;
      case SDL_TEXTINPUT:
        /* FIXME: Handle control characters separately from text input */
        st_Terminal_textInput(&shelltoy.terminal,
            event.text.text  /* text */
            );
        break;
      case SDL_KEYDOWN:
        st_Terminal_keyInput(&shelltoy.terminal,
            event.key.keysym.sym,  /* keycode */
            event.key.keysym.mod  /* modifiers */
            );
        break;
      /* TODO: Handle SDL_WINDOWEVENT_CLOSE events if we ever have more than
       * one terminal at a time. */
      case SDL_QUIT:
        exit(EXIT_SUCCESS);
        break;
      default:
        if (event.type == st_PTY_eventType()) {
          int error;
          fprintf(stderr, "Recieved PTY event\n");
          /* Instruct the pty to read from the pseudo terminal */
          st_PTY *pty = (st_PTY *)event.user.data1;
          error = st_PTY_read(pty);
          if (error == EWOULDBLOCK) {
            /* TODO: Reset the epoll handle so that we will get a pty event
             * soon? */
            /* TODO: Should we simply schedule a new pty event? */
          }
          /* FIXME: We need to draw immediately after the screen changes */
          /* FIXME: A flag should be set to re-draw now that the screen has
           * changed */
        }
    }
  }
}

void st_destroyConfig() {
  st_Config_destroy(&shelltoy.config);
}

void st_destroyTerminal() {
  st_Terminal_destroy(&shelltoy.terminal);
}

/* TODO: Load 'toy' information from JSON files with '.toy' extensions */
/* TODO: Load toys from shared libraries */

/* TODO: Actually get this font path as user input */
const char *FONT_FACE_PATH = "/nix/store/fvwp39z54ka2s7h3gawhfmayrqjnd05a-dejavu-fonts-2.37/share/fonts/truetype/DejaVuSansMono.ttf";
/* TODO: We might even want to read a small font into memory */

int main(int argc, char** argv) {
  char *configFilePath, *profileName;
  size_t len;
  st_Profile *profile;
  st_ErrorCode error;

  configFilePath = NULL;
  profileName = NULL;

  /* Parse command line arguments */
  while (1) {
    int c;
    static struct option long_options[] = {
      {"config", required_argument,    0, 'c' },
      {    NULL,                 0, NULL,   0 },
    };

    c = getopt_long(
        argc, argv,
        "",  /* optstring */
        long_options,  /* longopts */
        NULL  /* longindex */
        );
    if (c == -1)
      break;

    switch (c) {
      case 'c':
        fprintf(stderr, "Using config file path: %s\n", optarg);
        len = strlen(optarg);
        configFilePath = (char *)malloc(len + 1);
        strcpy(configFilePath, optarg);
        break;
    }
  }

  st_Config_init(&shelltoy.config);
  atexit(st_destroyConfig);
  if (configFilePath != NULL) {
    /* Config file path was given; read the configuration from file */
    error = st_Config_setConfigFilePath(&shelltoy.config, configFilePath);
    if (error != ST_NO_ERROR) {
      ST_LOG_ERROR_CODE(error);
      exit(EXIT_FAILURE);
    }
  } else {
    /* Config file path not given; look for the config file in the default
     * locations */
    error = st_Config_findConfigFile(&shelltoy.config);
    if (error == ST_ERROR_CONFIG_FILE_NOT_FOUND) {
      /* Could not find config file; we're using the default configuration */
    } else if (error != ST_NO_ERROR) {
      ST_LOG_ERROR_CODE(error);
      exit(EXIT_FAILURE);
    }
  }

  /* Get the terminal profile from the configuration */
  if (profileName != NULL) {
    error = st_Config_getProfile(&shelltoy.config,
        profileName,  /* name */
        &profile  /* profile */
        );
    if (error != ST_NO_ERROR) {
      ST_LOG_ERROR_CODE(error);
      exit(EXIT_FAILURE);
    }
  } else {
    error = st_Config_getDefaultProfile(&shelltoy.config,
        &profile  /* profile */
        );
    if (error != ST_NO_ERROR) {
      ST_LOG_ERROR_CODE(error);
      exit(EXIT_FAILURE);
    }
  }

  st_initSDL();
  atexit(st_quitSDL);
  st_Fonts_init();
  atexit(st_Fonts_destroy);

  st_Terminal_init(&shelltoy.terminal,
    profile,  /* profile */
    argc - optind, &argv[optind]);
  atexit(st_destroyTerminal);

  while (1) {
    st_dispatchEvents();
    st_Terminal_draw(&shelltoy.terminal);
    SDL_Delay(50);
    /* TODO: Write a more intelligent loop that waits for vsync, etc. */
    /* FIXME: We really need to avoid drawing if the terminal window has not
     * changed. */
  }

  assert(0);  /* Should never reach here */
  return EXIT_FAILURE;
}
