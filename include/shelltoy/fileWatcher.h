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

#ifndef SHELLTOY_FILE_WATCHER_H_
#define SHELLTOY_FILE_WATCHER_H_

#include <shelltoy/error.h>

typedef void (*st_FileWatcher_FileChangedCallback)(
    void *data,
    const char *filePath);

struct st_FileWatcher_Internal_;
typedef struct st_FileWatcher_Internal_ st_FileWatcher_Internal;

typedef struct st_FileWatcher_ {
  st_FileWatcher_Internal *internal;
} st_FileWatcher;

void
st_FileWatcher_init(
    st_FileWatcher *self);

void
st_FileWatcher_setCallback(
    st_FileWatcher *self,
    st_FileWatcher_FileChangedCallback callback,
    void *data);

st_ErrorCode
st_FileWatcher_watchFile(
    st_FileWatcher *self,
    const char *filePath);

#endif
