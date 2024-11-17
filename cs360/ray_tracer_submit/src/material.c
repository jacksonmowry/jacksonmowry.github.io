#include "material.h"
#include "rtweekend.h"
#include "vec3.h"
#include <math.h>
#include <stdint.h>

material material_make_lambertian(Pixel albedo) {
    return (material){.albedo = albedo, .type = LAMBERTIAN};
}
material material_make_metal(Pixel albedo) {
    return (material){.albedo = albedo, .type = METAL};
}

static double reflectance(double cosine, double refraction_index) {
    double r0 = (1 - refraction_index) / (1 + refraction_index);
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow((1 - cosine), 5);
}

// `scatter` takes a material `m` and ray `r` then calculates the ray bounce
// properties depending on hit angle
bool scatter(material* m, const Ray* r, const hit_record* rec,
             Pixel* attenuation, Ray* scattered) {
    if (m) {
        switch (m->type) {
        case LAMBERTIAN: {
            Vec3 scatter_direction =
                vec3_add(rec->normal, vec3_random_unit_vector());

            if (vec3_near_zero(scatter_direction)) {
                scatter_direction = rec->normal;
            }

            *scattered =
                (Ray){.origin = rec->p, .direction = scatter_direction};
            *attenuation = m->albedo;
            return true;
        }
        case METAL: {
            Vec3 reflected = vec3_reflect(r->direction, rec->normal);
            reflected =
                vec3_add(vec3_unit_vector(reflected),
                         vec3_mul_dbl(vec3_random_unit_vector(), m->fuzz));
            *scattered = (Ray){.origin = rec->p, .direction = reflected};
            *attenuation = m->albedo;
            return vec3_dot(scattered->direction, rec->normal) > 0;
        }
        case DIELECTRIC: {
            *attenuation = (Pixel){.r = 1, .g = 1, .b = 1};
            double ri = rec->front_face ? (1.0 / m->refraction_index)
                                        : m->refraction_index;

            Vec3 unit_direction = vec3_unit_vector(r->direction);
            double cos_theta =
                fmin(vec3_dot(vec3_negate(unit_direction), rec->normal), 1.0);
            double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

            bool cannot_refract = ri * sin_theta > 1.0;
            Vec3 direction;

            if (cannot_refract ||
                reflectance(cos_theta, ri) > random_double()) {
                direction = vec3_reflect(unit_direction, rec->normal);
            } else {
                direction = vec3_refract(unit_direction, rec->normal, ri);
            }

            *scattered = (Ray){.origin = rec->p, .direction = direction};
            return true;
        }
        }
    }

    return false;
}
