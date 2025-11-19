#pragma once

#include "interval.h"
#include "ray.h"
#include "vec3.h"
#include <stdbool.h>

struct Material;

typedef struct {
    Vec3 p;
    Vec3 normal;
    double t;
    bool front_face;
    struct Material* mat;
} HitRecord;

void set_face_normal(HitRecord* record, const Ray r, const Vec3 outward_normal);

typedef struct {
    union {
        struct {
            Vec3 center;
            double radius;
            struct Material* mat;
        };
    };
    enum { SPHERE } type;
} Hittable;

bool hit(Hittable h, const Ray, Interval i, HitRecord* record);
