#include "camera.h"
#include "hittable_list.h"
#include "rtweekend.h"
#include "writer.h"
#include <math.h>
#include <stdint.h>

Pixel ray_color(camera c, const Ray* r, const hittable_list* world) {
    hit_record rec;

    if (hittable_list_hit(*world, r, (interval){.min = 0, .max = INFINITY},
                          &rec)) {
        return vec3_mul_dbl(
            vec3_add(rec.normal, (Pixel){.r = 1, .g = 1, .b = 1}), 0.5);
    }

    Vec3 unit_direction = unit_vector(r->direction);
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

    c.aspect_ratio = (double)width / (double)height;

    double focal_length = 1.0;
    Vec3 focal = (Vec3){.x = 0, .y = 0, .z = focal_length};
    double viewport_height = 2.0;
    double viewport_width = viewport_height * c.aspect_ratio;
    c.camera_center = (Vec3){.x = 0, .y = 0, .z = 0};

    // Calculating the vectors for outer edges
    Vec3 viewport_u = (Vec3){.x = viewport_width, .y = 0, .z = 0};  // top
    Vec3 viewport_v = (Vec3){.x = 0, .y = viewport_height, .z = 0}; // left side

    // Calcuate the horizontal and vertical delta vectors for pixel spacing
    c.pixel_delta_u = vec3_div(viewport_u, width);
    c.pixel_delta_v = vec3_div(viewport_v, height);

    // Pos of first pixel (upper left)
    Vec3 viewport_upper_left = vec3_sub(
        vec3_sub(vec3_sub(c.camera_center, focal), vec3_div(viewport_u, 2)),
        vec3_div(viewport_v, 2));
    c.pixel00_location =
        vec3_add(viewport_upper_left,
                 vec3_mul_dbl(vec3_add(c.pixel_delta_u, c.pixel_delta_v), 0.5));

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
                color = vec3_add(color, ray_color(c, &new_ray, world));
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

Ray camera_get_ray(camera c, int i, int j) {
    Vec3 offset = sample_square();
    Vec3 pixel_sample =
        vec3_add(c.pixel00_location,
                 vec3_add(vec3_mul_dbl(c.pixel_delta_u, (j + offset.x)),
                          vec3_mul_dbl(c.pixel_delta_v, (i + offset.y))));

    Vec3 ray_direciton = vec3_sub(pixel_sample, c.camera_center);

    return (Ray){.origin = c.camera_center, .direction = ray_direciton};
}
