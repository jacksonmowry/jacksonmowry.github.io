#include "ray.h"
#include "vec3.h"
#include <math.h>

// `hit_sphere` detects if the provided ray `r` collides with a sphere centered
// at `center` and with radius `radius`
double hit_sphere(Vec3 center, double radius, Ray r) {
    Vec3 oc = vec3_sub(center, r.origin);
    double a = vec3_len2(r.direction);
    double h = vec3_dot(r.direction, oc);
    double c = vec3_len2(oc) - (radius * radius);
    double discriminant = h * h - a * c;
    if (discriminant < 0) {
        return -1.0;
    } else {
        return (h - sqrt(discriminant)) / a;
    }
}

// `ray_at` moves along a vector by factor `t`, returning the new vector
Vec3 ray_at(Ray r, double t) {
    return vec3_add(r.origin, vec3_mul(r.direction, vec3(t, t, t)));
}
