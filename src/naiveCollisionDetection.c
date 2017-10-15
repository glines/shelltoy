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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "naiveCollisionDetection.h"

/* Storage for vtable */
const ttoy_CollisionDetection_VTable ttoy_NaiveCollisionDetection_vtable = {
  .destroy =
    (ttoy_CollisionDetection_destroy_t)
    ttoy_NaiveCollisionDetection_destroy,
  .addEntity =
    (ttoy_CollisionDetection_addEntity_t)
    ttoy_NaiveCollisionDetection_addEntity,
  .checkCollision =
    (ttoy_CollisionDetection_checkCollision_t)
    ttoy_NaiveCollisionDetection_checkCollision,
};

/* Private member function declarations */
void ttoy_NaiveCollisionDetection_growEntities(
    ttoy_NaiveCollisionDetection *self);

struct ttoy_NaiveCollisionDetection_Internal {
  ttoy_CollisionEntity *entities;
  size_t numEntities, sizeEntities;
};

void ttoy_NaiveCollisionDetection_init(
    ttoy_NaiveCollisionDetection *self)
{
  ttoy_CollisionDetection_init((ttoy_CollisionDetection *)self);
  /* Initialize the vtable pointer */
  self->base.vptr = &ttoy_NaiveCollisionDetection_vtable;
  /* Initialize internal data structure */
  self->internal = (struct ttoy_NaiveCollisionDetection_Internal *)malloc(
      sizeof(*self->internal));
  self->internal->sizeEntities = TTOY_NAIVE_COLLISION_DETECTION_INIT_SIZE_ENTITIES;
  self->internal->entities = (ttoy_CollisionEntity *)malloc(
      sizeof(ttoy_CollisionEntity) * self->internal->sizeEntities);
  self->internal->numEntities = 0;

}

void ttoy_NaiveCollisionDetection_destroy(
    ttoy_NaiveCollisionDetection *self)
{
  /* Free internal data structure */
  free(self->internal->entities);
  free(self->internal);
  ttoy_CollisionDetection_destroy((ttoy_CollisionDetection *)self);
}

void ttoy_NaiveCollisionDetection_addEntity(
    ttoy_NaiveCollisionDetection *self,
    const ttoy_BoundingBox *bbox,
    void *data)
{
  ttoy_CollisionEntity *newEntity;
  /* Ensure our array of entities is large enough */
  if (self->internal->numEntities + 1 > self->internal->sizeEntities) {
    ttoy_NaiveCollisionDetection_growEntities(self);
  }
  /* Add to our list of entities */
  newEntity = &self->internal->entities[self->internal->numEntities++];
  /* FIXME: It might be better (more useful) to store a pointer to the bounding
   * box rather than copying its contents */
  memcpy(&newEntity->bbox, bbox, sizeof(*bbox));
  newEntity->data = data;
}

void *ttoy_NaiveCollisionDetection_checkCollision(
    ttoy_NaiveCollisionDetection *self,
    const ttoy_BoundingBox *bbox)
{
  ttoy_CollisionEntity *currentEntity;
  /* Simply iterate over all collision entities and check if they intersect
   * with this bounding box */
  for (size_t i = 0; i < self->internal->numEntities; ++i) {
    currentEntity = &self->internal->entities[i];
    if (ttoy_BoundingBox_checkIntersection(bbox, &currentEntity->bbox)) {
      return currentEntity->data;
    }
  }
  return NULL;
}

void ttoy_NaiveCollisionDetection_growEntities(
    ttoy_NaiveCollisionDetection *self)
{
  ttoy_CollisionEntity *newEntities;

  /* Double the size of the array of entities */
  newEntities = (ttoy_CollisionEntity *)malloc(self->internal->sizeEntities * 2);
  memcpy(newEntities, self->internal->entities,
      self->internal->numEntities * sizeof(ttoy_CollisionEntity));
  free(self->internal->entities);
  self->internal->entities = newEntities;
}
