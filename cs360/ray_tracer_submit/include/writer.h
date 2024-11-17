#ifndef WRITER_H_
#define WRITER_H_

#include "vec3.h"
#include <stdint.h>

int write_prop(const char* file, uint32_t width, uint32_t height,
               const Pixel pixels[]);

#endif // WRITER_H_
