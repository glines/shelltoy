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

#ifndef TTOY_FILE_WATCHER_H_
#define TTOY_FILE_WATCHER_H_

#include <ttoy/error.h>

typedef void (*ttoy_FileWatcher_FileChangedCallback)(
    void *data,
    const char *filePath);

struct ttoy_FileWatcher_Internal_;
typedef struct ttoy_FileWatcher_Internal_ ttoy_FileWatcher_Internal;

typedef struct ttoy_FileWatcher_ {
  ttoy_FileWatcher_Internal *internal;
} ttoy_FileWatcher;

void
ttoy_FileWatcher_init(
    ttoy_FileWatcher *self);

void
ttoy_FileWatcher_setCallback(
    ttoy_FileWatcher *self,
    ttoy_FileWatcher_FileChangedCallback callback,
    void *data);

ttoy_ErrorCode
ttoy_FileWatcher_watchFile(
    ttoy_FileWatcher *self,
    const char *filePath);

#endif