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

#ifndef TTOY_NAIVE_COLLISION_DETECTION_H_
#define TTOY_NAIVE_COLLISION_DETECTION_H_

#include "collisionDetection.h"

#define TTOY_NAIVE_COLLISION_DETECTION_INIT_SIZE_ENTITIES 128

struct ttoy_NaiveCollisionDetection_Internal;

typedef struct ttoy_NaiveCollisionDetection_ {
  ttoy_CollisionDetection base;
  struct ttoy_NaiveCollisionDetection_Internal *internal;
} ttoy_NaiveCollisionDetection;

void ttoy_NaiveCollisionDetection_init(
    ttoy_NaiveCollisionDetection *self);

void ttoy_NaiveCollisionDetection_destroy(
    ttoy_NaiveCollisionDetection *self);

void ttoy_NaiveCollisionDetection_addEntity(
    ttoy_NaiveCollisionDetection *self,
    const ttoy_BoundingBox *bbox,
    void *data);

void *ttoy_NaiveCollisionDetection_checkCollision(
    ttoy_NaiveCollisionDetection *self,
    const ttoy_BoundingBox *bbox);

#endif
