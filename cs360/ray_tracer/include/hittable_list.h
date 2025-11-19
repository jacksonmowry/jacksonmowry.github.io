#pragma once

#include "hittable.h"
#include "interval.h"
#include "ray.h"
#include "vector.h"

VECTOR_PROTOTYPE(Hittable)

typedef struct {
    vector_Hittable objects;
} HittableList;

void hittable_list_clear(HittableList* hl);
void hittable_list_add(HittableList* hl, Hittable h);

bool hittable_list_hit(HittableList hl, const Ray* r, Interval i,
                       HitRecord* record);
