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

#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <sys/stat.h>

#include "../logging.h"

#include "mkdir.h"

st_ErrorCode
st_mkdir(
    const char *workingDir,
    const char *dir)
{
  char *path;
  int dirLen, workingDirLen, len;
  int i, j;
  int result;
  struct stat sbuf;

  workingDirLen = strlen(workingDir);
  dirLen = strlen(dir);
  len = workingDirLen + 1 + dirLen;

  /* Concatenate the directory paths into a temporary buffer */
  path = (char *)malloc(len + 1);
  if (path == NULL) {
    return ST_ERROR_OUT_OF_MEMORY;
  }
  strcpy(path, workingDir);
  path[workingDirLen] = '/';
  strcpy(path + workingDirLen + 1, dir);

  /* Split temporary dir path, creating directories along the way */
  i = workingDirLen + 1;
  while (path[i] == '/') {
    i += 1;  /* Ignore leading slashes */
  }
  while (i < len) {
    /* Search for the next directory slash */
    for (j = i; j < len; ++j) {
      if (path[j] == '/') {
        path[j] = '\0';
        break;
      }
    }
    /* Attempt to create the directory */
    result = mkdir(path, 0755);
    if (result < 0) {
      if (errno == EEXIST) {
        /* Check for an existing file/directory */
        result = stat(path, &sbuf);
        if (result < 0) {
          ST_LOG_ERROR(
              "Failed to create '%s' directory: %s",
              path,
              strerror(EEXIST));
          return ST_ERROR_FAILED_TO_CREATE_DIRECTORY;
        } else if (!S_ISDIR(sbuf.st_mode)) {
          ST_LOG_ERROR(
              "Tried to create '%s' directory because '%s' is not a directory",
              dir,
              path);
          return ST_ERROR_FAILED_TO_CREATE_DIRECTORY;
        }
        /* The directory already existed */
      } else {
        ST_LOG_ERROR(
            "Failed to create '%s' directory: %s",
            path,
            strerror(errno));
        return ST_ERROR_FAILED_TO_CREATE_DIRECTORY;
      }
    }
    if (j < len) {
      /* Restore the directory slash */
      path[j] = '/';
    }
    /* Continue to the next directory in the path hierarchy */
    i = j;
    while (path[i] == '/') {
      i += 1;  /* Ignore repeated slashes */
    }
  }
  free(path);
  return ST_NO_ERROR;
}
