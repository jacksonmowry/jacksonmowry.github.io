#include "writer.h"
#include "interval.h"
#include <stdio.h>
#include <stdlib.h>

// proprietary writer, outputs described .rto format
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
    Interval inter_val = (Interval){.min = 0.000, .max = 0.999};

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
