#include "vec3.h"
#include <ctype.h>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef Vec3 Pixel;

typedef struct {
    Vec3 origin;
    Vec3 direction;
} Ray;

double vec3_len2(Vec3 v) { return v.x * v.x + v.y * v.y + v.z * v.z; }

double vec3_len(Vec3 v) { return sqrt(vec3_len2(v)); }

Vec3 vec3_add(Vec3 a, Vec3 b) {
    return (Vec3){.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z};
}

Vec3 vec3_sub(Vec3 a, Vec3 b) {
    return (Vec3){.x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z};
}

Vec3 vec3_mul(Vec3 a, Vec3 b) {
    return (Vec3){.x = a.x * b.x, .y = a.y * b.y, .z = a.z * b.z};
}

Vec3 vec3_mul_dbl(Vec3 a, double b) {
    return (Vec3){.x = a.x * b, .y = a.y * b, .z = a.z * b};
}

Vec3 vec3_div(Vec3 a, double t) {
    return (Vec3){.x = a.x / t, .y = a.y / t, .z = a.z / t};
}

double vec3_dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

Vec3 unit_vector(Vec3 src) {
    double len = vec3_len(src);
    return vec3(src.x / len, src.y / len, src.z / len);
}

Vec3 vec3_mul_val(Vec3 left, double val) {
    return (Vec3){.x = left.x * val, .y = left.y * val, .z = left.z * val};
}

static bool hit_sphere(Vec3 center, double radius, Ray r) {
    Vec3 oc = vec3_sub(r.origin, center);
    double a = vec3_dot(r.direction, r.direction);
    double b = 2.0 * vec3_dot(oc, r.direction);
    double c = vec3_dot(oc, oc) - radius * radius;
    double d = b * b - 4 * a * c;
    return d >= 0;
}

Pixel ray_color(Ray r) {
    if (hit_sphere((Vec3){.x = 0, .y = 0, .z = -1}, 0.5, r)) {
        return (Pixel){.x = 1, .y = 0, .z = 0};
    }

    Vec3 unit_direction = unit_vector(r.direction);
    double a = 0.5 * (unit_direction.y + 1.0);
    return vec3_add(vec3_mul_val(vec3(1.0, 1.0, 1.0), 1.0 - a),
                    vec3_mul_val(vec3(0.5, 0.7, 1.0), a));
}

Vec3 ray_at(Ray r, double t) {
    return vec3_add(r.origin, vec3_mul(r.direction, vec3(t, t, t)));
}

/* int  (*write)(const char *file, u32 width, u32 height, const Pixel pixels[]);
 */
// proprietary writer
int write_prop(const char* file, uint32_t width, uint32_t height,
               const Pixel pixels[]) {
    FILE* fp = stdout;
    if (file) {
        fp = fopen(file, "w");
        if (!fp) {
            perror(file);
            fclose(fp);
            exit(1);
        }
    }

    fwrite(&width, 2, 1, fp);
    fwrite(&height, 2, 1, fp);

    size_t bytes_written = 0;

    /* return fwrite(pixels, 3, width * height, fp); */

    for (size_t i = 0; i < width * height; i++) {
        int r = pixels[i].r * 255.999;
        int g = pixels[i].g * 255.999;
        int b = pixels[i].b * 255.999;
        bytes_written += fwrite(&r, 1, 1, fp);
        bytes_written += fwrite(&g, 1, 1, fp);
        bytes_written += fwrite(&b, 1, 1, fp);
    }

    if (fp != stdout) {
        fclose(fp);
    }
    return bytes_written;
}

int write_ppm(const char* file, uint32_t width, uint32_t height,
              const Pixel pixels[]) {
    FILE* fp = stdout;
    if (file) {
        fp = fopen(file, "w");
        if (!fp) {
            perror(file);
            exit(1);
        }
    }

    fprintf(fp, "P3\n");
    fprintf(fp, "%u %u\n", width, height);
    fprintf(fp, "255\n");

    size_t bytes_written = 0;

    for (size_t row = 0; row < height; row++) {
        /* fprintf(stderr, "Rows remaining: %lu\n", height - row); */
        for (size_t col = 0; col < width; col++) {
            int r = pixels[(row * width) + col].r * 255.999;
            int g = pixels[(row * width) + col].g * 255.999;
            int b = pixels[(row * width) + col].b * 255.999;
            bytes_written += fprintf(fp, "%u %u %u\n", r, g, b);
        }
    }
    return bytes_written;
}

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

    double aspect_ratio = (double)width / (double)height;

    double focal_length = 1.0;
    double viewport_height = 2.0;
    double viewport_width = viewport_height * aspect_ratio;
    fprintf(stderr, "viewport width: %f\n", viewport_width);
    Vec3 camera_center = (Vec3){.x = 0, .y = 0, .z = 0};

    // Calculating the vectors for outer edges
    Vec3 viewport_u = (Vec3){.x = viewport_width, .y = 0, .z = 0}; // top
    Vec3 viewport_v =
        (Vec3){.x = 0, .y = -viewport_height, .z = 0}; // left side

    // Calcuate the horizontal and vertical delta vectors for pixel spacing
    Vec3 pixel_delta_u = vec3_div(viewport_u, width);
    Vec3 pixel_delta_v = vec3_div(viewport_v, height);

    // Pos of first pixel (upper left)
    Vec3 viewport_upper_left =
        vec3_sub(vec3_sub(vec3_sub(camera_center,
                                   (Vec3){.x = 0, .y = 0, .z = focal_length}),
                          vec3_div(viewport_u, 2)),
                 vec3_div(viewport_v, 2));
    fprintf(stderr, "viewport upper left: %f %f %f\n", viewport_upper_left.x,
            viewport_upper_left.y, viewport_upper_left.z);
    Vec3 pixel00_location =
        vec3_add(viewport_upper_left,
                 vec3_mul_dbl(vec3_add(pixel_delta_u, pixel_delta_v), 0.5));
    fprintf(stderr, "pixel 00: %f %f %f\n", pixel00_location.x,
            pixel00_location.y, pixel00_location.z);

    Pixel* p = malloc(width * height * sizeof(Pixel));

    for (uint32_t row = 0; row < height; row++) {
        for (uint32_t col = 0; col < width; col++) {
            Vec3 pixel_center = vec3_add(
                pixel00_location, vec3_add(vec3_mul_dbl(pixel_delta_u, row),
                                           vec3_mul_dbl(pixel_delta_v, col)));
            Vec3 ray_direction = vec3_sub(pixel_center, camera_center);
            Ray r = (Ray){.origin = camera_center, .direction = ray_direction};
            Pixel color = ray_color(r);

            p[(row * width) + col] = color;
        }
    }

    /* write_prop(NULL, 16, 16, p); */
    write_ppm(NULL, width, height, p);
    free(p);
}
