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
void st_Terminal_drawCallback(
  struct tsm_screen *con,
  uint32_t id,
  const uint32_t *ch,
  size_t len,
  unsigned int width,
  unsigned int posx,
  unsigned int posy,
  const struct tsm_screen_attr *attr,
  tsm_age_t age,
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

void st_Terminal_drawCallback(
  struct tsm_screen *con,
  uint32_t id,
  const uint32_t *ch,
  size_t len,
  unsigned int width,
  unsigned int posx,
  unsigned int posy,
  const struct tsm_screen_attr *attr,
  tsm_age_t age,
  st_Terminal *self)
{
  /* TODO: Update the OpenGL buffers backing the instance rendered text */
  fprintf(stderr, "Printing character: '%c' at location (%d, %d)\n",
      (char)(*ch),
      posx,
      posy);
}

/* FIXME: I don't think this is needed; read events are sent through SDL */
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
  /* TODO: Update the terminal display */
  st_Terminal_updateDisplay(self);
}

void st_Terminal_initWindow(st_Terminal *self) {
  /* Create the SDL window */
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
  /* Create an OpenGL context for our window */
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

  /* Configure the GL */
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClearDepth(1.0);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glDisable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  /* TODO: Calculate the window width and height */
  /* glViewport(0, 0, width, height); */
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

/* TODO: Allow the user to configure the shell command to invoke */
#define ENV_PATH "/usr/bin/env"
#define SHELL "bash"

void st_Terminal_init(st_Terminal *self) {
  size_t len;
  /* Initialize the SDL window */
  st_Terminal_initWindow(self);
  /* Initialize the render context */
  st_RenderContext_init(&self->renderContext);
  /* Initialize the terminal state machine */
  st_Terminal_initTSM(self);
  /* Initialize the pseudo terminal and corresponding child process */
  st_PTY_init(&self->pty);
  char **argv = (char **)malloc(3 * sizeof(char *));
#define COPY_STRING(dst,src) \
  len = strlen(src); \
  dst = (char*)malloc(len + 1); \
  strncpy(dst, src, len); \
  dst[len] = '\0';
  COPY_STRING(argv[0], ENV_PATH);
  COPY_STRING(argv[1], SHELL);
  argv[2] = NULL;
  /* TODO: calculate terminal width and height */
  st_PTY_startChild(&self->pty,
      "/usr/bin/env",  /* path */
      argv,  /* argv */
      (st_PTY_readCallback_t)st_Terminal_ptyReadCallback,  /* callback */
      self,  /* callback_data */
      80,  /* width */
      24  /* height */
      );
  /* FIXME: Is it safe to free argv at this point? */
  free(argv[0]);
  free(argv[1]);
  free(argv);
  /* TODO: Start sending input from the child process to tsm_vte_input()? */
  /* TODO: Start sending keyboard input to tsm_vte_handle_keyboard()? */
}

void st_Terminal_destroy(st_Terminal *self) {
  /* FIXME: Destroy the libtsm state machine */
  /* FIXME: Destroy the libtsm screen */
  st_PTY_destroy(&self->pty);
}

void st_Terminal_updateDisplay(st_Terminal *self) {
  /* TODO: The update should probably be queued as an event... maybe? */
  /* TODO: Read the character grid from the libtsm screen */
  /* TODO: Make sure we have loaded all of the glyphs that we need */
  /* TODO: Make sure the changes get rendered immediately (might be useful to
   * render even sooner than the toy can render) */
  st_RenderContext_updateTerminalText(&self->renderContext, self->screen);
}
