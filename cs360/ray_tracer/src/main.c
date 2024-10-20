#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "rtweekend.h"
#include "vec3.h"
#include <ctype.h>
#include <getopt.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

VECTOR_PROTOTYPE(material)
VECTOR(material)

int main(int argc, char* argv[]) {
    char* file_format = ".XX";
    uint32_t width = 320;
    uint32_t height = 240;
    uint32_t seed = time(NULL);
    uint32_t threads = 1;
    char* file_name = argv[argc - 1];

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

    char* file_extension = strrchr(file_name, '.');
    if (file_extension && (strcmp("rto", file_extension + 1) ||
                           strcmp("bmp", file_extension + 1) ||
                           strcmp("ppm", file_extension + 1))) {
        file_extension++;
    } else {
        // Grab file output type from switches
        if (!file_format) {
            fprintf(stderr,
                    "The -d switch must be given when outputting to stdout\n");
            return 1;
        }
        file_extension = file_format;
        file_name = NULL;
    }

    srand(seed);

    hittable_list hl = {.objects = vector_hittable_init(5)};

    material* material_ground = malloc(sizeof(material));
    *material_ground = (material){
        .type = LAMBERTIAN, .albedo = (Pixel){.r = 0.5, .g = 0.5, .b = 0.5}};
    hittable_list_add(
        &hl, (hittable){.type = SPHERE,
                        .sphere.center = (Vec3){.x = 0, .y = -1000, .z = 0},
                        .sphere.radius = 1000,
                        .sphere.mat = (struct material*)material_ground});

    for (int i = -11; i < 11; i++) {
        for (int j = -11; j < 11; j++) {
            double choose_material = random_double();
            Vec3 center = (Vec3){.x = i + 0.9 * random_double(),
                                 .y = 0.2,
                                 .z = j + 0.9 * random_double()};

            if ((vec3_len(vec3_sub(center, (Vec3){.x = 4, .y = 0.2, .z = 0}))) >
                0.9) {

                if (choose_material < 0.8) {
                    // Diffuse
                    Pixel albedo = vec3_mul(vec3_random(), vec3_random());
                    material* sphere_material = malloc(sizeof(material));
                    *sphere_material =
                        (material){.albedo = albedo, .type = LAMBERTIAN};

                    hittable_list_add(
                        &hl,
                        (hittable){.type = SPHERE,
                                   .sphere.center = center,
                                   .sphere.radius = 0.2,
                                   .sphere.mat =
                                       (struct material*)(sphere_material)});
                } else if (choose_material < 0.95) {
                    // Metal
                    Pixel albedo = vec3_random_params(0.5, 1);
                    double fuzz = random_double_min_max(0, 0.5);
                    material* sphere_material = malloc(sizeof(material));
                    *sphere_material = (material){
                        .albedo = albedo, .fuzz = fuzz, .type = METAL};

                    hittable_list_add(
                        &hl,
                        (hittable){.type = SPHERE,
                                   .sphere.center = center,
                                   .sphere.radius = 0.2,
                                   .sphere.mat =
                                       (struct material*)(sphere_material)});
                } else {
                    // Glass
                    double refraction_index = 1.5;
                    material* sphere_material = malloc(sizeof(material));
                    *sphere_material =
                        (material){.refraction_index = refraction_index,
                                   .type = DIELECTRIC};

                    hittable_list_add(
                        &hl,
                        (hittable){.type = SPHERE,
                                   .sphere.center = center,
                                   .sphere.radius = 0.2,
                                   .sphere.mat =
                                       (struct material*)(sphere_material)});
                }
            }
        }
    }

    // Fixed 1
    material* sphere_material_fixed1 = malloc(sizeof(material));
    *sphere_material_fixed1 =
        (material){.refraction_index = 1.5, .type = DIELECTRIC};
    hittable_list_add(
        &hl,
        (hittable){.type = SPHERE,
                   .sphere.center = (Vec3){.x = 0, .y = 1, .z = 0},
                   .sphere.radius = 1.0,
                   .sphere.mat = (struct material*)sphere_material_fixed1});
    // Fixed 2
    material* sphere_material_fixed2 = malloc(sizeof(material));
    *sphere_material_fixed2 = (material){
        .albedo = (Pixel){.r = 0.4, .g = 0.2, .b = 0.1}, .type = LAMBERTIAN};
    hittable_list_add(
        &hl,
        (hittable){.type = SPHERE,
                   .sphere.center = (Vec3){.x = -4, .y = 1, .z = 0},
                   .sphere.radius = 1.0,
                   .sphere.mat = (struct material*)sphere_material_fixed2});
    // Fixed 3
    material* sphere_material_fixed3 = malloc(sizeof(material));
    *sphere_material_fixed3 =
        (material){.albedo = (Pixel){.r = 0.7, .g = 0.6, .b = 0.5},
                   .fuzz = 0,
                   .type = METAL};
    hittable_list_add(
        &hl,
        (hittable){.type = SPHERE,
                   .sphere.center = (Vec3){.x = 4, .y = 1, .z = 0},
                   .sphere.radius = 1.0,
                   .sphere.mat = (struct material*)sphere_material_fixed3});

    camera cam = camera_initialize(width, height);
    camera_render(cam, &hl, file_name);

    for (size_t i = 0; i < hl.objects.len; i++) {
        free(hl.objects.array[i].sphere.mat);
    }

    vector_hittable_free(hl.objects);
}
