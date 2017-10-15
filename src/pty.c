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

#define _XOPEN_SOURCE 600

#include <SDL.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "logging.h"
#include "pty.h"

int ttoy_PTY_eventType() {
  static int eventType = -1;

  /* Register the event type with SDL, if we have not done so already */
  if (eventType == -1) {
    /* FIXME: This call to SDL_RegisterEvents() might not be thread safe */
    eventType = SDL_RegisterEvents(1);
    if (eventType == -1) {
      fprintf(stderr, "Failed to register PTY event with SDL\n");
      /* TODO: Fail gracefully */
      assert(0);
    }
  }

  return eventType;
}

void ttoy_PTY_pushEvent(ttoy_PTY *self) {
  SDL_Event event;
  memset(&event, 0, sizeof(event));
  event.type = ttoy_PTY_eventType();
  event.user.data1 = self;
  if (SDL_PushEvent(&event) < 0) {
    fprintf(stderr,
        "Failed to push PTY event to SDL: %s\n",
        SDL_GetError());
    /* TODO: Fail gracefully */
    assert(0);
  }
}

/* TODO: Have ttoy_PTY_watchPTY() opperate in a singleton capacity, since epoll
 * is very good at servicing multiple file descriptors */
void *ttoy_PTY_watchPTY(ttoy_PTY *self) {
  struct epoll_event ev;
  int eventType;
  /* Thread routine for watching for input from the pseudo terminal master */

  eventType = ttoy_PTY_eventType();

  while (1) {
    /* TODO: Check for signal to join main thread */
    /* NOTE: Might want to use epoll_pwait() in conjunction with a signal so
     * that this thread can be more responsive when joining the main thread
     * */

    /* Wait for any changes to the pseudo terminal master */
    epoll_wait(
        self->epoll_fd,  /* epfd */
        &ev,  /* events */
        1,  /* maxevents */
        -1  /* timeout */
        );

    /* Avoid filling the SDL event queue */
    if (SDL_PeepEvents(
          NULL,  /* events */
          1,  /* numevents */
          SDL_PEEKEVENT,  /* action */
          eventType,  /* minType */
          eventType  /* maxType */
          ) >= 1)
    {
      continue;
    }

    /* Notify the main thread with an SDL event */
    ttoy_PTY_pushEvent(self);
  }

  return NULL;
}

void ttoy_PTY_openPTY(ttoy_PTY *self) {
  struct epoll_event ev;
  int result;
  /* Open a pseudo terminal */
  /* TODO: Might want to add O_CLOEXEC later */
  self->master_fd = posix_openpt(O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (self->master_fd == -1) {
    perror("posix_openpt");
    fprintf(stderr, "Failed to open pseudo terminal\n");
    /* TODO: Fail gracefully */
    assert(0);
  }
  /* Set up poll to watch for changes to the pseudo terminal master */
  /* FIXME: Move this epoll stuff to a central location (since we can watch
   * more than one fd at a time) */
  self->epoll_fd = epoll_create1(0);
  if (self->epoll_fd < 0) {
    perror("epoll_create1");
    fprintf(stderr, "Failed to create epoll file handle\n");
    /* TODO: Fail gracefully */
    assert(0);
  }
  memset(&ev, 0, sizeof(ev));
  ev.events =
    EPOLLIN  /* listen for input */
    | EPOLLOUT  /* listen for output */
    | EPOLLET  /* event triggered */
    ;
  ev.data.ptr = self;
  epoll_ctl(
      self->epoll_fd,  /* epfd */
      EPOLL_CTL_ADD,  /* op */
      self->master_fd,  /* fd */
      &ev  /* event */
      );
  /* Open a thread to watch for input from the pseudo terminal master */
  result = pthread_create(
      &self->poll_thread,  /* thread */
      NULL,  /* attr */
      (void *(*)(void*))ttoy_PTY_watchPTY,  /* start_routine */
      self  /* arg */
      );
  if (result != 0) {
    perror("pthread_create");
    fprintf(stderr, "Failed to create pty watch thread\n");
    /* TODO: Fail gracefully */
    assert(0);
  }
}

void ttoy_PTY_joinChildProcess(ttoy_PTY *self) {
  /* TODO: Try killing the child nicely with SIGTERM first */
  kill(self->child, SIGKILL);
  /* TODO: Wait for the child process to exit */
}

void ttoy_PTY_init(
    ttoy_PTY *self,
    int width,
    int height)
{
  self->width = width;
  self->height = height;
  self->master_fd = -1;
  ttoy_PTY_openPTY(self);
  ttoy_PTY_resize(self, width, height);
}

void ttoy_PTY_destroy(ttoy_PTY *self) {
  /* TODO: Join the child process? */
  ttoy_PTY_joinChildProcess(self);
  /* TODO: Close the pty? */
}

void ttoy_PTY_prepareChild(ttoy_PTY *self) {
  /* NOTE: This is always called within the child process */
  int slave_fd;
  size_t len;
  char *slave_name, *temp;
  int result;
  struct termios attr;
  int i;
  pid_t pid;
  /* Set TERM environment variable to xterm-256color, since libtsm approximates
   * the functionality of xterm */
  setenv("TERM", "xterm-256color", 1);
  /* Disable the SIGCHLD signal handler before calling grantpt(3) */
  /* FIXME: I'm not sure if this signal handler business is necessary */
  sigset_t sigset;
  sigemptyset(&sigset);
  result = sigprocmask(SIG_SETMASK, &sigset, NULL);
  /* TODO: Fail gracefully */
  if (result < 0) assert(0);
  for (i = 1; i < SIGUNUSED; ++i)
    signal(i, SIG_DFL);
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
  slave_fd = open(slave_name, O_RDWR | O_NOCTTY);
  if (slave_fd < 0) {
    perror("open");
    fprintf(stderr, "Failed to open pseudo terminal slave: %s\n",
        slave_name);
    /* TODO: Fail gracefully */
    assert(0);
  }
  free(slave_name);
  /* Create new session and relinquish the controlling terminal */
  pid = setsid();
  if (pid < 0) {
    perror("setsid");
    fprintf(stderr, "Failed to create child process group");
    /* TODO: Fail gracefully */
    assert(0);
  }
  /* Set the controlling terminal */
  result = ioctl(slave_fd, TIOCSCTTY, NULL);
  if (result < 0) {
    perror("ioctl");
    fprintf(stderr, "Failed to set controlling terminal of child");
    /* TODO: Fail gracefully */
    assert(0);
  }
  /* Get the terminal attributes */
  if (tcgetattr(slave_fd, &attr) < 0) {
    fprintf(stderr, "Failed to get pseudo terminal attributes");
    /* TODO: Fail gracefully */
    assert(0);
  }
  /* TODO: Are there any attributes that we want to set here? */
  /* Set the terminal attributes */
  if (tcsetattr(slave_fd, TCSANOW, &attr) < 0) {
    fprintf(stderr, "Failed to set pseudo terminal attributes");
    /* TODO: Fail gracefully */
    assert(0);
  }
  /* Redirect stdin, stdout, and stderr to the pseudo terminal slave */
  /* TODO: Check for errors here */
  dup2(slave_fd, STDIN_FILENO);
  dup2(slave_fd, STDOUT_FILENO);
  dup2(slave_fd, STDERR_FILENO);
}

void ttoy_PTY_startChild(
    ttoy_PTY *self,
    const char *path,
    char *const argv[],
    ttoy_PTY_readCallback_t callback,
    void *callback_data)
{
  pid_t result;
  /* Register the read callback */
  self->callback = callback;
  self->callback_data = callback_data;
  /* Fork our child process */
  result = fork();
  if (result < 0) {
    fprintf(stderr, "Failed to fork process.\n");
  } else if (result == 0) {
    /* Inside the child process */
    ttoy_PTY_prepareChild(self);
    /* Execute the shell (should not return) */
    execv(path, argv);
    perror("execv");
    fprintf(stderr, "Failed to execute shell: %s\n",
        path);
    /* Exit the child without calling routines registered with atexit(3) */
    quick_exit(EXIT_FAILURE);
  }
  /* FIXME: wlterm will wait for the child setup here, and subsequently set up
   * epoll. Maybe we need to do something like that. */
  self->child = result;
}

int ttoy_PTY_read(ttoy_PTY *self) {
  ssize_t len;
#define TTOY_PTY_BUFF_SIZE 4096
#define TTOY_PTY_MAX_READ 32
  char buff[TTOY_PTY_BUFF_SIZE];
  int count;
  /* Loop to read data sent from the pseudo terminal */
  /* This read loop is limited (as with wlterm) so that we do not block the
   * main thread for too long. When the read limit is reached, we return
   * EWOULDBLOCK to indicate to the caller that there is still more data to
   * read. */
  count = 0;
  do {
    /* Read from the pseudo terminal master file descriptor and send
     * everything to the libtsm state machine through our callback */
    len = read(self->master_fd, buff, sizeof(buff));
    if (len > 0) {
      self->callback(self->callback_data, buff, len);
    }
  } while (len > 0 && count < TTOY_PTY_MAX_READ);

  return count < TTOY_PTY_MAX_READ ? 0 : EWOULDBLOCK;
}

void ttoy_PTY_write(ttoy_PTY *self, const char *u8, size_t len) {
  ssize_t bytes_written;
  /* TODO: wlterm uses some crazy ring buffer and writev(). We should look into
   * why they do that. */
  /* TODO: Check for errors? */
  bytes_written = write(self->master_fd, u8, len);
  if (bytes_written < 0) {
    fprintf(stderr, "Error writting to pseudo terminal: %s\n",
        strerror(errno));
  }
}

void ttoy_PTY_resize(ttoy_PTY *self, int width, int height) {
  struct winsize ws;
  int result;

  assert(self->master_fd >= 0);

  /* Send the new window size to the pseudo terminal slave. See page 718 of
   * Advanced Programming in the UNIX Environment, Third Edition. */
  memset(&ws, 0, sizeof(ws));
  ws.ws_col = width;
  ws.ws_row = height;
  result = ioctl(self->master_fd, TIOCSWINSZ, &ws);
  if (result != 0) {
    TTOY_LOG_ERROR("Failed to resize pseudo terminal: %s",
        strerror(errno));
  }

  /* Store the new width and height */
  self-> width = width;
  self-> height = height;
}
