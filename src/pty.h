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

#ifndef TTOY_PTY_H_
#define TTOY_PTY_H_

#include <pthread.h>
#include <sys/types.h>

typedef void (*ttoy_PTY_readCallback_t)(
    void *data,
    char *buff,
    size_t buff_size);

typedef struct {
  /* TODO: Move some of these to private data structures */
  pid_t child;
  int master_fd, epoll_fd;
  pthread_t poll_thread;
  ttoy_PTY_readCallback_t callback;
  void *callback_data;
  int width, height;
} ttoy_PTY;

/* TODO: We need a method to execute a child process? */
/* TODO: We need a method in which to pass a callback for reading data from the
 * child process */
/* TODO: Use epoll API to watch the pseudo terminal master file handle for
 * changes
 * NOTE: wlterm uses the edge-triggered epoll API (I suppose for better
 * responsiveness)
 */
int ttoy_PTY_eventType();

void ttoy_PTY_init(
    ttoy_PTY *self,
    int width,
    int height);
void ttoy_PTY_destroy(ttoy_PTY *self);

void ttoy_PTY_startChild(
    ttoy_PTY *self,
    const char *path,
    char *const argv[],
    ttoy_PTY_readCallback_t callback,
    void *callback_data);

int ttoy_PTY_read(ttoy_PTY *self);
void ttoy_PTY_write(ttoy_PTY *self, const char *u8, size_t len);

void ttoy_PTY_resize(ttoy_PTY *self, int width, int height);

#endif
