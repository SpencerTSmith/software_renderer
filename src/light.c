#include "light.h"

static float clamp_light_factor(float light_factor, float min, float max) {
    const float result = light_factor < min ? min : light_factor;
    return result > max ? max : result;
}

// Change color's brightness by factor
color_t light_apply_intensity(color_t original_color, float percentage_factor) {
    percentage_factor = clamp_light_factor(percentage_factor, 0.0f, 1.0f);
    color_t color = {0};

    color.a = original_color.a;
    color.r = original_color.r * percentage_factor;
    color.g = original_color.g * percentage_factor;
    color.b = original_color.b * percentage_factor;

    return color;
}
