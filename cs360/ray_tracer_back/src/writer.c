#include "writer.h"
#include "interval.h"
#include <stdio.h>
#include <stdlib.h>

// proprietary writer
int write_prop(const char* file, uint32_t width, uint32_t height,
               const Pixel pixels[]) {
    FILE* fp = stdout;
    if (file) {
        fp = fopen(file, "w");
        if (!fp) {
            perror(file);
            exit(1);
        }
    }

    fwrite(&width, 2, 1, fp);
    fwrite(&height, 2, 1, fp);

    size_t bytes_written = 0;
    interval inter_val = (interval){.min = 0.000, .max = 0.999};

    for (size_t i = 0; i < width * height; i++) {
        int r =
            interval_clamp(inter_val, linear_to_gamma(pixels[i].r)) * 255.999;
        int g =
            interval_clamp(inter_val, linear_to_gamma(pixels[i].g)) * 255.999;
        int b =
            interval_clamp(inter_val, linear_to_gamma(pixels[i].b)) * 255.999;
        bytes_written += fwrite(&r, 1, 1, fp);
        bytes_written += fwrite(&g, 1, 1, fp);
        bytes_written += fwrite(&b, 1, 1, fp);
    }

    if (file) {
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
    interval i = (interval){.min = 0.000, .max = 0.999};

    for (int64_t row = 0; row < height; row++) {
        fprintf(stderr, "Rows remaining: %lu\n", height - row);
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
