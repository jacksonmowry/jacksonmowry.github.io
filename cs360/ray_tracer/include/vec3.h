#ifndef VEC3_H_
#define VEC3_H_

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
Vec3 unit_vector(Vec3 src);
Vec3 vec3_mul_val(Vec3 left, double val);

#endif // VEC3_H_
