#include "camera.h"
#include "hittable_list.h"
#include "material.h"
#include "rtweekend.h"
#include "vec3.h"
#include "writer.h"
#include <bits/pthreadtypes.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>

// `ray_color` returns the RGB Pixels describing the color at the closest
// collision point
Pixel ray_color(camera c, const Ray* r, int max_depth,
                const hittable_list* world) {
    if (max_depth <= 0) {
        return (Vec3){0};
    }

    hit_record rec;

    if (hittable_list_hit(*world, r, (interval){.min = 0.001, .max = INFINITY},
                          &rec)) {
        Ray scattered;
        Pixel attenuation;

        if (scatter((material*)rec.mat, r, &rec, &attenuation, &scattered)) {
            return vec3_mul(attenuation,
                            ray_color(c, &scattered, max_depth - 1, world));
        }
        return (Pixel){0};
    }

    Vec3 unit_direction = vec3_unit_vector(r->direction);
    double a = 0.5 * (unit_direction.y + 1.0);
    return vec3_add(vec3_mul_val(vec3(1.0, 1.0, 1.0), 1.0 - a),
                    vec3_mul_val(vec3(0.5, 0.7, 1.0), a));
}

// `camera_initialize` sets up some defaults, and sets the scenes width and
// height
camera camera_initialize(int width, int height) {
    camera c;
    c.width = width;
    c.height = height;
    // NOTE You can change the image "quality" with the following line
    c.samples_per_pixel = 100;
    c.pixel_samples_scale = 1.0 / c.samples_per_pixel;
    c.max_depth = 25;
    c.vfov = 20;

    c.lookfrom = (Vec3){.x = 13, .y = 2, .z = 3};
    c.lookat = (Vec3){.x = 0, .y = 0, .z = 0};
    c.vup = (Vec3){.x = 0, .y = 1, .z = 0};

    c.defocus_angle = 0.6;
    c.focus_dist = 10.0;

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

typedef struct Shared {
    camera c;
    const hittable_list* world;
    Pixel* pixels;
    pthread_mutex_t* mutex;
    int* row;
} Shared;

void* worker(void* arg) {
    Shared* s = (Shared*)arg;

    while (true) {
        pthread_mutex_lock(s->mutex);
        int row = *(s->row) + 1;
        *(s->row) = *(s->row) + 1;
        pthread_mutex_unlock(s->mutex);

        if (row >= s->c.height) {
            break;
        }

        for (int col = 0; col < s->c.width; col++) {
            Pixel color = (Pixel){.r = 0, .g = 0, .b = 0};
            for (int sample = 0; sample < s->c.samples_per_pixel; sample++) {
                Ray new_ray = camera_get_ray(s->c, row, col);
                color = vec3_add(
                    color, ray_color(s->c, &new_ray, s->c.max_depth, s->world));
            }

            s->pixels[(row * s->c.width) + col] =
                vec3_mul_dbl(color, s->c.pixel_samples_scale);
        }
    }

    return NULL;
}

// `camera_render` takes in an entire scene `world`, the camera `c`, and an
// output file to render to
void camera_render(camera c, const hittable_list* world, const char* file_name,
                   int (*write)(const char* file, uint32_t width,
                                uint32_t height, const Pixel pixels[]),
                   int num_threads) {
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    Pixel* p = malloc(c.width * c.height * sizeof(Pixel));
    if (!p) {
        perror("malloc");
        return;
    }

    int row = 0;

    Shared s = {
        .c = c, .world = world, .pixels = p, .mutex = &mutex, .row = &row};

    pthread_t threads[num_threads];
    for (int i = 0; i < num_threads; i++) {
        pthread_create(threads + i, NULL, worker, (void*)&s);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_mutex_destroy(&mutex);

    write(file_name, c.width, c.height, p);
    free(p);
}

Vec3 sample_square(void) {
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
