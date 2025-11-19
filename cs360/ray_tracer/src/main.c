#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "plugin.h"
#include "rtweekend.h"
#include "vec3.h"
#include "writer.h"
#include <ctype.h>
#include <dlfcn.h>
#include <getopt.h>
#include <libgen.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

VECTOR_PROTOTYPE(Material)
VECTOR(Material)

int main(int argc, char* argv[]) {
    char* file_format = NULL;
    uint32_t width = 320;
    uint32_t height = 240;
    uint32_t seed = time(NULL);
    uint32_t threads = 1;
    char* file_name = argv[argc - 1];
    void* dl_handle = NULL;
    int (*write)(const char* file, uint32_t width, uint32_t height,
                 const Pixel pixels[]) = write_prop;

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
            if (width < 50 || width > 9000) {
                fprintf(stderr, "error: width must be between 50 and 9000\n");
                return 1;
            }
            break;
        case 'h':
            sscanf(optarg, "%u", &height);
            if (height < 50 || height > 9000) {
                fprintf(stderr, "error: height must be between 50 and 9000\n");
                return 1;
            }
            break;
        case 's':
            sscanf(optarg, "%u", &seed);
            if (seed > 10000) {
                fprintf(stderr, "error: seed must be between 0 and 10000\n");
                return 1;
            }
            break;
        case 't':
            sscanf(optarg, "%u", &threads);
            if (threads < 1 || threads > 400) {
                fprintf(stderr,
                        "error: num threads must be between 1 and 400\n");
                return 1;
            }
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

    if (strncmp("rto", file_extension, 3)) {
        // file extension is not equal to rto
        // We look in the cwd for a shared object called lib%file_extension%.so
        char pathname[PATH_MAX] = {0};
        snprintf(pathname, sizeof(pathname), "./lib%s.so", file_extension);
        dl_handle = dlopen(pathname, RTLD_LAZY);
        if (!dl_handle) {
            fprintf(stderr, "%s\n", dlerror());
            return 1;
        }

        struct Export* e = (struct Export*)dlsym(dl_handle, "export");

        write = e->write;

        if (!write) {
            fprintf(stderr, "%s\n", dlerror());
        }
    }
    srand(seed);

    HittableList hl = {.objects = vector_Hittable_init(5)};

    Material* material_ground = malloc(sizeof(Material));
    if (!material_ground) {
        perror("malloc");
        return 1;
    }
    *material_ground =
        (Material){.type = LAMBERTIAN,
                   .albedo = (Pixel){.r = 0.5, .g = 0.5, .b = 0.5},
                   .fuzz = 0};
    hittable_list_add(&hl,
                      (Hittable){.type = SPHERE,
                                 .center = (Vec3){.x = 0, .y = -1000, .z = 0},
                                 .radius = 1000,
                                 .mat = (struct Material*)material_ground});

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
                    Material* sphere_material = malloc(sizeof(Material));
                    if (!sphere_material) {
                        perror("malloc");
                        return 1;
                    }
                    *sphere_material =
                        (Material){.albedo = albedo, .type = LAMBERTIAN};

                    hittable_list_add(
                        &hl,
                        (Hittable){.type = SPHERE,
                                   .center = center,
                                   .radius = 0.2,
                                   .mat = (struct Material*)(sphere_material)});
                } else if (choose_material < 0.95) {
                    // Metal
                    Pixel albedo = vec3_random_params(0.5, 1);
                    double fuzz = random_double_min_max(0, 0.5);
                    Material* sphere_material = malloc(sizeof(Material));
                    if (!sphere_material) {
                        perror("malloc");
                        return 1;
                    }
                    *sphere_material = (Material){
                        .albedo = albedo, .fuzz = fuzz, .type = METAL};

                    hittable_list_add(
                        &hl,
                        (Hittable){.type = SPHERE,
                                   .center = center,
                                   .radius = 0.2,
                                   .mat = (struct Material*)(sphere_material)});
                } else {
                    // Glass
                    double refraction_index = 1.5;
                    Material* sphere_material = malloc(sizeof(Material));
                    if (!sphere_material) {
                        perror("malloc");
                        return 1;
                    }
                    *sphere_material =
                        (Material){.refraction_index = refraction_index,
                                   .type = DIELECTRIC};

                    hittable_list_add(
                        &hl,
                        (Hittable){.type = SPHERE,
                                   .center = center,
                                   .radius = 0.2,
                                   .mat = (struct Material*)(sphere_material)});
                }
            }
        }
    }

    // Fixed 1
    Material* sphere_material_fixed1 = malloc(sizeof(Material));
    if (!sphere_material_fixed1) {
        perror("malloc");
        return 1;
    }
    *sphere_material_fixed1 =
        (Material){.refraction_index = 1.5, .type = DIELECTRIC};
    hittable_list_add(
        &hl, (Hittable){.type = SPHERE,
                        .center = (Vec3){.x = 0, .y = 1, .z = 0},
                        .radius = 1.0,
                        .mat = (struct Material*)sphere_material_fixed1});
    // Fixed 2
    Material* sphere_material_fixed2 = malloc(sizeof(Material));
    if (!sphere_material_fixed2) {
        perror("malloc");
        return 1;
    }
    *sphere_material_fixed2 =
        (Material){.albedo = (Pixel){.r = 0.4, .g = 0.2, .b = 0.1},
                   .type = LAMBERTIAN,
                   .fuzz = 0};
    hittable_list_add(
        &hl, (Hittable){.type = SPHERE,
                        .center = (Vec3){.x = -4, .y = 1, .z = 0},
                        .radius = 1.0,
                        .mat = (struct Material*)sphere_material_fixed2});
    // Fixed 3
    Material* sphere_material_fixed3 = malloc(sizeof(Material));
    if (!sphere_material_fixed3) {
        perror("malloc");
        return 1;
    }
    *sphere_material_fixed3 =
        (Material){.albedo = (Pixel){.r = 0.7, .g = 0.6, .b = 0.5},
                   .fuzz = 0,
                   .type = METAL};
    hittable_list_add(
        &hl, (Hittable){.type = SPHERE,
                        .center = (Vec3){.x = 4, .y = 1, .z = 0},
                        .radius = 1.0,
                        .mat = (struct Material*)sphere_material_fixed3});

    Camera cam = camera_initialize(width, height);
    camera_render(cam, &hl, file_name, write, threads);

    for (size_t i = 0; i < hl.objects.len; i++) {
        free(hl.objects.array[i].mat);
    }

    vector_Hittable_free(hl.objects);

    if (dl_handle) {
        dlclose(dl_handle);
    }
}
