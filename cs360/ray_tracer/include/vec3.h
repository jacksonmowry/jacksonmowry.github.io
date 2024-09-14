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

Vec3 vec3(double, double, double);
Vec3 vec3_cross(Vec3, Vec3);

#endif // VEC3_H_
