#ifndef SHELLTOY_PTY_H_
#define SHELLTOY_PTY_H_

#include <sys/types.h>

typedef struct {
  /* TODO: Declare handle to child process */
  pid_t child;
  int master_fd;
} st_PTY;

typedef void (*st_PTY_readCallback_t)(
    st_PTY *pty,
    char *buff,
    size_t buff_size,
    void *data);

/* TODO: We need a method to execute a child process? */
/* TODO: We need a method in which to pass a callback for reading data from the
 * child process */
/* TODO: Use epoll API to watch the pseudo terminal master file handle for
 * changes
 * NOTE: wlterm uses the edge-triggered epoll API (I suppose for better
 * responsiveness)
 */
void st_PTY_init(st_PTY *self);
void st_PTY_destroy(st_PTY *self);

void st_PTY_startChild(
    st_PTY *self,
    const char *path,
    char *const argv[],
    st_PTY_readCallback_t callback,
    int width,
    int height);

void st_PTY_resize(st_PTY *self, int width, int height);

#endif
