#ifndef HITTABLE_H_
#define HITTABLE_H_

#include "interval.h"
#include "ray.h"
#include "vec3.h"
#include <stdbool.h>

struct material;

typedef struct {
    Vec3 p;
    Vec3 normal;
    double t;
    bool front_face;
    struct material* mat;
} hit_record;

void set_face_normal(hit_record* record, const Ray r,
                     const Vec3 outward_normal);

struct material;

typedef struct {
    union {
        struct {
            Vec3 center;
            double radius;
            struct material* mat;
        } sphere;
    };
    enum { SPHERE } type;
} hittable;

bool hit(hittable h, const Ray, interval i, hit_record* record);

#endif // HITTABLE_H_
