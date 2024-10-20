#include "hittable_list.h"
#include "hittable.h"
#include "interval.h"

VECTOR(hittable)

void hittable_list_clear(hittable_list* hl) { hl->objects.len = 0; }

void hittable_list_add(hittable_list* hl, hittable h) {
    vector_hittable_pb(&hl->objects, h);
}

bool hittable_list_hit(hittable_list hl, const Ray* r, interval intv,
                       hit_record* record) {
    hit_record temp_record;
    bool hit_anything = false;
    double closest_so_far = intv.max;

    for (size_t i = 0; i < hl.objects.len; i++) {
        hittable object = hl.objects.array[i];

        if (hit(object, *r, (interval){.min = intv.min, .max = closest_so_far},
                &temp_record)) {
            hit_anything = true;
            closest_so_far = temp_record.t;
            *record = temp_record;
        }
    }

    return hit_anything;
}
