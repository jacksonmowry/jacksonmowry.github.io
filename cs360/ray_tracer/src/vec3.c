#include "vec3.h"
#include <math.h>

Vec3 vec3(double x, double y, double z) {
    return (Vec3){.x = x, .y = y, .z = z};
}

Vec3 vec3_negate(Vec3 v) { return (Vec3){.x = -v.x, .y = -v.y, .z = -v.z}; }

Vec3 vec3_cross(Vec3 u, Vec3 v) {
    return vec3(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z,
                u.x * v.y - u.y * v.x);
}

double vec3_len2(Vec3 v) { return v.x * v.x + v.y * v.y + v.z * v.z; }

double vec3_len(Vec3 v) { return sqrt(vec3_len2(v)); }

Vec3 vec3_add(Vec3 a, Vec3 b) {
    return (Vec3){.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z};
}

Vec3 vec3_sub(Vec3 a, Vec3 b) {
    return (Vec3){.x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z};
}

Vec3 vec3_mul(Vec3 a, Vec3 b) {
    return (Vec3){.x = a.x * b.x, .y = a.y * b.y, .z = a.z * b.z};
}

Vec3 vec3_mul_dbl(Vec3 a, double b) {
    return (Vec3){.x = a.x * b, .y = a.y * b, .z = a.z * b};
}

Vec3 vec3_div(Vec3 a, double t) {
    return (Vec3){.x = a.x / t, .y = a.y / t, .z = a.z / t};
}

double vec3_dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

Vec3 unit_vector(Vec3 src) {
    double len = vec3_len(src);
    return vec3(src.x / len, src.y / len, src.z / len);
}

Vec3 vec3_mul_val(Vec3 left, double val) {
    return (Vec3){.x = left.x * val, .y = left.y * val, .z = left.z * val};
}
