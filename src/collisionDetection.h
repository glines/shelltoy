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

#ifndef ST_COLLISION_DETECTION_H_
#define ST_COLLISION_DETECTION_H_

#include <assert.h>

#include "boundingBox.h"

typedef struct st_CollisionEntity_ {
  st_BoundingBox bbox;
  void *data;
} st_CollisionEntity;

typedef struct st_CollisionDetection_ st_CollisionDetection;

/* Type declarations for virtual methods */
typedef void (*st_CollisionDetection_addEntity_t)(
    st_CollisionDetection *,
    const st_BoundingBox *,
    void *);
typedef void *(*st_CollisionDetection_checkCollision_t)(
    st_CollisionDetection *,
    const st_BoundingBox *);
typedef void *(*st_CollisionDetection_destroy_t)(
    st_CollisionDetection *);

typedef struct st_CollisionDetection_VTable_ {
  st_CollisionDetection_destroy_t destroy;
  st_CollisionDetection_addEntity_t addEntity;
  st_CollisionDetection_checkCollision_t checkCollision;
} st_CollisionDetection_VTable;

typedef struct st_CollisionDetection_ {
  const st_CollisionDetection_VTable *vptr;
} st_CollisionDetection;

void st_CollisionDetection_init(
    st_CollisionDetection *self);

void st_CollisionDetection_destroy(
    st_CollisionDetection *self);

void st_CollisionDetection_addEntity(
    st_CollisionDetection *self,
    const st_BoundingBox *bbox,
    void *data);

void *st_CollisionDetection_checkCollision(
    st_CollisionDetection *self,
    const st_BoundingBox *bbox);

#endif
