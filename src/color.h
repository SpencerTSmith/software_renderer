#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

// 0xAABBGGRR
typedef union {
    uint32_t argb;
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };

} color_t;

#define WHITE (color_t)0xFFFFFFFF
#define RED (color_t)0xFF0000FF
#define GREEN (color_t)0xFF00FF00
#define BLUE (color_t)0xFFFF0000
#define PURPLE (color_t)0xFFFF00FF
#define BLACK (color_t)0xFF000000

#endif
