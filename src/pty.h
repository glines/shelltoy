#ifndef SHELLTOY_PTY_H_
#define SHELLTOY_PTY_H_

#include <sys/types.h>

typedef struct {
  /* TODO: Declare handle to child process */
  pid_t child;
  int master_fd;
} st_PTY;

void st_PTY_init(st_PTY *self);
void st_PTY_destroy(st_PTY *self);

void st_PTY_resize(st_PTY *self, int width, int height);

#endif
