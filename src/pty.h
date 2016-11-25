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

#ifndef SHELLTOY_PTY_H_
#define SHELLTOY_PTY_H_

#include <pthread.h>
#include <sys/types.h>

typedef void (*st_PTY_readCallback_t)(
    void *data,
    char *buff,
    size_t buff_size);

typedef struct {
  /* TODO: Move some of these to private data structures */
  pid_t child;
  int master_fd, epoll_fd;
  pthread_t poll_thread;
  st_PTY_readCallback_t callback;
  void *callback_data;
  int width, height;
} st_PTY;

/* TODO: We need a method to execute a child process? */
/* TODO: We need a method in which to pass a callback for reading data from the
 * child process */
/* TODO: Use epoll API to watch the pseudo terminal master file handle for
 * changes
 * NOTE: wlterm uses the edge-triggered epoll API (I suppose for better
 * responsiveness)
 */
int st_PTY_eventType();

void st_PTY_init(
    st_PTY *self,
    int width,
    int height);
void st_PTY_destroy(st_PTY *self);

void st_PTY_startChild(
    st_PTY *self,
    const char *path,
    char *const argv[],
    st_PTY_readCallback_t callback,
    void *callback_data);

void st_PTY_read(st_PTY *self);
void st_PTY_write(st_PTY *self, const char *u8, size_t len);

void st_PTY_resize(st_PTY *self, int width, int height);

#endif
