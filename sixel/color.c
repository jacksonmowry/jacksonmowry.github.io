#include "color.h"
#include "utils.h"

Color_t color_from_packed(PackedColor_t p) {
  return (Color_t){.r = p.r * 32, .g = p.g * 32, .b = p.b * 64};
}

Color_t color_from_intensity(float r, float g, float b) {
  return (Color_t){.r = CLAMP(r, 0.0, 1.0) * 256,
                   .g = CLAMP(g, 0.0, 1.0) * 256,
                   .b = CLAMP(b, 0.0, 1.0) * 256};
}

PackedColor_t color_from_rgb(uint8_t r, uint8_t g, uint8_t b) {
  return (PackedColor_t){.r = r / 32, .g = g / 32, .b = b / 64};
}

PackedColor_t packed_from_color(Color_t c) {
  return color_from_rgb(c.r, c.g, c.b);
}

PackedColor_t packed_from_intensity(float r, float g, float b) {
  return (PackedColor_t){.r = CLAMP(r, 0.0, 1.0) * 8,
                         .g = CLAMP(g, 0.0, 1.0) * 8,
                         .b = CLAMP(b, 0.0, 1.0) * 4};
}

PackedColor_t packed_from_u8(uint8_t idx) {
  return (PackedColor_t){.r = (idx & 0b11100000) >> 5,
                         .g = (idx & 0b00011100) >> 2,
                         .b = idx & 0b00000011};
}

size_t packed_index(PackedColor_t pc) {
  return (pc.r << 5) + (pc.g << 2) + pc.b;
}
