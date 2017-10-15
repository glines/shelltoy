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
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <ttoy/version.h>
#include "config.h"
#include "fonts.h"
#include "logging.h"
#include "terminal.h"

struct ttoy_Main {
  ttoy_Config config;
  ttoy_Terminal terminal;
} ttoy;

void ttoy_initSDL() {
  SDL_Init(SDL_INIT_VIDEO);
}

void ttoy_quitSDL() {
  SDL_Quit();
}

void ttoy_dispatchEvents() {
  /* Dispatch all events in the SDL event queue */
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_WINDOWEVENT:
        switch (event.window.event) {
          case SDL_WINDOWEVENT_SIZE_CHANGED:
            /* TODO: Support event dispatch to multiple terminals here */
            /* Inform the terminal of the new window size */
            ttoy_Terminal_windowSizeChanged(&ttoy.terminal,
                event.window.data1,  /* width */
                event.window.data2  /* height */
                );
            break;
        }
        break;
      case SDL_TEXTINPUT:
        /* NOTE: Control keys are handled separately from text input */
        ttoy_Terminal_textInput(&ttoy.terminal,
            event.text.text  /* text */
            );
        break;
      case SDL_KEYDOWN:
        if (event.key.repeat) {
          /* FIXME: For some reason, SDL key repeat events are
           * completely useless. Ordinary, non-repeat events are
           * actually repeated, while the "repeat" events result in
           * duplicate key events. */
          /* FIXME: The key repeat rate for certain keys, backspace in
           * particular, does not appear to match the repeat rate
           * configured on the system. */
          /* FIXME: Because SDL key repeat events do not work, it is
           * difficult to check for key events that originate outside of
           * the window. This is problematic for alt+tab events. */
          break;
        }
        /* Stop receiving text input events while certain modifier
         * keys are pressed */
        {
          int skip = 0;
          switch (event.key.keysym.sym) {
            case SDLK_LALT:
            case SDLK_LCTRL:
            case SDLK_RALT:
            case SDLK_RCTRL:
              skip = 1;
              SDL_StopTextInput();
              break;
          }
          if (skip)
            break;
        }
        ttoy_Terminal_keyInput(&ttoy.terminal,
            event.key.keysym.sym,  /* virtual key code */
            event.key.keysym.mod  /* modifiers */
            );
        break;
      case SDL_KEYUP:
        /* Resume receiving text input events once all modifier keys are
         * released */
        switch (event.key.keysym.sym) {
          case SDLK_LALT:
          case SDLK_LCTRL:
          case SDLK_RALT:
          case SDLK_RCTRL:
            if ((SDL_GetModState() & (
                  KMOD_LALT |
                  KMOD_LCTRL |
                  KMOD_RALT |
                  KMOD_RCTRL)) == 0)
            {
              SDL_StartTextInput();
            }
            break;
        }
        break;
      /* TODO: Handle SDL_WINDOWEVENT_CLOSE events if we ever have more than
       * one terminal at a time. */
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        ttoy_Terminal_mouseButton(&ttoy.terminal,
            &event.button  /* event */
            );
        break;
      case SDL_MOUSEMOTION:
        ttoy_Terminal_mouseMotion(&ttoy.terminal,
            &event.motion  /* event */
            );
        break;
      case SDL_QUIT:
        exit(EXIT_SUCCESS);
        break;
      default:
        if (event.type == ttoy_PTY_eventType()) {
          int error;
          /* Instruct the pty to read from the pseudo terminal */
          ttoy_PTY *pty = (ttoy_PTY *)event.user.data1;
          error = ttoy_PTY_read(pty);
          if (error == EWOULDBLOCK) {
            /* TODO: Reset the epoll handle so that we will get a pty event
             * soon? */
            /* TODO: Should we simply schedule a new pty event? */
          }
          /* FIXME: We need to draw immediately after the screen changes? */
          /* FIXME: A flag should be set to re-draw now that the screen has
           * changed */
        }
    }
  }
}

/* This routine handles SIGCHLD signals which we recieve when any of our
 * child processes (typically shells) exit. */
void ttoy_childExit(int signal) {
  SDL_Event event;
  pid_t pid;
  int status;

  assert(signal == SIGCHLD);

  /* Loop for the case that more than one child has exited */
  while (1) {
    /* Check for waitable children without blocking */
    pid = waitpid(
        -1,  /* pid (wait for any child process) */
        &status,  /* status */
        WNOHANG  /* options */
        );
    if (pid == 0) {
      /* No children are waitable; we are done here. */
      return;
    } else if (pid < 0) {
      /* Some sort of error. Hmm... */
      return;
    }
    /* TODO: Look for the terminal associated with the child that has exited */
    /* XXX: We only have one terminal emulator window at the moment, so here we
     * simply call exit(). */
    assert(ttoy.terminal.pty.child == pid);
    fprintf(stderr, "Our child process %d has exited\n", pid);  /* XXX */
    /* Push an SDL quit event so that we can cleanly exit */
    event.type = SDL_QUIT;
    if (SDL_PushEvent(&event) < 0) {
      fprintf(stderr, "Failed to push quit event to SDL\n");
      /* TODO: Fail gracefully */
      assert(0);
    }
  }
}

void ttoy_writeConfig() {
  /* ttoy_Config_write(&ttoy.config); */
}

void ttoy_destroyConfig() {
  ttoy_Config_destroy(&ttoy.config);
}

void ttoy_destroyTerminal() {
  ttoy_Terminal_destroy(&ttoy.terminal);
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
    "Usage: ttoy [options]\n"
    "       ttoy [options] -- [shell] [shell arguments]\n"
    "\n"
    "Options:\n"
    ;

  fprintf(stderr, "%s",
      helpString);
  print_option("-h, --help", "Print this help text and exit");
  print_option("-v, --version", "Print the ttoy version and exit");
  print_option("--config <file>",
      "Path to ttoy config file, overriding the default");
  print_option("-p, --profile <name>",
      "Name of the profile to use for ttoy instance");
  print_option("--plugin-path <dir>",
      "Directory path in which ttoy will look for plugins");
}

void print_version() {
  fprintf(stderr,
      "ttoy %s\n",
      TTOY_QUALIFIED_VERSION);
}

/* TODO: Load 'toy' information from JSON files with '.toy' extensions */
/* TODO: Include a small font embedded in the ttoy binary? Hmm... */

int main(int argc, char** argv) {
  char *configFilePath, *profileName, *pluginPath;
  size_t len;
  ttoy_Profile *profile;
  ttoy_ErrorCode error;
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
          TTOY_LOG_ERROR("%s", "More than one config file specified; ignoring");
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
          TTOY_LOG_ERROR("%s", "More than one profile specified; ignoring");
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
      case '?':
      case ':':
        print_help();
        return EXIT_FAILURE;
      default:
        assert(longindex >= 0);  /* FIXME: I'm not sure what longindex is set
                                    to in case the option does not exist */
        if (strcmp(long_options[longindex].name, "plugin-path") == 0) {
          if (pluginPath != NULL) {
            TTOY_LOG_ERROR("%s", "More than one plugin path specified; ignoring");
            break;
          }
          fprintf(stderr, "Using plugin path: '%s'\n", optarg);
          len = strlen(optarg);
          pluginPath = (char *)malloc(len + 1);
          strcpy(pluginPath, optarg);
        }
    }
  }

  /* Register a signal handler for handling child processes (typically shells)
   * that have exited */
  {
    const struct sigaction sigchldAction = {
      .sa_handler = ttoy_childExit,
      .sa_flags = SA_NOCLDSTOP,  /* ignore suspended/resumed children */
    };
    /* FIXME: This might interfere with the operation of grantpt(3). Do we
     * actually need a signal handler for SIGCHLD? */
    sigaction(
        SIGCHLD,  /* signum */
        &sigchldAction,  /* act */
        NULL  /* oldact */
        );
  }

  ttoy_Fonts_init();
  atexit(ttoy_Fonts_destroy);

  /* Prepare the configuration */
  ttoy_Config_init(&ttoy.config);
  atexit(ttoy_destroyConfig);
  if (pluginPath != NULL) {
    ttoy_Config_setPluginPath(&ttoy.config, pluginPath);
  }
  if (configFilePath != NULL) {
    /* Config file path was given; read the configuration from file */
    error = ttoy_Config_setConfigFilePath(&ttoy.config, configFilePath);
    if (error != TTOY_NO_ERROR) {
      TTOY_LOG_ERROR_CODE(error);
      exit(EXIT_FAILURE);
    }
  } else {
    /* Config file path not given; look for the config file in the default
     * locations */
    error = ttoy_Config_findConfigFile(&ttoy.config);
    if (error == TTOY_ERROR_CONFIG_FILE_NOT_FOUND) {
      /* Could not find config file; we're using the default configuration */
      error = ttoy_Config_createDefaultConfigFile(&ttoy.config);
      if (error != TTOY_NO_ERROR) {
        TTOY_LOG_ERROR_CODE(error);
      }
    } else if (error != TTOY_NO_ERROR) {
      TTOY_LOG_ERROR_CODE(error);
      exit(EXIT_FAILURE);
    }
  }

  /* Get the terminal profile from the configuration */
  if (profileName != NULL) {
    error = ttoy_Config_getProfile(&ttoy.config,
        profileName,  /* name */
        &profile  /* profile */
        );
    if (error != TTOY_NO_ERROR) {
      TTOY_LOG_ERROR_CODE(error);
      exit(EXIT_FAILURE);
    }
  } else {
    error = ttoy_Config_getDefaultProfile(&ttoy.config,
        &profile  /* profile */
        );
    if (error != TTOY_NO_ERROR) {
      TTOY_LOG_ERROR_CODE(error);
      exit(EXIT_FAILURE);
    }
  }

  ttoy_initSDL();
  atexit(ttoy_quitSDL);

  if (argc - optind == 0) {
    /* No shell was given; we check the SHELL environment variable */
    char *shell = getenv("SHELL");

    if (shell == NULL) {
      TTOY_LOG_ERROR("%s", "SHELL environment variable not set");
      shell_argv_buff[0] = "/usr/bin/env";
      shell_argv_buff[1] = "bash";
      shell_argv_buff[2] = "-il";  /* interactive + login */
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

  ttoy_Terminal_init(&ttoy.terminal,
    profile,  /* profile */
    shell_argc,  /* argc */
    shell_argv  /* argv */
    );
  atexit(ttoy_destroyTerminal);

  SDL_StartTextInput();  /* Receive text input by default */
  SDL_GL_SetSwapInterval(1);  /* Wait for vsync */
  while (1) {
    ttoy_dispatchEvents();
    ttoy_Terminal_draw(&ttoy.terminal);
    /* FIXME: We should avoid drawing if the terminal window has not
     * changed. */
  }

  assert(0);  /* Should never reach here */
  return EXIT_FAILURE;
}
