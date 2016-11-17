#define _XOPEN_SOURCE 600

#include <SDL.h>
#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "pty.h"

int st_PTY_eventType() {
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

void st_PTY_pushEvent(st_PTY *self) {
  SDL_Event event;
  memset(&event, 0, sizeof(event));
  event.type = st_PTY_eventType();
  event.user.data1 = self;
  if (SDL_PushEvent(&event) < 0) {
    fprintf(stderr, "Failed to push PTY event to SDL\n");
    /* TODO: Fail gracefully */
    assert(0);
  }
  fprintf(stderr, "Pushed PTY event to SDL\n");  /* XXX */
}

/* TODO: Have st_PTY_watchPTY() opperate in a singleton capacity, since epoll
 * is very good at servicing multiple file descriptors */
void *st_PTY_watchPTY(st_PTY *self) {
  struct epoll_event ev;
  /* Thread routine for watching for input from the pseudo terminal master */
  fprintf(stderr, "Output from st_PTY_watchPTY()\n");

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

    fprintf(stderr, "Got an epoll event in st_PTY_watchPTY()\n");

    /* Notify the main thread with an SDL event */
    st_PTY_pushEvent(self);
  }

  return NULL;
}

void st_PTY_openPTY(st_PTY *self) {
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
      (void *(*)(void*))st_PTY_watchPTY,  /* start_routine */
      self  /* arg */
      );
  if (result != 0) {
    perror("pthread_create");
    fprintf(stderr, "Failed to create pty watch thread\n");
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
    void *callback_data,
    int width,
    int height)
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
    st_PTY_prepareChild(self);
    /* Execute the shell (should not return) */
    execv(path, argv);
    perror("execv");
    fprintf(stderr, "Failed to execute shell: %s\n",
        path);
    /* TODO: Fail gracefully? */
    exit(EXIT_FAILURE);
  }
  self->child = result;
}

void st_PTY_read(st_PTY *self) {
  ssize_t len;
#define PTY_BUFF_SIZE 4096
  char buff[PTY_BUFF_SIZE];
  /* FIXME: Not to sure how to do this properly */
  /* FIXME: This needs an arbitrary limit (as with wlterm) */
  do {
    /* Read from the pseudo terminal master file descriptor and send
     * everything to the libtsm state machine through our callback */
    len = read(self->master_fd, buff, sizeof(buff));
    if (len > 0) {
      self->callback(self->callback_data, buff, len);
      buff[len] = '\0';  /* XXX */
      fprintf(stderr, "%s", buff);  /* XXX: hack */
    }
  } while (len > 0);
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

  /* Store the new width and height */
  self-> width = width;
  self-> height = height;
}
