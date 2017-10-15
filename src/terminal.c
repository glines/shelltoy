/*
 * Copyright (c) 2016-2017 Jonathan Glines
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

#include <SDL_syswm.h>
#include <X11/Xlib.h>
#include <assert.h>

#include "../extern/xkbcommon-keysyms.h"

#include "backgroundRenderer.h"
#include "common/glError.h"
#include "glyphRendererRef.h"
#include "logging.h"
#include "profile.h"
#include "textRenderer.h"

#include "terminal.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))

typedef enum ttoy_Terminal_SelectionState_ {
  TTOY_TERMINAL_NO_SELECTION,
  TTOY_TERMINAL_SELECTION_BETWEEN_CELLS,
  TTOY_TERMINAL_SELECTION_STARTED,
} ttoy_Terminal_SelectionState;

/* Internal data structure */
struct ttoy_Terminal_Internal {
  ttoy_TextRenderer textRenderer;
  ttoy_BackgroundRenderer backgroundRenderer;
  ttoy_GlyphRendererRef *glyphRenderer;
  ttoy_Profile *profile;
  int beginSelection[2];
  int selectionTargetCell[2];
  ttoy_Terminal_SelectionState selectionState;
};

/* Private methods */
void ttoy_Terminal_tsmLogCallback(
    ttoy_Terminal *self,
    const char *file,
    int line,
    const char *func,
    const char *subs,
    unsigned int sev,
    const char *format,
    va_list args);
void ttoy_Terminal_tsmWriteCallback(
    struct tsm_vte *vte,
    const char *u8,
    size_t len,
    ttoy_Terminal *self);
void ttoy_Terminal_initWindow(ttoy_Terminal *self);
void ttoy_Terminal_initTSM(ttoy_Terminal *self);
void ttoy_Terminal_updateScreenSize(ttoy_Terminal *self);
void ttoy_Terminal_calculateScreenSize(
    ttoy_Terminal *self,
    int *columns,
    int *rows);
void ttoy_Terminal_calculateWindowSize(
    ttoy_Terminal *self,
    int *width,
    int *height);
void ttoy_Terminal_calculateCell(
    const ttoy_Terminal *self,
    int x,
    int y,
    int *cellColumn,
    int *cellRow);

void ttoy_Terminal_tsmLogCallback(
    ttoy_Terminal *self,
    const char *file,
    int line,
    const char *func,
    const char *subs,
    unsigned int sev,
    const char *format,
    va_list args)
{
  /* TODO: Do something with this data */
  fprintf(stderr, "ttoy_Terminal_tsmLogCallback() called\n");
}

void ttoy_Terminal_tsmWriteCallback(
    struct tsm_vte *vte,
    const char *u8,
    size_t len,
    ttoy_Terminal *self)
{
  /* Write to the pseudo terminal */
  ttoy_PTY_write(&self->pty, u8, len);
}

void ttoy_Terminal_ptyReadCallback(
    ttoy_Terminal *self,
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
  ttoy_Terminal_updateScreen(self);
}

void ttoy_Terminal_initWindow(ttoy_Terminal *self) {
  /* Calculate the window size based on the desired screen size */
  ttoy_Terminal_calculateWindowSize(self,
      &self->width,  /* columns */
      &self->height  /* rows */
      );
  /* Create the SDL window */
  self->window = SDL_CreateWindow(
      "TToY Graphical Terminal Emulator",  /* title */
      SDL_WINDOWPOS_UNDEFINED,  /* x */
      SDL_WINDOWPOS_UNDEFINED,  /* y */
      self->width,  /* w */
      self->height,  /* h */
      SDL_WINDOW_OPENGL
      | SDL_WINDOW_RESIZABLE  /* flags */
      );
  if (self->window == NULL) {
    fprintf(stderr, "Failed to create SDL window: %s\n",
        SDL_GetError());
    /* TODO: Fail gracefully */
    assert(0);
  }

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
  /* FIXME: This is a hack for an unfortunate bug in GLEW */
  while (glGetError() != GL_NO_ERROR);

  /* Configure the GL */
  ttoy_Color *bgColor =
    &self->internal->profile->colorScheme.colors[TTOY_COLOR_BACKGROUND];
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

void ttoy_Terminal_initTSM(ttoy_Terminal *self) {
  int result;
  /* Initialize the screen and state machine provided by libtsm */
  tsm_screen_new(
      &self->screen,  /* out */
      (tsm_log_t)ttoy_Terminal_tsmLogCallback,  /* log */
      self  /* log_data */
      );
  tsm_vte_new(
      &self->vte,  /* out */
      self->screen,  /* con */
      (tsm_vte_write_cb)ttoy_Terminal_tsmWriteCallback,  /* write_cb */
      self,  /* data */
      (tsm_log_t)ttoy_Terminal_tsmLogCallback,  /* log */
      self  /* log_data */
      );
  /* Set the screen size */
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
}

void ttoy_Terminal_calculateScreenSize(
    ttoy_Terminal *self,
    int *columns,
    int *rows)
{
  /* The screen size is the number of (halfwidth) glyph cells that will fit in
   * the terminal window */
  *columns = self->width / self->cellWidth;
  *rows = self->height / self->cellHeight;
  /* At least one character must fit on the screen */
  *columns = MAX(*columns, 1);
  *rows = MAX(*rows, 1);
}

void ttoy_Terminal_calculateWindowSize(
    ttoy_Terminal *self,
    int *width,
    int *height)
{
  /* The window size is a multiple of the (halfwidth) glyph cell size */
  *width = self->columns * self->cellWidth;
  *height = self->rows * self->cellHeight;
}

void ttoy_Terminal_calculateCell(
    const ttoy_Terminal *self,
    int x,
    int y,
    int *cellColumn,
    int *cellRow)
{
  /* Calculate the position of the cell containing the given screen position */
  *cellColumn = x / self->cellWidth;
  *cellRow = y / self->cellHeight;
}

void ttoy_Terminal_init(
    ttoy_Terminal *self,
    ttoy_Profile *profile,
    int argc,
    char **argv)
{
  /* Allocate memory for internal data structures */
  self->internal = (struct ttoy_Terminal_Internal *)malloc(
      sizeof(struct ttoy_Terminal_Internal));
  self->internal->selectionState = TTOY_TERMINAL_NO_SELECTION;
  self->internal->profile = profile;
  /* TODO: The default columns and rows should be configurable */
  self->columns = 80;
  self->rows = 25;
  /* Initialize the glyph renderer */
  ttoy_GlyphRendererRef_init(&self->internal->glyphRenderer);
  ttoy_GlyphRenderer_init(
      ttoy_GlyphRendererRef_get(self->internal->glyphRenderer),
      profile  /* profile */
      );
  /* Store the cell size as calculated by the glyph renderer */
  ttoy_GlyphRenderer_getCellSize(
      ttoy_GlyphRendererRef_get(self->internal->glyphRenderer),
      &self->cellWidth,  /* width */
      &self->cellHeight  /* height */
      );
  /* Initialize the SDL window */
  ttoy_Terminal_initWindow(self);
  /* Initialize the text renderer */
  ttoy_TextRenderer_init(&self->internal->textRenderer,
      self->internal->glyphRenderer,  /* glyphRenderer */
      profile  /* profile */
      );
  /* Initialize the background renderer */
  ttoy_BackgroundRenderer_init(
      &self->internal->backgroundRenderer,
      profile  /* profile */
      );
  /* Initialize the terminal state machine */
  ttoy_Terminal_initTSM(self);
  /* Initialize the pseudo terminal and corresponding child process */
  ttoy_PTY_init(&self->pty,
      self->columns,  /* width */
      self->rows  /* height */
      );
  /* TODO: Construct a ttoy_MonospaceFont object that combines multiple font
   * faces into one font that supports normal, bold, and wide glyphs */
  /* TODO: Calculate terminal width and height */
  ttoy_PTY_startChild(&self->pty,
      argv[0],  /* path */
      argv,  /* argv */
      (ttoy_PTY_readCallback_t)ttoy_Terminal_ptyReadCallback,  /* callback */
      self  /* callback_data */
      );
  /* TODO: Start sending input from the child process to tsm_vte_input()? */
  /* TODO: Start sending keyboard input to tsm_vte_handle_keyboard()? */
}

void ttoy_Terminal_destroy(ttoy_Terminal *self) {
  /* Destroy all of the objects that we initialized */
  ttoy_BackgroundRenderer_destroy(&self->internal->backgroundRenderer);
  ttoy_TextRenderer_destroy(&self->internal->textRenderer);
  ttoy_GlyphRendererRef_decrement(self->internal->glyphRenderer);
  /* FIXME: Destroy the libtsm state machine */
  /* FIXME: Destroy the libtsm screen */
  ttoy_PTY_destroy(&self->pty);
  /* Release memory for internal data structures */
  free(self->internal);
  /* FIXME: Close the SDL window */
}

void ttoy_Terminal_windowSizeChanged(
    ttoy_Terminal *self,
    int width,
    int height)
{
  /* Store the new window width and height */
  self->width = width;
  self->height = height;
  /* Update the screen size based on the new width and height */
  ttoy_Terminal_updateScreenSize(self);
  /* Update the GL viewport size */
  glViewport(0, 0, width, height);
  FORCE_ASSERT_GL_ERROR();
}

void ttoy_Terminal_updateScreenSize(ttoy_Terminal *self) {
  int newColumns, newRows;
  int result;
  /* Calculate new pseudo terminal size */
  ttoy_Terminal_calculateScreenSize(self,
      &newColumns,  /* columns */
      &newRows  /* rows */
      );
  if ((newColumns != self->columns) || (newRows != self->rows))
  {
    self->columns = newColumns;
    self->rows = newRows;
    /* Change the pseudo terminal screen size */
    result = tsm_screen_resize(
        self->screen,  /* con */
        self->columns,  /* x */
        self->rows  /* y */
        );
    if (result < 0) {
      fprintf(stderr, "Failed to resize libtsm screen\n");
      /* TODO: Fail gracefully */
      /* NOTE: This might fail if libtsm cannot allocate memory for the
       * screen */
      assert(0);
    }
    ttoy_PTY_resize(&self->pty,
        self->columns,  /* width */
        self->rows  /* height */
        );
    /* Update the screen */
    ttoy_TextRenderer_updateScreen(&self->internal->textRenderer,
        self->screen,  /* screen */
        self->cellWidth,  /* cellWidth */
        self->cellHeight  /* cellHeight */
        );
  }
}

void ttoy_Terminal_updateScreen(ttoy_Terminal *self) {
  ttoy_TextRenderer_updateScreen(&self->internal->textRenderer,
      self->screen,  /* screen */
      self->cellWidth,  /* cellWidth */
      self->cellHeight  /* cellHeight */
      );
}

void ttoy_Terminal_draw(ttoy_Terminal *self) {
  /* Clear the screen */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  FORCE_ASSERT_GL_ERROR();

  /* Draw the background */
  ttoy_BackgroundRenderer_draw(&self->internal->backgroundRenderer,
      self->width,  /* viewportWidth */
      self->height  /* viewportHeight */
      );

  /* Draw the glyphs on the screen */
  ttoy_TextRenderer_draw(&self->internal->textRenderer,
      self->cellWidth,  /* cellWidth */
      self->cellHeight,  /* cellHeight */
      self->width,  /* viewportWidth */
      self->height  /* viewportHeight */
      );

  SDL_GL_SwapWindow(self->window);
}

void ttoy_Terminal_textInput(
    ttoy_Terminal *self,
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

void ttoy_Terminal_keyInput(
    ttoy_Terminal *self,
    SDL_Keycode keycode,
    uint16_t modifiers)
{
  unsigned int modifiers_tsm;
  uint32_t key_xkb;
  int result;

  /* Don't act on alt+tab events */
  if ((modifiers & KMOD_ALT)
      && (keycode == SDLK_TAB))
    return;

  /* Convert the SDL modifier flags to libtsm modifier flags */
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
      case SDLK_EQUALS:  /* plus */
        /* Shortcut for increasing the font size */
        /* FIXME: Somehow this code does not swallow the = sign as we would
         * like, i.e. = is still sent to the pty. */
        /* TODO: Allow for increase font size shortcut to be configured */
        ttoy_Terminal_increaseFontSize(self);
        return;
      case SDLK_MINUS:
        /* Shortcut for decreasing the font size */
        /* FIXME: Somehow this code does not swallow the - sign as we would
         * like, i.e. - is still sent to the pty. */
        /* TODO: Allow for decrease font size shortcut to be configured */
        ttoy_Terminal_decreaseFontSize(self);
        return;
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

void ttoy_Terminal_mouseButton(
    ttoy_Terminal *self,
    const SDL_MouseButtonEvent *event)
{
  switch (event->button) {
    case SDL_BUTTON_LEFT:
      if (event->state == SDL_PRESSED) {
        if (event->clicks == 1) {
          /* Clear the old selection */
          tsm_screen_selection_reset(self->screen);
          /* Update the screen */
          ttoy_TextRenderer_updateScreen(&self->internal->textRenderer,
              self->screen,  /* screen */
              self->cellWidth,  /* cellWidth */
              self->cellHeight  /* cellHeight */
              );
          /* Begin the selection (no cells are selected until dragging starts) */
          self->internal->beginSelection[0] = event->x;
          self->internal->beginSelection[1] = event->y;
          self->internal->selectionState = TTOY_TERMINAL_SELECTION_BETWEEN_CELLS;
        } else if (event->clicks == 2) {
          /* TODO: Select the word under the cursor */
          /* TODO: Begin word selection mode */
        } else if (event->clicks == 3) {
          /* TODO: Select the line under the cursor */
          /* TODO: Begin line selection mode */
        }
      } else {
        if (self->internal->selectionState == TTOY_TERMINAL_SELECTION_STARTED) {
          char *selection;
          int len;
          assert(event->state == SDL_RELEASED);
          /* TODO: If there was a selection, then copy it to the clipboard */
          /* FIXME: The screen selection implementation in libtsm is supposedly
           * just a "proof-of-concept" according to the comments within the
           * libtsm source code. We might want to improve upon it. */
          /* FIXME: The screen selection implementation in libtsm will copy a
           * large amount of trailing whitespace that we do not want in our
           * clipboard. */
          len = tsm_screen_selection_copy(
              self->screen,  /* conn */
              &selection  /* out */
              );
          assert(len > 0);
          /* Replace the '\0' null values within the selection string with
           * spaces. I am not entirely sure why there are null values. */
          for (size_t i = 0; i < len - 1; ++i) {
            if (selection[i] == '\0') {
              selection[i] = ' ';
            }
          }
          assert(selection[len] == '\0');
          /* NOTE: SDL sets both the X11 primary selection and clipboard
           * selection. It would be ideal to only set the primary selection in
           * this routine. */
          /* TODO: Avoid setting the clipboard selection for X11 here */
          SDL_SetClipboardText(selection);
          free(selection);  /* NOTE: libtsm has malloc'ed the selection */
        }
      }
      break;
    case SDL_BUTTON_MIDDLE:
      if (event->state == SDL_PRESSED && SDL_HasClipboardText())
      {
        char *clipboardText;
        /* Retrieve the clipboard text */
        /* FIXME: SDL only supports the clipboard selection. We should really be
         * using the primary selection with X11. */
        clipboardText = SDL_GetClipboardText();
        if (clipboardText == NULL) {
          TTOY_LOG_ERROR_CODE(TTOY_ERROR_OUT_OF_MEMORY);
          break;
        }
        /* Paste the clipboard text to the terminal */
        ttoy_Terminal_textInput(self,
            clipboardText  /* text */
            );
        SDL_free(clipboardText);
      }
      break;
  }
}

void ttoy_Terminal_mouseMotion(
    ttoy_Terminal *self,
    const SDL_MouseMotionEvent *event)
{
  if (event->state & SDL_BUTTON_LMASK) {
    /* TODO: Support selection dragging even when the cursor is dragged outside
     * of the window */
    int beginSelection[2], endSelection[2], beginCell[2], endCell[2],
        beginCellHalf, endCellHalf, beginPos, endPos;
    beginSelection[0] = self->internal->beginSelection[0];
    beginSelection[1] = self->internal->beginSelection[1];
    endSelection[0] = event->x;
    endSelection[1] = event->y;
    /* Determine the cells in which the selection begins and ends */
    ttoy_Terminal_calculateCell(self,
        beginSelection[0],  /* x */
        beginSelection[1],  /* y */
        &beginCell[0],  /* cellColumn */
        &beginCell[1]  /* cellRow */
        );
    ttoy_Terminal_calculateCell(self,
        endSelection[0],  /* x */
        endSelection[1],  /* y */
        &endCell[0],  /* cellColumn */
        &endCell[1]  /* cellRow */
        );
    /* Determine which half of each cell the selection begins at and ends
     * at respectively */
    beginCellHalf = ((2 * beginSelection[0]) / self->cellWidth) % 2;
    endCellHalf = ((2 * endSelection[0]) / self->cellWidth) % 2;
    beginPos = (beginCell[0] << 1) + beginCellHalf;
    endPos = (endCell[0] << 1) + endCellHalf;
    switch (self->internal->selectionState) {
      case TTOY_TERMINAL_NO_SELECTION:
      case TTOY_TERMINAL_SELECTION_BETWEEN_CELLS:
        if (beginCell[1] == endCell[1]) {
#define ROUND_CELL(a) (((a) >> 1) + ((a) & 1))
          /* The selection is within a single row */
          if (ROUND_CELL(beginPos) == ROUND_CELL(endPos)) {
            /* The selection is between cells; nothing is selected */
            break;
          }
          if (beginPos < endPos) {
            /* Selection from left to right */
            tsm_screen_selection_start(
                self->screen,  /* con */
                ROUND_CELL(beginPos),  /* posx */
                beginCell[1]  /* posy */
                );
          } else if (endPos < beginPos) {
            /* Selection from right to left */
            tsm_screen_selection_start(
                self->screen,  /* con */
                ROUND_CELL(beginPos) - 1,  /* posx */
                beginCell[1]  /* posy */
                );
          }
        } else if (beginCell[1] < endCell[1]) {
          /* Selection from the top down */
          /* FIXME: Check for selecting cell at a column past the number of
           * columns on the screen, which is possible here */
          tsm_screen_selection_start(
              self->screen,  /* con */
              ROUND_CELL(beginPos),  /* posx */
              beginCell[1]  /* posy */
              );
        } else {
          assert(endCell[1] < beginCell[1]);
          /* Selection from the bottom up */
          /* Note that we are careful to avoid selecting a cell at column -1 */
          tsm_screen_selection_start(
              self->screen,  /* con */
              ROUND_CELL(beginPos) - 1 < 0 ?
                self->columns - 1
                : ROUND_CELL(beginPos) - 1,  /* posx */
              ROUND_CELL(beginPos) - 1 < 0 ?
                beginCell[1] - 1 : beginCell[1]  /* posy */
              );
        }
        self->internal->selectionState = TTOY_TERMINAL_SELECTION_STARTED;
      case TTOY_TERMINAL_SELECTION_STARTED:
        {
          int newSelectionTarget[2];
          /* Calculate the new selection target */
          /* FIXME: If the selection transitions from top down to bottom up or
           * vice versa, we must restart the selection with the appropriate
           * starting cell. The current solution where the selection only
           * restarts when the user returns the cursor to the initial cell of
           * the selection is not very robust. */
          if (beginCell[1] == endCell[1]) {
            /* The selection is within a single row */
            if (ROUND_CELL(beginPos) == ROUND_CELL(endPos)) {
              /* The selection is between cells; nothing is selected */
              tsm_screen_selection_reset(self->screen);
              /* Update the screen */
              ttoy_TextRenderer_updateScreen(&self->internal->textRenderer,
                  self->screen,  /* screen */
                  self->cellWidth,  /* cellWidth */
                  self->cellHeight  /* cellHeight */
                  );
              self->internal->selectionState =
                TTOY_TERMINAL_SELECTION_BETWEEN_CELLS;
              break;
            }
            if (beginPos < endPos) {
              /* Selection from left to right */
              newSelectionTarget[0] = ROUND_CELL(endPos) - 1;
              newSelectionTarget[1] = endCell[1];
            } else {
              assert(endPos < beginPos);
              /* Selection from right to left */
              newSelectionTarget[0] = ROUND_CELL(endPos);
              newSelectionTarget[1] = endCell[1];
            }
          } else if (beginCell[1] < endCell[1]) {
            /* Selection from the top down */
            /* Note that we are careful to avoid selecting a cell at column -1 */
            newSelectionTarget[0] =
              ROUND_CELL(endPos) - 1 < 0 ?
                self->columns - 1
                : ROUND_CELL(endPos) - 1;
            newSelectionTarget[1] =
              ROUND_CELL(endPos) - 1 < 0 ?
                endCell[1] - 1 : endCell[1];
          } else {
            assert(endCell[1] < beginCell[1]);
            /* Selection from the bottom up */
            /* FIXME: Check for selecting cell at a column past the number of
             * columns on the screen, which is possible here */
            newSelectionTarget[0] = ROUND_CELL(endPos);
            newSelectionTarget[1] = endCell[1];
          }
          /* Check for changes to the selection target */
          if (newSelectionTarget[0] != self->internal->selectionTargetCell[0]
              || newSelectionTarget[1] != self->internal->selectionTargetCell[1])
          {
            /* Update the selection target */
            tsm_screen_selection_target(
                self->screen,  /* con */
                newSelectionTarget[0],  /* posx */
                newSelectionTarget[1]  /* posy */
                );
            self->internal->selectionTargetCell[0] = newSelectionTarget[0];
            self->internal->selectionTargetCell[1] = newSelectionTarget[1];
            /* Update the screen */
            ttoy_TextRenderer_updateScreen(&self->internal->textRenderer,
                self->screen,  /* screen */
                self->cellWidth,  /* cellWidth */
                self->cellHeight  /* cellHeight */
                );
          }
        }
        break;
      default:
        assert(0);
    }
  }
}

ttoy_ErrorCode
ttoy_Terminal_increaseFontSize(
    ttoy_Terminal *self)
{
#define MAX_INCREASE_FONT_SIZE_ATTEMPTS 32
  float fontSize;  /* Font size in pixels */
  ttoy_GlyphRendererRef *newGlyphRenderer;
  ttoy_ErrorCode error;

  /* Look for the next largest available font */
  fontSize = ttoy_Profile_getFontSize(self->internal->profile);
  fontSize = ceil(fontSize) + 1.0f;
  for (int i = 0; i < MAX_INCREASE_FONT_SIZE_ATTEMPTS; ++i) {
    error = ttoy_Profile_setFontSize(self->internal->profile,
        fontSize);
    if (error == TTOY_NO_ERROR)
      break;

    fontSize += 1.0f;
  }

  if (error != TTOY_NO_ERROR) {
    /* We were unable to find the font in a larger size */
    return error;
  }

  /* Construct a new glyph renderer */
  ttoy_GlyphRendererRef_init(&newGlyphRenderer);
  ttoy_GlyphRenderer_init(
      ttoy_GlyphRendererRef_get(newGlyphRenderer),
      self->internal->profile  /* profile */
      );

  /* TODO: Construct a new glyph renderer and pass it to the text renderer */
  /* NOTE: This will prompt the text renderer to start re-building its glyph
   * atlas. Because the text renderer will need to continue to use the old
   * glyph renderer for a number of frames, it is important that the glyph
   * renderer actually exist as an ttoy_GlyphRendererRef */
  ttoy_TextRenderer_setGlyphRenderer(&self->internal->textRenderer,
      newGlyphRenderer  /* glyphRenderer */
      );

  /* Disown our old glyph renderer */
  ttoy_GlyphRendererRef_decrement(self->internal->glyphRenderer);
  self->internal->glyphRenderer = newGlyphRenderer;

  /* Update the cell size to the new size as calculated by the glyph renderer */
  ttoy_GlyphRenderer_getCellSize(
      ttoy_GlyphRendererRef_get(self->internal->glyphRenderer),
      &self->cellWidth,  /* width */
      &self->cellHeight  /* height */
      );

  /* Update the screen size based on the new cell size */
  ttoy_Terminal_updateScreenSize(self);

  /* TODO: Do we need an error code for not being able to increase the font
   * size? */
  return error;
}

ttoy_ErrorCode
ttoy_Terminal_decreaseFontSize(
    ttoy_Terminal *self)
{
#define MAX_DECREASE_FONT_SIZE_ATTEMPTS 32
  float fontSize;  /* Font size in pixels */
  ttoy_GlyphRendererRef *newGlyphRenderer;
  ttoy_ErrorCode error;

  /* Look for the next largest available font */
  fontSize = ttoy_Profile_getFontSize(self->internal->profile);
  fontSize = floor(fontSize) - 1.0f;
  for (int i = 0; i < MAX_DECREASE_FONT_SIZE_ATTEMPTS; ++i) {
    error = ttoy_Profile_setFontSize(self->internal->profile,
        fontSize);
    if (error == TTOY_NO_ERROR)
      break;

    fontSize -= 1.0f;
  }

  if (error != TTOY_NO_ERROR) {
    /* We were unable to find the font in a larger size */
    return error;
  }

  /* Construct a new glyph renderer */
  ttoy_GlyphRendererRef_init(&newGlyphRenderer);
  ttoy_GlyphRenderer_init(
      ttoy_GlyphRendererRef_get(newGlyphRenderer),
      self->internal->profile  /* profile */
      );

  /* TODO: Construct a new glyph renderer and pass it to the text renderer */
  /* NOTE: This will prompt the text renderer to start re-building its glyph
   * atlas. Because the text renderer will need to continue to use the old
   * glyph renderer for a number of frames, it is important that the glyph
   * renderer actually exist as an ttoy_GlyphRendererRef */
  ttoy_TextRenderer_setGlyphRenderer(&self->internal->textRenderer,
      newGlyphRenderer  /* glyphRenderer */
      );

  /* Disown our old glyph renderer */
  ttoy_GlyphRendererRef_decrement(self->internal->glyphRenderer);
  self->internal->glyphRenderer = newGlyphRenderer;

  /* Update the cell size to the new size as calculated by the glyph renderer */
  ttoy_GlyphRenderer_getCellSize(
      ttoy_GlyphRendererRef_get(self->internal->glyphRenderer),
      &self->cellWidth,  /* width */
      &self->cellHeight  /* height */
      );

  /* Update the screen size based on the new cell size */
  ttoy_Terminal_updateScreenSize(self);

  /* TODO: Do we need an error code for not being able to increase the font
   * size? */
  return error;
}

ttoy_ErrorCode
ttoy_Terminal_setSelectionClipboard(
    ttoy_Terminal *self,
    const char *selection)
{
  /* TODO: The ttoy_Terminal_setSelectionClipboard() routine is written in an
   * attempt to support setting the X11 primary selection without setting the
   * clipboard selection. This might not actually be possible to achieve with
   * SDL, since SDL is ultimately responsible for responding to the window
   * system events that request the contents of the primary and clipboard
   * selections. We might need to patch SDL in order to support this. */
  SDL_SysWMinfo windowInfo;
  SDL_bool sdlResult;
  Window window;
  Display *display;
  Atom primarySelection;
  /* Get the X11 Window associated with our SDL window */
  sdlResult = SDL_GetWindowWMInfo(self->window, &windowInfo);
  if (sdlResult != SDL_TRUE) {
    /* TODO: Get the SDL-specific error message */
    return TTOY_ERROR_SDL_ERROR;
  }
  window = windowInfo.info.x11.window;
  display = windowInfo.info.x11.display;
  /* TODO: Get the X11 atom associated with the selection buffer in our
   * window */
  primarySelection = XInternAtom(display, "PRIMARY", 0);  /* XXX */
  /* TODO: Set the clipboard to our selection buffer */
  if (XGetSelectionOwner(display, primarySelection) != window) {
    XSetSelectionOwner(display, primarySelection, window, CurrentTime);
  }
  return TTOY_NO_ERROR;
}
