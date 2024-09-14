#include "vec3.h"

Vec3 vec3(double x, double y, double z) {
    return (Vec3){.x = x, .y = y, .z = z};
}

Vec3 vec3_cross(Vec3 u, Vec3 v) {
    return vec3(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z,
                u.x * v.y - u.y * v.x);
}
