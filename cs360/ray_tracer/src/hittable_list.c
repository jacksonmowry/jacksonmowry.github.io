#include "hittable_list.h"
#include "hittable.h"
#include "interval.h"

VECTOR(Hittable);

void hittable_list_clear(HittableList* hl) { hl->objects.len = 0; }

void hittable_list_add(HittableList* hl, Hittable h) {
    vector_Hittable_pb(&hl->objects, h);
}

bool hittable_list_hit(HittableList hl, const Ray* r, Interval intv,
                       HitRecord* record) {
    HitRecord temp_record;
    bool hit_anything = false;
    double closest_so_far = intv.max;

    for (size_t i = 0; i < hl.objects.len; i++) {
        Hittable object = hl.objects.array[i];

        if (hit(object, *r, (Interval){.min = intv.min, .max = closest_so_far},
                &temp_record)) {
            hit_anything = true;
            closest_so_far = temp_record.t;
            *record = temp_record;
        }
    }

    return hit_anything;
}
