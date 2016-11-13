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

#ifndef ST_NAIVE_COLLISION_DETECTION_H_
#define ST_NAIVE_COLLISION_DETECTION_H_

#include "collisionDetection.h"

#define ST_NAIVE_COLLISION_DETECTION_INIT_SIZE_ENTITIES 128

struct st_NaiveCollisionDetection_Internal;

typedef struct st_NaiveCollisionDetection_ {
  st_CollisionDetection base;
  struct st_NaiveCollisionDetection_Internal *internal;
} st_NaiveCollisionDetection;;

void st_NaiveCollisionDetection_init(
    st_NaiveCollisionDetection *self);

void st_NaiveCollisionDetection_destroy(
    st_NaiveCollisionDetection *self);

void st_NaiveCollisionDetection_addEntity(
    st_NaiveCollisionDetection *self,
    const st_BoundingBox *bbox,
    void *data);

void *st_NaiveCollisionDetection_checkCollision(
    st_NaiveCollisionDetection *self,
    const st_BoundingBox *bbox);

#endif
