/*
 * Copyright (c) 2017 Jonathan Glines
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

#include <SDL_thread.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

#include <sys/inotify.h>
#include <unistd.h>

#include "./common/array.h"
#include "logging.h"

#include <shelltoy/fileWatcher.h>

/* Private methods */
int st_FileWatcher_watchThread(
    st_FileWatcher *self);

typedef struct st_FileWatcher_Watch_ {
  int descriptor;
} st_FileWatcher_Watch;

ST_DECLARE_ARRAY(st_FileWatcher_Watch)
ST_DEFINE_ARRAY(st_FileWatcher_Watch)

struct st_FileWatcher_Internal_ {
  st_FileWatcher_WatchArray watches;
  st_FileWatcher_FileChangedCallback callback;
  void *callbackData;
  int fd;
};

void
st_FileWatcher_init(
    st_FileWatcher *self)
{
  /* Allocate memory for internal structures */
  self->internal = (st_FileWatcher_Internal *)malloc(
      sizeof(st_FileWatcher_Internal));
  st_FileWatcher_WatchArray_init(&self->internal->watches);
  self->internal->callback = NULL;
  self->internal->callbackData = NULL;
  /* Create an inotify instance and store the resulting file descriptor */
  self->internal->fd = inotify_init();
  if (self->internal->fd < 0) {
    ST_LOG_ERROR("Error initializing inotify: %s",
        strerror(errno));
  }
  /* Spawn a thread to watch the inotify file descriptor for changes */
  SDL_CreateThread(
      (SDL_ThreadFunction)st_FileWatcher_watchThread,  /* fn */
      "st_FileWatcher_watchThread",  /* name */
      (void *)self  /* data */
      );
}

void
st_FileWatcher_destroy(
    st_FileWatcher *self)
{
  /* Free each of our watch objects */
  for (size_t i = 0;
      i < st_FileWatcher_WatchArray_size(&self->internal->watches);
      ++i)
  {
    st_FileWatcher_Watch *watch;
    watch = st_FileWatcher_WatchArray_get(&self->internal->watches, i);
    free(watch);
  }
  /* Destroy our watch array */
  st_FileWatcher_WatchArray_destroy(&self->internal->watches);
  /* TODO: Close the inotify file descriptor to signal our thread to stop */
  /* TODO: Join the watch thread */
}

void
st_FileWatcher_setCallback(
    st_FileWatcher *self,
    st_FileWatcher_FileChangedCallback callback,
    void *data)
{
  self->internal->callback = callback;
  self->internal->callbackData = data;
}

st_ErrorCode
st_FileWatcher_watchFile(
    st_FileWatcher *self,
    const char *filePath)
{
  st_FileWatcher_Watch *watch;
  /* Allocate memory for the watch entry */
  watch = (st_FileWatcher_Watch *)malloc(sizeof(st_FileWatcher_Watch));
  /* Add the file at the given file path to inotify's list of files to watch */
  watch->descriptor =
    inotify_add_watch(
      self->internal->fd,  /* fd */
      filePath,  /* pathname */
      IN_MODIFY  /* mask */
      );
  fprintf(stderr, "watch descriptor: %d\n", watch->descriptor);
  /* Add this watch to our list of watches */
  /* FIXME: Why are we adding this to our list? It is not very useful... We
   * need to somehow quickly index the watches by their descriptors in order to
   * find the file path. (I don't know if we can rely on inotify to always give
   * us the file path in the event structure) */
  st_FileWatcher_WatchArray_append(&self->internal->watches, watch);
  return ST_NO_ERROR;
}

int
st_FileWatcher_watchThread(
    st_FileWatcher *self)
{
  struct inotify_event *buf, *event;
  size_t bufSize;
  fprintf(stderr, "file watch thread started\n");  /* XXX */
  /* Allocate a buffer large enough to read the next inotify event (see the
   * inotify(7) man page for details) */
  /* FIXME: Align this buffer to the allignment of struct inotify_event (see
   * the inotify(7) man page for details) */
  bufSize = (sizeof(struct inotify_event *) + NAME_MAX + 1) * 3;
  buf = (struct inotify_event *)malloc(bufSize);

  /* Watch the inotify file descriptor for events until it closes */
  while (1) {
    ssize_t bytesRead;
    fprintf(stderr, "reading inotify file descriptor (hopefully blocking)\n");  /* XXX */
    bytesRead = read(
        self->internal->fd,  /* fd */
        buf,  /* buf */
        bufSize  /* count */
        );
    fprintf(stderr, "read inotify fd, with %ld byte(s) read\n", bytesRead);  /* XXX */
    if (bytesRead < 0) {
      /* TODO: Figure out which of these error codes signal us to join the main
       * thread */
      ST_LOG_ERROR(
          "inotify file descriptor error: %s\n",
          strerror(errno));  /* XXX */
      break;  /* Exit our thread */
    }
    /* Iterate over the inotify events we received */
    for (
        event = buf;
        event < buf + bytesRead;
        event += sizeof(struct inotify_event) + event->len)
    {
      if (self->internal->callback != NULL) {
        self->internal->callback(
            self->internal->callbackData,  /* data */
            event->name  /* name */
            );
      }
    }
  }
  free(buf);
  fprintf(stderr, "Whoops, we left the watch thread\n");  /* XXX */
  return 0;
}
