#pragma once

#include <stdint.h>
#include <stdio.h>

typedef struct PackedColor {
  uint8_t r : 3;
  uint8_t g : 3;
  uint8_t b : 2;
} PackedColor_t;

typedef struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} Color_t;

// `color_from_packed` unpacks a provided `PackedColor_t` into a `Color_t`
Color_t color_from_packed(PackedColor_t p);
Color_t color_from_intensity(float r, float g, float b);

// `packed_from_rgb` takes `r` `g` and `b` as `uint8_t` values
// returning an 8-bit packed `Color_t`
PackedColor_t packed_from_rgb(uint8_t r, uint8_t g, uint8_t b);

// `packed_from_color` packs a provided `Color_t` value
PackedColor_t packed_from_color(Color_t c);

// `packed_from_intensity` takes `r` `g` and `b` as `float` values
// between 0.0 and 1.0 (clamping if needed) returning an 8-bit packed `Color_t`
PackedColor_t packed_from_intensity(float r, float g, float b);

PackedColor_t packed_from_u8(uint8_t idx);

size_t packed_index(PackedColor_t pc);
