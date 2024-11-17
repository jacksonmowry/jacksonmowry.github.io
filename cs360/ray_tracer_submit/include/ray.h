#ifndef RAY_H_
#define RAY_H_

#include "vec3.h"
#include <stdbool.h>

typedef struct {
    Vec3 origin;
    Vec3 direction;
} Ray;

double hit_sphere(Vec3 center, double radius, Ray r);
Vec3 ray_at(Ray r, double t);

#endif // RAY_H_
