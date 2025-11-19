#pragma once

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
} Material;

Material material_make_lambertian(Pixel albedo);
Material material_make_metal(Pixel albedo);

bool material_scatter(Material* m, const Ray* r, const HitRecord* rec,
                      Pixel* attenuation, Ray* scattered);
