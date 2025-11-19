#include "interval.h"
#include "plugin.h"
#include "vec3.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#pragma pack(push, 2)
typedef struct BFH {
    char type[2];
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} BFH;
#pragma pop

#pragma pack(push, 2)
typedef struct BIH {
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bitcount;
    uint32_t compression;
    uint32_t imagesize;
    uint32_t x_ppm;
    uint32_t y_ppm;
    uint32_t color_used;
    uint32_t color_important;
} BIH;
#pragma pop

int write_bmp(const char* file, uint32_t width, uint32_t height,
              const Pixel pixels[]) {
    const size_t row_padding_needed = (4 - ((width * 3) % 4)) % 4;
    const uint8_t zero_byte = 0;

    BFH header =
        (BFH){.type = {'B', 'M'},
              .size = sizeof(BFH) + sizeof(BIH) + (width * height * 3) +
                      (height * row_padding_needed),
              .reserved1 = 0,
              .reserved2 = 0,
              .offset = sizeof(BFH) + sizeof(BIH)};
    BIH information = (BIH){.size = 40,
                            .width = width,
                            .height = height,
                            .planes = 1,
                            .bitcount = 24,
                            .compression = 0,
                            .imagesize = width * height,
                            .x_ppm = 0,
                            .y_ppm = 0,
                            .color_used = 0,
                            .color_important = 0};

    FILE* fp = stdout;
    if (file) {
        fp = fopen(file, "w");
        if (!fp) {
            perror(file);
            exit(1);
        }
    }

    fwrite(&header, sizeof(header), 1, fp);
    fwrite(&information, sizeof(information), 1, fp);

    size_t bytes_written = 0;
    Interval i = (Interval){.min = 0.000, .max = 0.999};

    for (int64_t row = 0; row < height; row++) {
        for (size_t col = 0; col < width; col++) {
            uint8_t r =
                interval_clamp(
                    i, linear_to_gamma(
                           pixels[((height - row - 1) * width) + col].r)) *
                255.999;
            uint8_t g =
                interval_clamp(
                    i, linear_to_gamma(
                           pixels[((height - row - 1) * width) + col].g)) *
                255.999;
            uint8_t b =
                interval_clamp(
                    i, linear_to_gamma(
                           pixels[((height - row - 1) * width) + col].b)) *
                255.999;
            fwrite(&b, sizeof(b), 1, fp);
            fwrite(&g, sizeof(g), 1, fp);
            fwrite(&r, sizeof(r), 1, fp);
            for (int i = 0; i < row_padding_needed; i++) {
                fwrite(&zero_byte, sizeof(zero_byte), 1, fp);
            }
        }
    }

    if (file) {
        fclose(fp);
    }

    return bytes_written;
}

struct Export export = (struct Export){write_bmp};
