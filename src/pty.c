#define _XOPEN_SOURCE 600

#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "pty.h"

/* Private methods */
void st_PTY_startChildProcess(st_PTY *self);
void st_PTY_joinChildProcess(st_PTY *self);

void st_PTY_openPTY(st_PTY *self) {
  /* Open a pseudo terminal */
  self->master_fd = posix_openpt(O_RDWR | O_NOCTTY);
  if (self->master_fd == -1) {
    perror("posix_openpt");
    fprintf(stderr, "Failed to open pseudo terminal\n");
    /* TODO: Fail gracefully */
    assert(0);
  }
}

void st_PTY_joinChildProcess(st_PTY *self) {
  /* TODO: Try killing the child nicely with SIGTERM first */
  kill(self->child, SIGKILL);
  /* TODO: Wait for the child process to exit */
}

void st_PTY_init(st_PTY *self) {
  st_PTY_openPTY(self);
}

void st_PTY_destroy(st_PTY *self) {
  /* TODO: Join the child process? */
  st_PTY_joinChildProcess(self);
  /* TODO: Close the pty? */
}

void st_PTY_prepareChild(st_PTY *self) {
  /* NOTE: This is always called within the child process */
  int slave_fd;
  size_t len;
  char *slave_name, *temp;
  /* Allow the child to access the pseudo terminal */
  if (grantpt(self->master_fd) < 0) {
    perror("grantpt");
    fprintf(stderr, "Failed to grant access to pseudo terminal\n");
    /* TODO: Fail gracefully */
    assert(0);
  }
  if (unlockpt(self->master_fd) < 0) {
    perror("unlockpt");
    fprintf(stderr, "Failed to unlock pseudo terminal\n");
    /* TODO: Fail gracefully */
    assert(0);
  }
  /* Get the file descriptor of the pseudo terminal slave */
  temp = ptsname(self->master_fd);
  if (temp == NULL) {
    perror("ptsname");
    fprintf(stderr, "Failed get pseudo terminal slave name\n");
    /* TODO: Fail gracefully */
    assert(0);
  }
  /* FIXME: Maybe we should have a fixed buffer size */
  len = strlen(temp);
  slave_name = (char*)malloc(len + 1);
  strncpy(slave_name, temp, len);
  slave_name[len] = '\0';
  /* Open the pseudo terminal slave */
  slave_fd = open(slave_name, O_RDWR);
  if (slave_fd < 0) {
    perror("open");
    fprintf(stderr, "Failed to open pseudo terminal slave: %s\n",
        slave_name);
    /* TODO: Fail gracefully */
    assert(0);
  }
  free(slave_name);
  /* Redirect stdin, stdout, and stderr to the pseudo terminal slave */
  /* TODO: Check for errors here */
  dup2(slave_fd, STDIN_FILENO);
  dup2(slave_fd, STDOUT_FILENO);
  dup2(slave_fd, STDERR_FILENO);
}

void st_PTY_startChild(
    st_PTY *self,
    const char *path,
    char *const argv[],
    st_PTY_readCallback_t callback,
    int width,
    int height)
{
  pid_t result;
  /* Fork our child process */
  result = fork();
  if (result < 0) {
    fprintf(stderr, "Failed to fork process.\n");
  } else if (result == 0) {
    /* Inside the child process */
    st_PTY_prepareChild(self);
    /* Execute the shell (should not return) */
    execv(path, argv);
    perror("execv");
    fprintf(stderr, "Failed to execute shell: %s\n",
        SHELL);
    /* TODO: Fail gracefully? */
    exit(EXIT_FAILURE);
  }
  self->child = result;
}

void st_PTY_resize(st_PTY *self, int width, int height) {
  struct winsize ws;
  int result;

  /* Send the new window size to the pseudo terminal slave. See page 718 of
   * Advanced Programming in the UNIX Environment, Third Edition. */
  memset(&ws, 0, sizeof(ws));
  ws.ws_col = width;
  ws.ws_row = height;
  result = ioctl(self->master_fd, TIOCSWINSZ, &ws);
  /* TODO: Check result for errors */
}
