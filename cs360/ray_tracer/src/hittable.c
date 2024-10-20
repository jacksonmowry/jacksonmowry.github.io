#include "hittable.h"
#include "interval.h"
#include "ray.h"
#include "vec3.h"
#include <math.h>

void set_face_normal(hit_record* record, const Ray r,
                     const Vec3 outward_normal) {
    // sets the hit record normal vector
    // NOTE: the parameter 'outward_normal' is assumed to have unit length

    record->front_face = vec3_dot(r.direction, outward_normal) < 0;
    record->normal =
        record->front_face ? outward_normal : vec3_negate(outward_normal);
}

bool hit(hittable hittable, const Ray r, interval i, hit_record* record) {
    switch (hittable.type) {
    case SPHERE: {
        Vec3 oc = vec3_sub(hittable.center, r.origin);
        double a = vec3_len2(r.direction);
        double h = vec3_dot(r.direction, oc);
        double c = vec3_len2(oc) - (hittable.radius * hittable.radius);
        double discriminant = h * h - a * c;
        if (discriminant < 0) {
            return false;
        }

        double sqrtd = sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range
        double root = (h - sqrtd) / a;
        if (!interval_surrounds(i, root)) {
            root = (h + sqrtd) / a;
            if (!interval_surrounds(i, root)) {
                return false;
            }
        }

        record->t = root;
        record->p = ray_at(r, record->t);
        Vec3 outward_normal = vec3_mul_dbl(vec3_sub(record->p, hittable.center),
                                           1 / hittable.radius);
        set_face_normal(record, r, outward_normal);
        record->mat = hittable.mat;

        return true;

        break;
    }
    }

    return false;
}
