#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "ray.h"
#include "vec3.h"
#include "writer.h"
#include <ctype.h>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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

    // World building
    hittable_list hl = {.objects = vector_hittable_init(2)};
    hittable_list_add(
        &hl, (hittable){.type = SPHERE,
                        .sphere.center = (Vec3){.x = 0, .y = 0, .z = -1},
                        .sphere.radius = 0.5});
    hittable_list_add(
        &hl, (hittable){.type = SPHERE,
                        .sphere.center = (Vec3){.x = 0, .y = -100.5, .z = -1},
                        .sphere.radius = 100});

    camera cam = camera_initialize(width, height);
    camera_render(cam, &hl);

    vector_hittable_free(hl.objects);
}
