#include <assert.h>

#include "terminal.h"

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
  /* TODO: something */
}

void st_Terminal_initWindow(st_Terminal *self) {
  self->window = SDL_CreateWindow(
      "Shelltoy",  /* title */
      SDL_WINDOWPOS_UNDEFINED,  /* x */
      SDL_WINDOWPOS_UNDEFINED,  /* y */
      640,  /* w */
      480,  /* w */
      SDL_WINDOW_OPENGL  /* flags */
      );
  if (self->window == NULL) {
    fprintf(stderr, "Failed to create SDL window: %s\n",
        SDL_GetError());
    /* TODO: Fail gracefully */
    assert(0);
  }
}

void st_Terminal_initTSM(st_Terminal *self) {
  /* TODO: Initialize the screen and state machine provided by libtsm */
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

void st_Terminal_init(st_Terminal *self) {
  /* Initialize the SDL window */
  st_Terminal_initWindow(self);
  /* Initialize the terminal state machine */
  st_Terminal_initTSM(self);
  /* Initialize the pseudo terminal and corresponding child process */
  st_PTY_init(&self->pty);
  /* TODO: Start sending input from the child process to tsm_vte_input()? */
  /* TODO: Start sending keyboard input to tsm_vte_handle_keyboard()? */
}

void st_Terminal_destroy(st_Terminal *self) {
  /* FIXME: Destroy the libtsm state machine */
  /* FIXME: Destroy the libtsm screen */
  st_PTY_destroy(&self->pty);
}
