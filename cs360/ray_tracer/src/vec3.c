#include "vec3.h"
#include "rtweekend.h"
#include <math.h>
#include <stdbool.h>

Vec3 vec3(double x, double y, double z) {
    return (Vec3){.x = x, .y = y, .z = z};
}

Vec3 vec3_negate(Vec3 v) { return (Vec3){.x = -v.x, .y = -v.y, .z = -v.z}; }

Vec3 vec3_cross(Vec3 u, Vec3 v) {
    return vec3(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z,
                u.x * v.y - u.y * v.x);
}

inline double vec3_len2(Vec3 v) {
    return __builtin_fma(
        v.x, v.x, __builtin_fma(v.y, v.y, __builtin_fma(v.z, v.z, 0.0f)));
}

double vec3_len(Vec3 v) { return sqrt(vec3_len2(v)); }

Vec3 vec3_add(Vec3 a, Vec3 b) {
    return (Vec3){.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z};
}

Vec3 vec3_sub(Vec3 a, Vec3 b) {
    return (Vec3){.x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z};
}

Vec3 vec3_sub_dbl(Vec3 a, double b) {
    return (Vec3){.x = a.x - b, .y = a.y - b, .z = a.z - b};
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

Vec3 vec3_unit_vector(Vec3 src) {
    double len = vec3_len(src);
    return vec3(src.x / len, src.y / len, src.z / len);
}

Vec3 vec3_mul_val(Vec3 left, double val) {
    return (Vec3){.x = left.x * val, .y = left.y * val, .z = left.z * val};
}

Vec3 vec3_random() {
    return (Vec3){
        .x = random_double(), .y = random_double(), .z = random_double()};
}

Vec3 vec3_random_params(double min, double max) {
    return (Vec3){.x = random_double_min_max(min, max),
                  .y = random_double_min_max(min, max),
                  .z = random_double_min_max(min, max)};
}

Vec3 vec3_random_unit_vector() {
    while (true) {
        Vec3 p = vec3_random_params(-1, 1);
        double lensq = vec3_len2(p);
        if (1e-160 < lensq && lensq <= 1) {
            return vec3_mul_dbl(p, 1 / sqrt(lensq));
        }
    }
}

Vec3 vec3_random_on_hemisphere(const Vec3 normal) {
    Vec3 on_unit_sphere = vec3_random_unit_vector();
    if (vec3_dot(on_unit_sphere, normal) >
        0.0) { // In the same hemisphere as the normal
        return on_unit_sphere;
    } else {
        return vec3_negate(on_unit_sphere);
    }
}

bool vec3_near_zero(Vec3 v) {
    double s = 1e-8;
    return (fabs(v.x) < s) && (fabs(v.y) < s) && (fabs(v.z) < s);
}

double linear_to_gamma(double linear_component) {
    return linear_component > 0 ? sqrt(linear_component) : 0;
}

Vec3 vec3_reflect(Vec3 v, Vec3 n) {
    return vec3_sub(v, vec3_mul_dbl(n, 2 * vec3_dot(v, n)));
}

Vec3 vec3_refract(Vec3 v, Vec3 n, double etai_over_etat) {
    double cos_theta = fmin(vec3_dot(vec3_negate(v), n), 1.0);
    Vec3 r_out_perp =
        vec3_mul_dbl((vec3_add(v, vec3_mul_dbl(n, cos_theta))), etai_over_etat);
    Vec3 r_out_parallel =
        vec3_mul_dbl(n, -sqrt(fabs(1 - vec3_len2(r_out_perp))));
    return vec3_add(r_out_perp, r_out_parallel);
}

Vec3 vec3_random_in_unit_disk() {
    while (true) {
        Vec3 p = (Vec3){.x = random_double_min_max(-1, 1),
                        .y = random_double_min_max(-1, 1),
                        .z = 0};
        if (vec3_len2(p) < 1) {
            return p;
        }
    }
}
