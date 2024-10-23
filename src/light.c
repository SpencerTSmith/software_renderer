#include "light.h"

static float clamp_light_factor(float light_factor, float min, float max) {
    const float result = light_factor < min ? min : light_factor;
    return result > max ? max : result;
}

// Change color's brightness by factor
uint32_t light_apply_intensity(uint32_t original_color, float percentage_factor) {
    percentage_factor = clamp_light_factor(percentage_factor, 0.0f, 1.0f);

    uint32_t a = (original_color & 0xFF000000);
    uint32_t r = (original_color & 0x00FF0000) * percentage_factor;
    uint32_t g = (original_color & 0x0000FF00) * percentage_factor;
    uint32_t b = (original_color & 0x000000FF) * percentage_factor;

    uint32_t new_color = a | (r & 0x00FF0000) | (g & 0x0000FF00) | (b & 0x000000FF);

    return new_color;
}
