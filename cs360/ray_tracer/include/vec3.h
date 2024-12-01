#pragma once

#include <stdbool.h>
typedef struct {
    union {
        struct {
            double x;
            double y;
            double z;
        };
        struct {
            double r;
            double g;
            double b;
        };
        double vec[3];
    };
} Vec3;
typedef Vec3 Pixel;

Vec3 vec3(double, double, double);
Vec3 vec3_cross(Vec3, Vec3);
Vec3 vec3(double x, double y, double z);
Vec3 vec3_negate(Vec3);
Vec3 vec3_cross(Vec3 u, Vec3 v);
double vec3_len2(Vec3 v);
double vec3_len(Vec3 v);
Vec3 vec3_add(Vec3 a, Vec3 b);
Vec3 vec3_sub(Vec3 a, Vec3 b);
Vec3 vec3_mul(Vec3 a, Vec3 b);
Vec3 vec3_mul_dbl(Vec3 a, double b);
Vec3 vec3_div(Vec3 a, double t);
double vec3_dot(Vec3 a, Vec3 b);
Vec3 vec3_unit_vector(Vec3 src);
Vec3 vec3_mul_val(Vec3 left, double val);
Vec3 vec3_random(void);
Vec3 vec3_random_params(double min, double max);
Vec3 vec3_random_unit_vector(void);
Vec3 vec3_random_on_hemisphere(const Vec3 normal);
bool vec3_near_zero(Vec3 v);
Vec3 vec3_reflect(Vec3 v, Vec3 n);
Vec3 vec3_refract(Vec3 v, Vec3 n, double etai_over_etat);
Vec3 vec3_random_in_unit_disk(void);

double linear_to_gamma(double linear_component);
