#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "vec3.h"
#include <ctype.h>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/* int  (*write)(const char *file, u32 width, u32 height, const Pixel pixels[]);
 */

int main(int argc, char* argv[]) {
    char* file_format = ".XX";
    uint32_t width = 320;
    uint32_t height = 240;
    uint32_t seed = 0;
    uint32_t threads = 1;

    // Parse CLI opts
    opterr = 0;
    int c;

    while ((c = getopt(argc, argv, "d:w:h:s:t:")) != -1) {
        switch (c) {
        case 'd':
            file_format = optarg;
            break;
        case 'w':
            sscanf(optarg, "%u", &width);
            break;
        case 'h':
            sscanf(optarg, "%u", &height);
            break;
        case 's':
            sscanf(optarg, "%u", &seed);
            break;
        case 't':
            sscanf(optarg, "%u", &threads);
            break;
        case '?':
            if (optopt == 'd' || optopt == 'w' || optopt == 'h' ||
                optopt == 's' || optopt == 't') {
                fprintf(stderr, "Option -%c requires and argument.\n", optopt);
            } else if (isprint(optopt)) {
                fprintf(stderr, "Unknown switch -%c.\n", optopt);
            } else {
                fprintf(stderr, "Uknown option character \\x%x.\n", optopt);
            }
            break;
        default:
            fprintf(stderr, "Error while parsing arguments. %d\n", c);
            return 1;
        }
    }

    srand(seed);

    material material_red = (material){
        .type = LAMBERTIAN, .albedo = (Pixel){.r = 1.0, .g = 0.0, .b = 0.0}};
    material material_blue = (material){
        .type = LAMBERTIAN, .albedo = (Pixel){.r = 0.0, .g = 0.0, .b = 1.0}};
    material material_ground = (material){
        .type = LAMBERTIAN, .albedo = (Pixel){.r = 0.8, .g = 0.8, .b = 0.0}};
    material material_center = (material){
        .type = LAMBERTIAN, .albedo = (Pixel){.r = 0.1, .g = 0.2, .b = 0.5}};
    material material_left =
        (material){.type = DIELECTRIC, .refraction_index = 1.50};
    material material_bubble =
        (material){.type = DIELECTRIC, .refraction_index = 1.00 / 1.50};
    material material_right =
        (material){.type = METAL,
                   .albedo = (Pixel){.r = 0.8, .g = 0.6, .b = 0.2},
                   .fuzz = 1.0};

    // World building
    hittable_list hl = {.objects = vector_hittable_init(5)};
    hittable_list_add(
        &hl, (hittable){.type = SPHERE,
                        .sphere.center = (Vec3){.x = 0, .y = 0, .z = -1},
                        .sphere.radius = 0.5,
                        .sphere.mat = (struct material*)&material_center});
    hittable_list_add(
        &hl, (hittable){.type = SPHERE,
                        .sphere.center = (Vec3){.x = 0, .y = -100.5, .z = -1},
                        .sphere.radius = 100,
                        .sphere.mat = (struct material*)&material_ground});
    hittable_list_add(
        &hl, (hittable){.type = SPHERE,
                        .sphere.center = (Vec3){.x = -1.0, .y = 0.0, .z = -1},
                        .sphere.radius = 0.5,
                        .sphere.mat = (struct material*)&material_left});
    hittable_list_add(
        &hl, (hittable){.type = SPHERE,
                        .sphere.center = (Vec3){.x = -1.0, .y = 0.0, .z = -1},
                        .sphere.radius = 0.4,
                        .sphere.mat = (struct material*)&material_bubble});
    hittable_list_add(
        &hl, (hittable){.type = SPHERE,
                        .sphere.center = (Vec3){.x = 1.0, .y = 0.0, .z = -1},
                        .sphere.radius = 0.5,
                        .sphere.mat = (struct material*)&material_right});

    camera cam = camera_initialize(width, height);
    camera_render(cam, &hl);

    vector_hittable_free(hl.objects);
}
