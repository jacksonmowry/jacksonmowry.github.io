#include "color.h"
#include "image.h"
#include "interval.h"
#include "plugin.h"
#include "vec3.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int write_sixel(const char* file, uint32_t width, uint32_t height,
                const Pixel pixels[]) {
    FILE* fp = stdout;
    if (file) {
        fp = fopen(file, "w");
        if (!fp) {
            perror(file);
            exit(1);
        }
    }

    fprintf(fp, "\033[H\n");
    fprintf(fp, "\033Pq\n");

    color_register_setup(fp);

    Image_t img = (Image_t){.grid = malloc(width * height * sizeof(*img.grid)),
                            .used = malloc(256 * sizeof(bool)),
                            .grid_height = height,
                            .grid_width = width};

    size_t bytes_written = 0;
    interval i = (interval){.min = 0.000, .max = 0.999};

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

            image_add_pixel(&img, (Color_t){.r = r, .g = g, .b = b}, row, col);
            bytes_written += 1;
        }
    }

    print_sixel(fp, &img);
    color_register_teardown(fp);

    puts("");
    printf("\033\\\n\n");

    if (file) {
        fclose(fp);
    }

    return bytes_written;
}

struct Export export = (struct Export){write_sixel};
