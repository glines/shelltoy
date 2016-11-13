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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "naiveCollisionDetection.h"

/* Storage for vtable */
const st_CollisionDetection_VTable st_NaiveCollisionDetection_vtable = {
  .destroy =
    (st_CollisionDetection_destroy_t)
    st_NaiveCollisionDetection_destroy,
  .addEntity =
    (st_CollisionDetection_addEntity_t)
    st_NaiveCollisionDetection_addEntity,
  .checkCollision =
    (st_CollisionDetection_checkCollision_t)
    st_NaiveCollisionDetection_checkCollision,
};

/* Private member function declarations */
void st_NaiveCollisionDetection_growEntities(
    st_NaiveCollisionDetection *self);

struct st_NaiveCollisionDetection_Internal {
  st_CollisionEntity *entities;
  size_t numEntities, sizeEntities;
};

void st_NaiveCollisionDetection_init(
    st_NaiveCollisionDetection *self)
{
  st_CollisionDetection_init((st_CollisionDetection *)self);
  /* Initialize the vtable pointer */
  self->base.vptr = &st_NaiveCollisionDetection_vtable;
  /* Initialize internal data structure */
  self->internal = (struct st_NaiveCollisionDetection_Internal *)malloc(
      sizeof(*self->internal));
  self->internal->sizeEntities = ST_NAIVE_COLLISION_DETECTION_INIT_SIZE_ENTITIES;
  self->internal->entities = (st_CollisionEntity *)malloc(
      sizeof(st_CollisionEntity) * self->internal->sizeEntities);
  self->internal->numEntities = 0;

}

void st_NaiveCollisionDetection_destroy(
    st_NaiveCollisionDetection *self)
{
  /* Free internal data structure */
  free(self->internal->entities);
  free(self->internal);
  st_CollisionDetection_destroy((st_CollisionDetection *)self);
}

void st_NaiveCollisionDetection_addEntity(
    st_NaiveCollisionDetection *self,
    const st_BoundingBox *bbox,
    void *data)
{
  st_CollisionEntity *newEntity;
  /* Ensure our array of entities is large enough */
  if (self->internal->numEntities + 1 > self->internal->sizeEntities) {
    st_NaiveCollisionDetection_growEntities(self);
  }
  /* Add to our list of entities */
  newEntity = &self->internal->entities[self->internal->numEntities++];
  memcpy(&newEntity->bbox, bbox, sizeof(*bbox));
  newEntity->data = data;
}

void *st_NaiveCollisionDetection_checkCollision(
    st_NaiveCollisionDetection *self,
    const st_BoundingBox *bbox)
{
  st_CollisionEntity *currentEntity;
  /* Simply iterate over all collision entities and check if they intersect
   * with this bounding box */
  for (size_t i = 0; i < self->internal->numEntities; ++i) {
    currentEntity = &self->internal->entities[i];
    if (st_BoundingBox_checkIntersection(bbox, &currentEntity->bbox)) {
      return currentEntity->data;
    }
  }
  return NULL;
}

void st_NaiveCollisionDetection_growEntities(
    st_NaiveCollisionDetection *self)
{
  st_CollisionEntity *newEntities;

  /* Double the size of the array of entities */
  newEntities = (st_CollisionEntity *)malloc(self->internal->sizeEntities * 2);
  memcpy(newEntities, self->internal->entities,
      self->internal->numEntities * sizeof(st_CollisionEntity));
  free(self->internal->entities);
  self->internal->entities = newEntities;
}
