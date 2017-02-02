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

void print_option(
    const char *flags,
    const char *description)
{
  fprintf(stderr,
      "  %-20s  %s\n",
      flags,
      description);
}

void print_help() {
  static const char *helpString =
    "Usage: shelltoy [options]\n"
    "       shelltoy [options] -- [shell] [shell arguments]\n"
    "\n"
    "Options:\n"
    ;

  fprintf(stderr, "%s",
      helpString);
  print_option("-h, --help", "Print this help text and exit");
  print_option("-v, --version", "Print the Shelltoy version and exit");
  print_option("--config <file>",
      "Path to Shelltoy config file, overriding the default");
  print_option("-p, --profile <name>",
      "Name of the profile to use for Shelltoy instance");
  print_option("--plugin-path <dir>",
      "Directory path in which Shelltoy will look for plugins");
}

void print_version() {
  fprintf(stderr,
      "Shelltoy %s\n",
      SHELLTOY_QUALIFIED_VERSION);
}

/* TODO: Load 'toy' information from JSON files with '.toy' extensions */
/* TODO: Include a small font embedded in the shelltoy binary? Hmm... */

int main(int argc, char** argv) {
  char *configFilePath, *profileName, *pluginPath;
  size_t len;
  st_Profile *profile;
  st_ErrorCode error;
  char **shell_argv;
  int shell_argc;
  char *shell_argv_buff[4];

  configFilePath = NULL;
  profileName = NULL;
  pluginPath = NULL;

  /* Parse command line arguments */
  while (1) {
    int c, longindex;
    static struct option long_options[] = {
      { "config",      required_argument,    0, 'c' },
      { "help",        no_argument,          0, 'h' },
      { "profile",     required_argument,    0, 'p' },
      { "plugin-path", required_argument,    0, 0 },
      { "version",     no_argument,          0, 'v' },
      { NULL,          0,                 NULL, 0 },
    };

    c = getopt_long(
        argc, argv,
        "c:hp:v",  /* optstring */
        long_options,  /* longopts */
        &longindex  /* longindex */
        );
    if (c == -1)
      break;

    switch (c) {
      case 'c':
        if (configFilePath != NULL) {
          ST_LOG_ERROR("%s", "More than one config file specified; ignoring");
          break;
        }
        fprintf(stderr, "Using config file path: %s\n", optarg);
        len = strlen(optarg);
        configFilePath = (char *)malloc(len + 1);
        strcpy(configFilePath, optarg);
        break;
      case 'h':
        print_help();
        return EXIT_SUCCESS;
      case 'p':
        if (profileName != NULL) {
          ST_LOG_ERROR("%s", "More than one profile specified; ignoring");
          break;
        }
        fprintf(stderr, "Using profile: '%s'\n", optarg);
        len = strlen(optarg);
        profileName = (char *)malloc(len + 1);
        strcpy(profileName, optarg);
        break;
      case 'v':
        print_version();
        return EXIT_SUCCESS;
        break;
      default:
        assert(longindex >= 0);  /* FIXME: I'm not sure what longindex is set
                                    to in case the option does not exist */
        if (strcmp(long_options[longindex].name, "plugin-path") == 0) {
          if (pluginPath != NULL) {
            ST_LOG_ERROR("%s", "More than one plugin path specified; ignoring");
            break;
          }
          fprintf(stderr, "Using plugin path: '%s'\n", optarg);
          len = strlen(optarg);
          pluginPath = (char *)malloc(len + 1);
          strcpy(pluginPath, optarg);
        }
    }
  }

  st_Config_init(&shelltoy.config);
  atexit(st_destroyConfig);
  if (pluginPath != NULL) {
    st_Config_setPluginPath(&shelltoy.config, pluginPath);
  }
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

  if (argc - optind == 0) {
    /* No shell was given; we check the SHELL environment variable */
    char *shell = getenv("SHELL");

    if (shell == NULL) {
      ST_LOG_ERROR("%s", "SHELL environment variable not set");
      shell_argv_buff[0] = "/usr/bin/env";
      shell_argv_buff[1] = "bash";
      shell_argv_buff[2] = "-i";  /* interactive + login */
      shell_argv_buff[3] = NULL;
      shell_argv = shell_argv_buff;
      shell_argc = 3;
    } else {
      shell_argv_buff[0] = shell;
      shell_argv_buff[1] = "-i";  /* interactive */
      shell_argv_buff[2] = NULL;
      shell_argv = shell_argv_buff;
      shell_argc = 2;
    }
  } else {
    shell_argc = argc - optind;
    shell_argv = &argv[optind];
  }

  st_Terminal_init(&shelltoy.terminal,
    profile,  /* profile */
    shell_argc,  /* argc */
    shell_argv  /* argv */
    );
  atexit(st_destroyTerminal);

  SDL_GL_SetSwapInterval(1);  /* Wait for vsync */
  while (1) {
    st_dispatchEvents();
    st_Terminal_draw(&shelltoy.terminal);
    /* FIXME: We should avoid drawing if the terminal window has not
     * changed. */
  }

  assert(0);  /* Should never reach here */
  return EXIT_FAILURE;
}
