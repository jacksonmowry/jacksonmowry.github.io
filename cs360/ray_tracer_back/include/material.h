#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "hittable.h"
#include "ray.h"
#include "vec3.h"
#include <stdbool.h>

typedef struct {
    union {
        struct {
            Pixel albedo;
            double fuzz;
        };
        struct {
            double refraction_index;
        };
    };
    enum { LAMBERTIAN, METAL, DIELECTRIC } type;
} material;

material material_make_lambertian(Pixel albedo);
material material_make_metal(Pixel albedo);

bool scatter(material* m, const Ray* r, const hit_record* rec,
             Pixel* attenuation, Ray* scattered);

#endif // MATERIAL_H_
