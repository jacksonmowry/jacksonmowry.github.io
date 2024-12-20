#pragma once

#include "hittable_list.h"
#include "vec3.h"
#include <stdint.h>

typedef struct {
    double aspect_ratio;
    int width;
    int height;

    Vec3 camera_center;
    Vec3 pixel00_location;
    Vec3 pixel_delta_u;
    Vec3 pixel_delta_v;

    int samples_per_pixel;
    double pixel_samples_scale;
    int max_depth;

    double vfov;
    Vec3 lookfrom;
    Vec3 lookat;
    Vec3 vup;

    double defocus_angle;
    double focus_dist;
    Vec3 defocus_disk_u;
    Vec3 defocus_disk_v;

    Vec3 u;
    Vec3 v;
    Vec3 w;
} camera;

void camera_render(camera c, const hittable_list* world, const char* file_name,
                   int (*write)(const char* file, uint32_t width,
                                uint32_t height, const Pixel pixels[]),
                   int num_threads);
camera camera_initialize(int width, int height);
Pixel ray_color(camera c, const Ray* r, int max_depth,
                const hittable_list* world);
Ray camera_get_ray(camera c, int i, int j);
