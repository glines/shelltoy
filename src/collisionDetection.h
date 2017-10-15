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

#ifndef TTOY_COLLISION_DETECTION_H_
#define TTOY_COLLISION_DETECTION_H_

#include <assert.h>

#include "boundingBox.h"

typedef struct ttoy_CollisionEntity_ {
  ttoy_BoundingBox bbox;
  void *data;
} ttoy_CollisionEntity;

typedef struct ttoy_CollisionDetection_ ttoy_CollisionDetection;

/* Type declarations for virtual methods */
typedef void (*ttoy_CollisionDetection_addEntity_t)(
    ttoy_CollisionDetection *,
    const ttoy_BoundingBox *,
    void *);
typedef void *(*ttoy_CollisionDetection_checkCollision_t)(
    ttoy_CollisionDetection *,
    const ttoy_BoundingBox *);
typedef void *(*ttoy_CollisionDetection_destroy_t)(
    ttoy_CollisionDetection *);

typedef struct ttoy_CollisionDetection_VTable_ {
  ttoy_CollisionDetection_destroy_t destroy;
  ttoy_CollisionDetection_addEntity_t addEntity;
  ttoy_CollisionDetection_checkCollision_t checkCollision;
} ttoy_CollisionDetection_VTable;

typedef struct ttoy_CollisionDetection_ {
  const ttoy_CollisionDetection_VTable *vptr;
} ttoy_CollisionDetection;

void ttoy_CollisionDetection_init(
    ttoy_CollisionDetection *self);

void ttoy_CollisionDetection_destroy(
    ttoy_CollisionDetection *self);

void ttoy_CollisionDetection_addEntity(
    ttoy_CollisionDetection *self,
    const ttoy_BoundingBox *bbox,
    void *data);

void *ttoy_CollisionDetection_checkCollision(
    ttoy_CollisionDetection *self,
    const ttoy_BoundingBox *bbox);

#endif
