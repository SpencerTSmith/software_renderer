#ifndef LIGHT_H
#define LIGHT_H

#include <stdint.h>

#include "display.h"
#include "vector.h"

typedef struct {
    vec3_t direction;
} light_t;

color_t light_apply_intensity(color_t original_color, float percentage_factor);

#endif
