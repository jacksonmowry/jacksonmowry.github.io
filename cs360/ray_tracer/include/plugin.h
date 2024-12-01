#pragma once

#include "vec3.h"
#include <stdint.h>

struct Export {
    int (*write)(const char* file, uint32_t width, uint32_t height,
                 const Pixel pixels[]);
};
