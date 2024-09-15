#ifndef CAMERA_H_
#define CAMERA_H_

#include "hittable_list.h"
#include "vec3.h"

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

void camera_render(camera cam, const hittable_list* world);
camera camera_initialize(int width, int height);
Pixel ray_color(camera c, const Ray* r, int max_depth,
                const hittable_list* world);
Ray camera_get_ray(camera c, int i, int j);

#endif // CAMERA_H_
