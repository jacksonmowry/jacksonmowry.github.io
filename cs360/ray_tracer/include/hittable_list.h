#ifndef HITTABLE_LIST_H_
#define HITTABLE_LIST_H_

#include "hittable.h"
#include "interval.h"
#include "ray.h"
#include "vector.h"

VECTOR_PROTOTYPE(hittable);

typedef struct {
    vector_hittable objects;
} hittable_list;

void hittable_list_clear(hittable_list* hl);
void hittable_list_add(hittable_list* hl, hittable h);

bool hittable_list_hit(hittable_list hl, const Ray* r, interval i,
                       hit_record* record);

#endif // HITTABLE_LIST_H_
