#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "vector.h"
#include "texture.h"

#include <stdlib.h>
#include <stdint.h>

// Face vertices index, clockwise
typedef struct {
	int a;
	int b;
	int c;
	tex2_t a_uv;
	tex2_t b_uv;
	tex2_t c_uv;
	uint32_t color; // if no texture mapping
} face_t;

// Screen space coordinates, clockwise
typedef struct {
	vec4_t points[3];
	tex2_t tex_coords[3];
	uint32_t color;
	float avg_depth;
} triangle_t;

void sort_triangle_by_y(triangle_t* triangle);

void draw_filled_triangle(triangle_t triangle);

void draw_affine_textured_triangle(
	int x0, int y0, float u0, float v0, 
	int x1, int y1, float u1, float v1, 
	int x2, int y2, float u2, float v2, 
	uint32_t* texture);

void draw_textured_triangle(triangle_t triangle, uint32_t* texture);

#endif