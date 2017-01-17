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

#include <assert.h>

#include "../extern/xkbcommon-keysyms.h"

#include "backgroundRenderer.h"
#include "common/glError.h"
#include "logging.h"
#include "profile.h"

#include "terminal.h"

/* Internal data structure */
struct st_Terminal_Internal {
  st_ScreenRenderer screenRenderer;
  st_BackgroundRenderer backgroundRenderer;
  st_GlyphRenderer glyphRenderer;
  st_Profile *profile;
};

/* Private methods */
void st_Terminal_tsmLogCallback(
    st_Terminal *self,
    const char *file,
    int line,
    const char *func,
    const char *subs,
    unsigned int sev,
    const char *format,
    va_list args);
void st_Terminal_tsmWriteCallback(
    struct tsm_vte *vte,
    const char *u8,
    size_t len,
    st_Terminal *self);
void st_Terminal_initWindow(st_Terminal *self);
void st_Terminal_initTSM(st_Terminal *self);
void st_Terminal_calculatePseudoTerminalSize(
    st_Terminal *self,
    int *columns,
    int *rows);

void st_Terminal_tsmLogCallback(
    st_Terminal *self,
    const char *file,
    int line,
    const char *func,
    const char *subs,
    unsigned int sev,
    const char *format,
    va_list args)
{
  /* TODO: Do something with this data */
  fprintf(stderr, "st_Terminal_tsmLogCallback() called\n");
}

void st_Terminal_tsmWriteCallback(
    struct tsm_vte *vte,
    const char *u8,
    size_t len,
    st_Terminal *self)
{
  /* Write to the pseudo terminal */
  st_PTY_write(&self->pty, u8, len);
}

void st_Terminal_ptyReadCallback(
    st_Terminal *self,
    const char *u8,
    size_t len)
{
  /* Give the output from our child process to the vte */
  tsm_vte_input(
      self->vte,  /* vte */
      u8,  /* u8 */
      len  /* len */
      );
  /* Update the terminal screen display */
  st_Terminal_updateScreen(self);
}

void st_Terminal_initWindow(st_Terminal *self) {
  /* Create the SDL window */
  self->window = SDL_CreateWindow(
      "Shelltoy",  /* title */
      SDL_WINDOWPOS_UNDEFINED,  /* x */
      SDL_WINDOWPOS_UNDEFINED,  /* y */
      640,  /* w */
      480,  /* w */
      SDL_WINDOW_OPENGL
      | SDL_WINDOW_RESIZABLE  /* flags */
      );
  if (self->window == NULL) {
    fprintf(stderr, "Failed to create SDL window: %s\n",
        SDL_GetError());
    /* TODO: Fail gracefully */
    assert(0);
  }
  /* Store the window dimensions */
  self->width = 640;
  self->height = 480;

  /* The terminal emulator recieves all text through the operating system's IME
   * interface. See: <https://wiki.libsdl.org/Tutorials/TextInput> */
  SDL_StartTextInput();

  /* Create an OpenGL context for our window */
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  self->glContext = SDL_GL_CreateContext(self->window);
  if (self->glContext == NULL) {
    fprintf(stderr, "Failed to initialize OpenGL context: %s\n",
        SDL_GetError());
    exit(EXIT_FAILURE);
  }
  /* Initialize GL entry points */
  glewExperimental = 1;
  GLenum error = glewInit();
  if (error != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW: %s\n",
        glewGetErrorString(error));
    exit(EXIT_FAILURE);
  }
  if (!GLEW_VERSION_3_3) {
    fprintf(stderr, "Requires OpenGL 3.3 core context or later.");
    exit(EXIT_FAILURE);
  }
  /* Swallow the error generated by GLEW. See:
   * <http://stackoverflow.com/a/20035078> */
  /* TODO: Suppress the message generated by swallowing this error */
  FORCE_CHECK_GL_ERROR();

  /* Configure the GL */
  st_Color *bgColor =
    &self->internal->profile->colorScheme.colors[ST_COLOR_BACKGROUND];
  glClearColor(
      (float)bgColor->rgb[0] / 255.0f,
      (float)bgColor->rgb[1] / 255.0f,
      (float)bgColor->rgb[2] / 255.0f,
      0.0f);
  FORCE_ASSERT_GL_ERROR();
  glClearDepth(1.0);
  FORCE_ASSERT_GL_ERROR();
  glEnable(GL_DEPTH_TEST);
  FORCE_ASSERT_GL_ERROR();
  glDepthFunc(GL_LESS);
  FORCE_ASSERT_GL_ERROR();
  glDisable(GL_CULL_FACE);  /* XXX */
  FORCE_ASSERT_GL_ERROR();
  glFrontFace(GL_CCW);
  FORCE_ASSERT_GL_ERROR();
  glViewport(0, 0, self->width, self->height);
  FORCE_ASSERT_GL_ERROR();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  FORCE_ASSERT_GL_ERROR();
  SDL_GL_SwapWindow(self->window);
}

void st_Terminal_initTSM(st_Terminal *self) {
  /* Initialize the screen and state machine provided by libtsm */
  tsm_screen_new(
      &self->screen,  /* out */
      (tsm_log_t)st_Terminal_tsmLogCallback,  /* log */
      self  /* log_data */
      );
  tsm_vte_new(
      &self->vte,  /* out */
      self->screen,  /* con */
      (tsm_vte_write_cb)st_Terminal_tsmWriteCallback,  /* write_cb */
      self,  /* data */
      (tsm_log_t)st_Terminal_tsmLogCallback,  /* log */
      self  /* log_data */
      );
}

void st_Terminal_calculatePseudoTerminalSize(
    st_Terminal *self,
    int *columns,
    int *rows)
{
  int cellWidth, cellHeight;
  st_GlyphRenderer_getCellSize(&self->internal->glyphRenderer,
      &cellWidth,  /* width */
      &cellHeight  /* height */
      );
  /* The pseudo terminal size is the number of (halfwidth) glyph cells that
   * will fit in the terminal window */
  *columns = self->width / cellWidth;
  *rows = self->height / cellHeight;
}

/* TODO: Allow the user to configure the shell command to invoke */
#define ENV_PATH "/usr/bin/env"
#define SHELL "bash"

static char *bash_argv[] = {
  "/usr/bin/env",
  "bash",
};

void st_Terminal_init(
    st_Terminal *self,
    st_Profile *profile,
    int argc,
    char **argv)
{
  size_t len;
  int ptyWidth, ptyHeight;
  char **argv_nullTerminated;
  if (argc == 0) {
    /* No shell was given; we check the SHELL environment variable */
    char *shell = getenv("SHELL");
    if (shell == NULL) {
      ST_LOG_ERROR("%s", "SHELL environment variable not set");
      /* TODO: Try our best to invoke bash */
      argv = bash_argv;
      argc = sizeof(bash_argv) / sizeof(bash_argv[0]);
    } else {
      argv = &shell;
      argc = 1;
    }
  }
  /* Copy arguments into a null terminated array */
  argv_nullTerminated = (char**)malloc(sizeof(char*) * (argc + 1));
  for (int i = 0; i < argc; ++i) {
    len = strlen(argv[i]);
    argv_nullTerminated[i] = (char*)malloc(len + 1);
    strcpy(argv_nullTerminated[i], argv[i]);
  }
  argv_nullTerminated[argc] = NULL;
  /* Allocate memory for internal data structures */
  self->internal = (struct st_Terminal_Internal *)malloc(
      sizeof(struct st_Terminal_Internal));
  self->internal->profile = profile;
  /* Initialize the SDL window */
  st_Terminal_initWindow(self);
  /* Initialize the glyph renderer */
  st_GlyphRenderer_init(&self->internal->glyphRenderer,
      profile  /* profile */
      );
  /* Initialize the screen renderer */
  st_ScreenRenderer_init(&self->internal->screenRenderer,
      &self->internal->glyphRenderer,  /* glyphRenderer */
      profile  /* profile */
      );
  /* Initialize the background renderer */
  st_BackgroundRenderer_init(
      &self->internal->backgroundRenderer,
      profile  /* profile */
      );
  /* Initialize the terminal state machine */
  st_Terminal_initTSM(self);
  /* Initialize the pseudo terminal and corresponding child process */
  st_Terminal_calculatePseudoTerminalSize(self,
      &ptyWidth,  /* ptyWidth */
      &ptyHeight  /* ptyHeight */
      );
  st_PTY_init(&self->pty,
      ptyWidth,  /* width */
      ptyHeight  /* height */
      );
  /* TODO: Construct a st_MonospaceFont object that combines multiple font
   * faces into one font that supports normal, bold, and wide glyphs */
  /* TODO: Calculate terminal width and height */
  st_PTY_startChild(&self->pty,
      argv_nullTerminated[0],  /* path */
      argv_nullTerminated,  /* argv */
      (st_PTY_readCallback_t)st_Terminal_ptyReadCallback,  /* callback */
      self  /* callback_data */
      );
  /* Free memory allocated for arguments */
  for (int i = 0; i < argc; ++i) {
    free(argv_nullTerminated[i]);
  }
  free(argv_nullTerminated);
  /* TODO: Start sending input from the child process to tsm_vte_input()? */
  /* TODO: Start sending keyboard input to tsm_vte_handle_keyboard()? */
}

void st_Terminal_destroy(st_Terminal *self) {
  /* Destroy all of the objects that we initialized */
  st_BackgroundRenderer_destroy(&self->internal->backgroundRenderer);
  st_ScreenRenderer_destroy(&self->internal->screenRenderer);
  st_GlyphRenderer_destroy(&self->internal->glyphRenderer);
  /* FIXME: Destroy the libtsm state machine */
  /* FIXME: Destroy the libtsm screen */
  st_PTY_destroy(&self->pty);
  /* Release memory for internal data structures */
  free(self->internal);
  /* FIXME: Close the SDL window */
}

void st_Terminal_windowSizeChanged(
    st_Terminal *self,
    int width,
    int height)
{
  int result;
  int newColumns, newRows;
  /* Store the new window width and height */
  self->width = width;
  self->height = height;
  fprintf(stderr, "new window dimensions: %dx%d\n", width, height);
  /* Calculate new pseudo terminal size */
  st_Terminal_calculatePseudoTerminalSize(self,
      &newColumns,  /* columns */
      &newRows  /* rows */
      );
  if ((newColumns != self->columns) || (newRows != self->rows))
  {
    self->columns = newColumns;
    self->rows = newRows;
    fprintf(stderr, "new size: %dx%d\n", self->columns, self->rows);
    /* Change the pseudo terminal screen size */
    result = tsm_screen_resize(
        self->screen,  /* con */
        self->columns,  /* x */
        self->rows  /* y */
        );
    if (result < 0) {
      fprintf(stderr, "Failed to resize libtsm screen\n");
      /* TODO: Fail gracefully */
      assert(0);
    }
    st_PTY_resize(&self->pty,
        self->columns,  /* width */
        self->rows  /* height */
        );
    /* Update the screen */
    st_ScreenRenderer_updateScreen(&self->internal->screenRenderer,
        self->screen,  /* screen */
        &self->internal->glyphRenderer  /* glyphRenderer */
        );
  }
  /* Update the GL viewport size */
  glViewport(0, 0, width, height);
  FORCE_ASSERT_GL_ERROR();
  /* Re-draw the screen */
  st_Terminal_draw(self);
  SDL_GL_SwapWindow(self->window);
}

void st_Terminal_updateScreen(st_Terminal *self) {
  /* TODO: The update should probably be queued as an event... maybe? */
  /* TODO: Read the character grid from the libtsm screen */
  /* TODO: Make sure we have loaded all of the glyphs that we need */
  /* TODO: Make sure the changes get rendered immediately (might be useful to
   * render even sooner than the toy can render) */
  st_ScreenRenderer_updateScreen(&self->internal->screenRenderer,
      self->screen,  /* screen */
      &self->internal->glyphRenderer  /* glyphRenderer */
      );
}

void st_Terminal_draw(st_Terminal *self) {
  /* Clear the screen */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  FORCE_ASSERT_GL_ERROR();

  /* Draw the background */
  st_BackgroundRenderer_draw(&self->internal->backgroundRenderer,
      self->width,  /* viewportWidth */
      self->height  /* viewportHeight */
      );

  /* Draw the glyphs on the screen */
  st_ScreenRenderer_draw(&self->internal->screenRenderer,
      &self->internal->glyphRenderer,  /* glyphRenderer */
      self->width,  /* viewportWidth */
      self->height  /* viewportHeight */
      );

  SDL_GL_SwapWindow(self->window);
}

void st_Terminal_textInput(
    st_Terminal *self,
    const char *text)
{
  uint32_t character;
  int result;

  /* FIXME: Convert the text from UTF-8 to UTF-32 (properly) */

  /* Iterate over the input text to generate corresponding keyboard events */
  for (int i = 0; text[i] != '\0'; ++i) {
    character = (uint32_t)text[i];
    /* FIXME: It's not clear what each of the arguments for
     * tsm_vte_handle_keyboard() need to be set to. In particular, we don't
     * actually have a keysym here. */
    result = tsm_vte_handle_keyboard(
        self->vte,  /* vte */
        character,  /* keysym */
        0,  /* ascii */
        0,  /* mods */
        character  /* unicode */
        );
    if (result) {
      /* FIXME: It's not clear what result signifies or what
       * tsm_screen_sb_reset() actually does. */
      tsm_screen_sb_reset(self->screen);
    }
  }
}
void st_Terminal_keyInput(
    st_Terminal *self,
    SDL_Keycode keycode,
    uint16_t modifiers)
{
  unsigned int modifiers_tsm;
  uint32_t key_xkb;
  int result;

  /* TODO: Convert the SDL modifier flags to libtsm modifier flags */
  modifiers_tsm = 0;
  if (modifiers & KMOD_CAPS)
    modifiers_tsm |= TSM_LOCK_MASK;
  if (modifiers & KMOD_CTRL)
    modifiers_tsm |= TSM_CONTROL_MASK;
  if (modifiers & KMOD_SHIFT)
    modifiers_tsm |= TSM_SHIFT_MASK;
  if (modifiers & KMOD_ALT)
    modifiers_tsm |= TSM_ALT_MASK;
  if (modifiers & KMOD_GUI)
    modifiers_tsm |= TSM_LOGO_MASK;

  /* TODO: Handle shift+pgup events to scroll through back buffer */

  /* Handle control key sequences */
  if (modifiers & KMOD_CTRL) {
    /* Convert SDL keys to XKB keys */
    switch (keycode) {
#define SDL_XKB(x) \
      case SDLK_ ## x: \
        key_xkb = XKB_KEY_ ## x; \
        break;
      SDL_XKB(a)
      SDL_XKB(b)
      SDL_XKB(c)
      SDL_XKB(d)
      SDL_XKB(e)
      SDL_XKB(f)
      SDL_XKB(g)
      /* FIXME: CTRL+h does not work for some reason. */
      SDL_XKB(h)
      SDL_XKB(i)
      SDL_XKB(j)
      SDL_XKB(k)
      SDL_XKB(l)
      SDL_XKB(m)
      SDL_XKB(n)
      SDL_XKB(o)
      SDL_XKB(p)
      SDL_XKB(q)
      /* FIXME: For some reason, CTRL+r does not activate bash
       * reverse-i-search. */
      SDL_XKB(r)
      SDL_XKB(s)
      SDL_XKB(t)
      SDL_XKB(u)
      SDL_XKB(v)
      SDL_XKB(w)
      SDL_XKB(x)
      SDL_XKB(y)
      SDL_XKB(z)
    }
  } else {
    /* Convert miscellaneous SDL keys to XKB keys */
    switch (keycode) {
#define SDLK2XKB(sdl,xkb) \
      case SDLK_ ## sdl: \
        key_xkb = XKB_KEY_ ## xkb; \
        break;
      SDLK2XKB(BACKSPACE, BackSpace)
      SDLK2XKB(DELETE, Delete)
      SDLK2XKB(DOWN, Down)
      SDLK2XKB(END, End)
      SDLK2XKB(ESCAPE, Escape)
      SDLK2XKB(F1, F1)
      SDLK2XKB(F2, F2)
      SDLK2XKB(F3, F3)
      SDLK2XKB(F4, F4)
      SDLK2XKB(F5, F5)
      SDLK2XKB(F6, F6)
      SDLK2XKB(F7, F7)
      SDLK2XKB(F8, F8)
      SDLK2XKB(F9, F9)
      SDLK2XKB(F10, F10)
      SDLK2XKB(F11, F11)
      SDLK2XKB(F12, F12)
      SDLK2XKB(F13, F13)
      SDLK2XKB(F14, F14)
      SDLK2XKB(F15, F15)
      SDLK2XKB(F16, F16)
      SDLK2XKB(F17, F17)
      SDLK2XKB(F18, F18)
      SDLK2XKB(F19, F19)
      SDLK2XKB(F20, F20)
      /* FIXME: For some reason, the home key does not work. */
      SDLK2XKB(HOME, Home)
      SDLK2XKB(INSERT, Insert)
      SDLK2XKB(KP_BACKSPACE, KP_Delete)
      SDLK2XKB(KP_ENTER, KP_Enter)
      SDLK2XKB(LEFT, Left)
      SDLK2XKB(PAGEDOWN, Page_Down);
      SDLK2XKB(PAGEUP, Page_Up)
      SDLK2XKB(PAUSE, Pause)
      SDLK2XKB(RETURN, Return)
      SDLK2XKB(RIGHT, Right)
      SDLK2XKB(SCROLLLOCK, Scroll_Lock)
      SDLK2XKB(SYSREQ, Sys_Req)
      /* FIXME: Somehow tab completion still does not always work with bash */
      /* FIXME: I'm not sure if XKB_KEY_Tab or XKB_KEY_ISO_Left_Tab should be
       * used here. libtsm will send "\x09" for the former and "\e[Z" for the
       * latter. */
      SDLK2XKB(TAB, Tab)
      SDLK2XKB(UP, Up)
      default:
        /* Nothing to handle; most input will be handled through the SDL text
         * input event */
        return;
    }
  }

  /* Send the key to the vte state machine */
  result = tsm_vte_handle_keyboard(
      self->vte,  /* vte */
      key_xkb,  /* keysym */
      XKB_KEY_NoSymbol,  /* ascii */
      modifiers_tsm,  /* mods */
      0  /* unicode */
      );
  if (result) {
    /* FIXME: It's not clear what result signifies or what
     * tsm_screen_sb_reset() actually does. */
    tsm_screen_sb_reset(self->screen);
  }
}

st_ErrorCode
st_Terminal_increaseFontSize(
    st_Terminal *self)
{
#define MAX_INCREASE_FONT_SIZE_ATTEMPTS 32
  float fontSize;  /* Font size in pixels */
  st_ErrorCode error;

  /* Look for the next largest available font for this face */
  fontSize = ceil(self->internal->profile->fontSize) + 1.0f;
  for (int i = 0; i < MAX_INCREASE_FONT_SIZE_ATTEMPTS; ++i) {
    error = st_Profile_setFont(self->internal->profile,
        self->internal->profile->fontFace,
        fontSize);
    if (error == ST_NO_ERROR)
      break;

    fontSize += 1.0f;
  }

  if (error != ST_NO_ERROR) {
    /* We were unable to find the font in a larger size */
    return error;
  }

  /* Load the new font */
  error = st_GlyphRenderer_loadFont(&self->internal->glyphRenderer,
      self->internal->profile->fontPath,
      self->internal->profile->fontSize);

  return error;
}
