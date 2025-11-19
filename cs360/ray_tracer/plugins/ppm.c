#include "interval.h"
#include "plugin.h"
#include "vec3.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
    Interval i = (Interval){.min = 0.000, .max = 0.999};

    for (int64_t row = 0; row < height; row++) {
        for (size_t col = 0; col < width; col++) {
            int r = interval_clamp(
                        i, linear_to_gamma(pixels[(row * width) + col].r)) *
                    255.999;
            int g = interval_clamp(
                        i, linear_to_gamma(pixels[(row * width) + col].g)) *
                    255.999;
            int b = interval_clamp(
                        i, linear_to_gamma(pixels[(row * width) + col].b)) *
                    255.999;
            bytes_written += fprintf(fp, "%u %u %u\n", r, g, b);
        }
    }

    if (file) {
        fclose(fp);
    }

    return bytes_written;
}

struct Export export = (struct Export){write_ppm};
