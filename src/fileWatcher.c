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

#include <ttoy/fileWatcher.h>

/* Private methods */
int ttoy_FileWatcher_watchThread(
    ttoy_FileWatcher *self);

typedef struct ttoy_FileWatcher_Watch_ {
  int descriptor;
  char *filePath, *fileName, *dirPath;
} ttoy_FileWatcher_Watch;

TTOY_DECLARE_ARRAY(ttoy_FileWatcher_Watch)
TTOY_DEFINE_ARRAY(ttoy_FileWatcher_Watch)

struct ttoy_FileWatcher_Internal_ {
  ttoy_FileWatcher_WatchArray watches;
  ttoy_FileWatcher_FileChangedCallback callback;
  void *callbackData;
  int fd;
};

void
ttoy_FileWatcher_init(
    ttoy_FileWatcher *self)
{
  /* Allocate memory for internal structures */
  self->internal = (ttoy_FileWatcher_Internal *)malloc(
      sizeof(ttoy_FileWatcher_Internal));
  ttoy_FileWatcher_WatchArray_init(&self->internal->watches);
  self->internal->callback = NULL;
  self->internal->callbackData = NULL;
  /* Create an inotify instance and store the resulting file descriptor */
  self->internal->fd = inotify_init();
  if (self->internal->fd < 0) {
    TTOY_LOG_ERROR("Error initializing inotify: %s",
        strerror(errno));
  }
  /* Spawn a thread to watch the inotify file descriptor for changes */
  SDL_CreateThread(
      (SDL_ThreadFunction)ttoy_FileWatcher_watchThread,  /* fn */
      "ttoy_FileWatcher_watchThread",  /* name */
      (void *)self  /* data */
      );
}

void
ttoy_FileWatcher_destroy(
    ttoy_FileWatcher *self)
{
  /* Free each of our watch objects */
  for (size_t i = 0;
      i < ttoy_FileWatcher_WatchArray_size(&self->internal->watches);
      ++i)
  {
    ttoy_FileWatcher_Watch *watch;
    watch = ttoy_FileWatcher_WatchArray_get(&self->internal->watches, i);
    free(watch->filePath);
    free(watch->fileName);
    free(watch->dirPath);
    free(watch);
  }
  /* Destroy our watch array */
  ttoy_FileWatcher_WatchArray_destroy(&self->internal->watches);
  /* TODO: Close the inotify file descriptor to signal our thread to stop */
  /* TODO: Join the watch thread */
}

void
ttoy_FileWatcher_setCallback(
    ttoy_FileWatcher *self,
    ttoy_FileWatcher_FileChangedCallback callback,
    void *data)
{
  /* FIXME: We need this in init. It's ugly to synchronize these. */
  self->internal->callback = callback;
  self->internal->callbackData = data;
}

ttoy_ErrorCode
ttoy_FileWatcher_watchFile(
    ttoy_FileWatcher *self,
    const char *filePath)
{
  const char *fileName;
  ttoy_FileWatcher_Watch *watch;
  /* Allocate memory for the watch entry */
  watch = (ttoy_FileWatcher_Watch *)malloc(sizeof(ttoy_FileWatcher_Watch));
  /* Copy the file path */
  watch->filePath = (char *)malloc(strlen(filePath) + 1);
  strcpy(watch->filePath, filePath);
  /* NOTE: Many editors on Linux will actually write to a new file and then
   * replace the original file with the new file by renaming the files. This
   * results in an entirely new file with an entirely new hard link. The
   * inotify API will not detect this situation if we simply watch the file
   * itself; we must watch the entire directory for changes. */
  fileName = strrchr(filePath, '/') ? strrchr(filePath, '/') + 1 : filePath;
  watch->fileName = (char *)malloc(strlen(fileName) + 1);
  strcpy(watch->fileName, fileName);
  if (fileName == filePath) {
    watch->dirPath = (char *)malloc(2);
    strcpy(watch->dirPath, ".");
  } else {
    watch->dirPath = (char *)malloc(fileName - filePath + 1);
    memcpy(watch->dirPath, filePath, fileName - filePath);
    watch->dirPath[fileName - filePath] = '\0';
  }
  fprintf(stderr, "watch dir: %s\n", watch->dirPath);
  /* Add the file at the given file path to inotify's list of files to watch */
  watch->descriptor =
    inotify_add_watch(
      self->internal->fd,  /* fd */
      watch->dirPath,  /* pathname */
      IN_CLOSE_WRITE | IN_MOVED_TO | IN_MODIFY  /* mask */
      );
  fprintf(stderr, "watch descriptor: %d\n", watch->descriptor);  /* XXX */
  /* Add this watch to our list of watches */
  /* FIXME: Why are we adding this to our list? It is not very useful... We
   * need to somehow quickly index the watches by their descriptors in order to
   * find the file path. (I don't know if we can rely on inotify to always give
   * us the file path in the event structure) */
  ttoy_FileWatcher_WatchArray_append(&self->internal->watches, watch);
  return TTOY_NO_ERROR;
}

int
ttoy_FileWatcher_watchThread(
    ttoy_FileWatcher *self)
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
    // fprintf(stderr, "reading inotify file descriptor (hopefully blocking)\n");  /* XXX */
    bytesRead = read(
        self->internal->fd,  /* fd */
        buf,  /* buf */
        bufSize  /* count */
        );
    // fprintf(stderr, "read inotify fd, with %ld byte(s) read\n", bytesRead);  /* XXX */
    if (bytesRead < 0) {
      /* TODO: Figure out which of these error codes signal us to join the main
       * thread */
      TTOY_LOG_ERROR(
          "inotify file descriptor error: %s\n",
          strerror(errno));  /* XXX */
      break;  /* Exit our thread */
    }
    if (self->internal->callback == NULL) {  /* FIXME: Synchronize this! Ugh! */
      /* Nothing to do with no callback */
      continue;
    }
    /* Iterate over the inotify events we received */
    for (
        event = buf;
        event < buf + bytesRead;
        event += sizeof(struct inotify_event) + event->len)
    {
      /* TODO: Check for relevant events */
      /* Note that a number of different events can indicate changes to our
       * file, namely creation, modification, and renaming. */
      /* XXX */
      /*
      if (event->mask & IN_CLOSE_WRITE) {
        fprintf(stderr, "file close write '%s'\n", event->name);
      }
      if (event->mask & IN_MOVED_TO) {
        fprintf(stderr, "file moved to '%s'\n", event->name);
      }
      if (event->mask & IN_MODIFY) {
        fprintf(stderr, "file modified '%s'\n", event->name);
      }
      */
      /* TODO: Check for watches matching this event */
      for (size_t i = 0;
          i < ttoy_FileWatcher_WatchArray_size(&self->internal->watches);
          ++i)
      {
        ttoy_FileWatcher_Watch *watch;
        watch = ttoy_FileWatcher_WatchArray_get(&self->internal->watches, i);
        /* Check for matching watch descriptors (i.e. the directory matches) */
        if (watch->descriptor != event->wd)
          continue;
        /* Check for matching file names */
        if (strcmp(watch->fileName, event->name) != 0)
          continue;
        assert(self->internal->callback != NULL);
        self->internal->callback(
            self->internal->callbackData,  /* data */
            watch->filePath  /* filePath */
            );
      }
    }
  }
  free(buf);
  fprintf(stderr, "Whoops, we left the watch thread\n");  /* XXX */
  return 0;
}
