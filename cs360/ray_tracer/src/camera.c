#include "camera.h"
#include "hittable_list.h"
#include "material.h"
#include "rtweekend.h"
#include "vec3.h"
#include "writer.h"
#include <math.h>
#include <stdint.h>

Pixel ray_color(camera c, const Ray* r, int max_depth,
                const hittable_list* world) {
    if (max_depth <= 0) {
        return (Vec3){};
    }

    hit_record rec;

    if (hittable_list_hit(*world, r, (interval){.min = 0.001, .max = INFINITY},
                          &rec)) {
        Ray scattered;
        Pixel attenuation;
        /* Vec3 direction = vec3_add(rec.normal, vec3_random_unit_vector()); */
        if (scatter((material*)rec.mat, r, &rec, &attenuation, &scattered)) {
            return vec3_mul(attenuation,
                            ray_color(c, &scattered, max_depth - 1, world));
        }
        return (Pixel){};
    }

    Vec3 unit_direction = vec3_unit_vector(r->direction);
    double a = 0.5 * (unit_direction.y + 1.0);
    return vec3_add(vec3_mul_val(vec3(1.0, 1.0, 1.0), 1.0 - a),
                    vec3_mul_val(vec3(0.5, 0.7, 1.0), a));
}

camera camera_initialize(int width, int height) {
    camera c;
    c.width = width;
    c.height = height;
    c.samples_per_pixel = 100;
    c.pixel_samples_scale = 1.0 / c.samples_per_pixel;
    c.max_depth = 50;
    c.vfov = 20;

    c.lookfrom = (Vec3){.x = -2, .y = 2, .z = 1};
    c.lookat = (Vec3){.x = 0, .y = 0, .z = -1};
    c.vup = (Vec3){.x = 0, .y = 1, .z = 0};

    c.defocus_angle = 10.0;
    c.focus_dist = 3.4;

    c.aspect_ratio = (double)width / (double)height;

    double theta = degrees_to_radians(c.vfov);
    double h = tan(theta / 2);
    double viewport_height = 2 * h * c.focus_dist;
    double viewport_width = viewport_height * ((double)width / height);
    c.camera_center = c.lookfrom;

    c.w = vec3_unit_vector(vec3_sub(c.lookfrom, c.lookat));
    c.u = vec3_unit_vector(vec3_cross(c.vup, c.w));
    c.v = vec3_cross(c.w, c.u);

    // Calculating the vectors for outer edges
    Vec3 viewport_u = vec3_mul_dbl(c.u, viewport_width);
    Vec3 viewport_v = vec3_mul_dbl(vec3_negate(c.v), viewport_height);

    // Calcuate the horizontal and vertical delta vectors for pixel spacing
    c.pixel_delta_u = vec3_div(viewport_u, width);
    c.pixel_delta_v = vec3_div(viewport_v, height);

    // Pos of first pixel (upper left)
    Vec3 viewport_upper_left = vec3_sub(
        vec3_sub(vec3_sub(c.camera_center, vec3_mul_dbl(c.w, c.focus_dist)),
                 vec3_div(viewport_u, 2)),
        vec3_div(viewport_v, 2));
    c.pixel00_location =
        vec3_add(viewport_upper_left,
                 vec3_mul_dbl(vec3_add(c.pixel_delta_u, c.pixel_delta_v), 0.5));

    double defocus_radius =
        c.focus_dist * tan(degrees_to_radians(c.defocus_angle / 2));
    c.defocus_disk_u = vec3_mul_dbl(c.u, defocus_radius);
    c.defocus_disk_v = vec3_mul_dbl(c.v, defocus_radius);

    return c;
}

void camera_render(camera c, const hittable_list* world) {
    Pixel* p = malloc(c.width * c.height * sizeof(Pixel));

    for (uint32_t row = 0; row < c.height; row++) {
        /* fprintf(stderr, "Rows remaining: %d\n", c.height - row); */
        for (uint32_t col = 0; col < c.width; col++) {
            Vec3 pixel_location =
                vec3_add(c.pixel00_location,
                         vec3_add(vec3_mul_dbl(c.pixel_delta_u, col),
                                  vec3_mul_dbl(c.pixel_delta_v, row)));

            Vec3 direction = vec3_sub(pixel_location, c.camera_center);

            Ray r = (Ray){.origin = c.camera_center, .direction = direction};
            Pixel color = (Pixel){.r = 0, .g = 0, .b = 0};
            for (int sample = 0; sample < c.samples_per_pixel; sample++) {
                Ray new_ray = camera_get_ray(c, row, col);
                color =
                    vec3_add(color, ray_color(c, &new_ray, c.max_depth, world));
            }

            p[(row * c.width) + col] =
                vec3_mul_dbl(color, c.pixel_samples_scale);
        }
    }
    /* fprintf(stderr, "Done generating pixels!\n"); */

    /* write_prop(NULL, 16, 16, p); */
    write_ppm(NULL, c.width, c.height, p);
    free(p);
}

Vec3 sample_square() {
    return (Vec3){
        .x = random_double() - 0.5, .y = random_double() - 0.5, .z = 0};
}

Vec3 defocus_disk_sample(camera c) {
    Vec3 p = vec3_random_in_unit_disk();
    return vec3_add(c.camera_center,
                    vec3_add(vec3_mul_dbl(c.defocus_disk_u, p.x),
                             vec3_mul_dbl(c.defocus_disk_v, p.y)));
}

Ray camera_get_ray(camera c, int i, int j) {
    Vec3 offset = sample_square();
    Vec3 pixel_sample =
        vec3_add(c.pixel00_location,
                 vec3_add(vec3_mul_dbl(c.pixel_delta_u, (j + offset.x)),
                          vec3_mul_dbl(c.pixel_delta_v, (i + offset.y))));

    Vec3 ray_origin =
        (c.defocus_angle <= 0) ? c.camera_center : defocus_disk_sample(c);
    Vec3 ray_direciton = vec3_sub(pixel_sample, ray_origin);

    return (Ray){.origin = ray_origin, .direction = ray_direciton};
}