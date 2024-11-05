#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "texture.h"
#include "vector.h"

#include <stdint.h>
#include <stdlib.h>

// Face vertices index, clockwise
typedef struct {
    int a;
    int b;
    int c;
    tex2_t a_uv;
    tex2_t b_uv;
    tex2_t c_uv;
    color_t color; // if no texture mapping
} face_t;

// Screen space coordinates, clockwise
typedef struct {
    // access it like an array or by name
    union {
        vec4_t points[3];
        struct {
            vec4_t a_pos, b_pos, c_pos;
        };
    };
    union {
        tex2_t tex_coords[3];
        struct {
            tex2_t a_uv, b_uv, c_uv;
        };
    };
    color_t color;
    float avg_depth;
} triangle_t;

int triangle_painter_compare(const void *t1, const void *t2);

void sort_triangle_by_y(triangle_t *triangle);

void draw_filled_triangle(triangle_t triangle);

void draw_affine_textured_triangle(triangle_t triangle, const texture_t *texture);

void draw_textured_triangle(triangle_t triangle, const texture_t *texture);

#endif
