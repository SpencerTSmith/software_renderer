#include "triangle.h"
#include "display.h"
#include "texture.h"

// utility for sorting
static void int_swap(int* a, int* b) {
	int temp = *a;
	*a = *b;
	*b = temp;
}

// utility for sorting
static void float_swap(float* a, float* b) {
	float temp = *a;
	*a = *b;
	*b = temp;
}

// ascending y's
static void sort_coords_by_y(int* x0, int* y0, int* x1, int* y1, int* x2, int* y2) {
	// sort vertex coordinates by "ascending" y's, remember y grows downwards in screen space
	if (*y0 > *y1) {
		int_swap(y0, y1);
		int_swap(x0, x1);
	}
	if (*y1 > *y2) {
		int_swap(y1, y2);
		int_swap(x1, x2);
	}
	if (*y0 > *y1) { // check again, previous may still not be in order
		int_swap(y0, y1);
		int_swap(x0, x1);
	}
}

// for affine textured triangles, also switch the associated uv coords
static void sort_uv_and_coords_by_y(
	int* x0, int* y0, float* u0, float* v0, 
	int* x1, int* y1, float* u1, float* v1, 
	int* x2, int* y2, float* u2, float* v2) {
	// sort vertex coordinates by "ascending" y's, remember y grows downwards in screen space
	if (*y0 > *y1) {
		int_swap(y0, y1);
		int_swap(x0, x1);
		float_swap(u0, u1);
		float_swap(v0, v1);
	}
	if (*y1 > *y2) {
		int_swap(y1, y2);
		int_swap(x1, x2);
		float_swap(u1, u2);
		float_swap(v1, v2);
	}
	if (*y0 > *y1) { // check again, previous may still not be in order
		int_swap(y0, y1);
		int_swap(x0, x1);
		float_swap(u0, u1);
		float_swap(v0, v1);
	}
}

// for perspective correct texture triangles
static void sort_all_by_y(
	int* x0, int* y0, float* z0, float* w0, float* u0, float* v0, 
	int* x1, int* y1, float* z1, float* w1, float* u1, float* v1, 
	int* x2, int* y2, float* z2, float* w2, float* u2, float* v2) {
	// sort vertex coordinates by "ascending" y's, remember y grows downwards in screen space
	if (*y0 > *y1) {
		int_swap(y0, y1);
		int_swap(x0, x1);
		float_swap(z0, z1);
		float_swap(w0, w1);
		float_swap(u0, u1);
		float_swap(v0, v1);
	}
	if (*y1 > *y2) {
		int_swap(y1, y2);
		int_swap(x1, x2);
		float_swap(z1, z2);
		float_swap(w1, w2);
		float_swap(u1, u2);
		float_swap(v1, v2);
	}
	if (*y0 > *y1) { // check again, previous may still not be in order
		int_swap(y0, y1);
		int_swap(x0, x1);
		float_swap(z0, z1);
		float_swap(w0, w1);
		float_swap(u0, u1);
		float_swap(v0, v1);
	}
}

// top to flat bottom, (x0, y0) at top of triangle, other two being the bottom
static void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
	int dx_1 = x1 - x0;
	int dy_1 = y1 - y0;
	float xstep_1 = (float) dx_1 / dy_1; // inverse slope, for every 1 increment in y, how much to step in x?

	int dx_2 = x2 - x0;
	int dy_2 = y2 - y0;
	float xstep_2 = (float) dx_2 / dy_2; // inverse slope, for every 1 increment in y, how much to step in x?

	float x_start = x0;
	float x_end = x0;
	for (int y = y0; y <= y1; y++) {
		draw_line((int) roundf(x_start), y, (int) roundf(x_end), y, color);
		x_start += xstep_1;
		x_end += xstep_2;
	}
}

// (x0, y0) and (x1, y1) being the two top vertices, (x2, y2) being the bottom vertex
static void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
	int dx_1 = x2 - x0;
	int dy_1 = y2 - y0;
	float xstep_1 = (float) dx_1 / dy_1;

	int dx_2 = x2 - x1;
	int dy_2 = y2 - y1;
	float xstep_2 = (float) dx_2 / dy_2;

	float x_start = x2;
	float x_end = x2;
	for (int y = y2; y >= y0; y--) {
		draw_line((int) roundf(x_start), y, (int) roundf(x_end), y, color);
		x_start -= xstep_1;
		x_end -= xstep_2;
	}
}

// flat bottom flat top algorithm
void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
	sort_coords_by_y(&x0, &y0, &x1, &y1, &x2, &y2);

	// already a flat bottom triangle 
	if (y1 == y2) { 
		fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);
		return;
	}

	// already a flat top triangle
	if (y0 == y1) {
		fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);
		return;
	}

	// Find that midpoint to divide into two triangles
	int mx = roundf(x0 + (float) (x2 - x0) * (y1 - y0) / (y2 - y0)); // want float division
	int my = y1;

	fill_flat_bottom_triangle(x0, y0, x1, y1, mx, my, color);
	fill_flat_top_triangle(x1, y1, mx, my, x2, y2, color);
}

static void affine_texture_flat_bottom_triangle(
	int x0, int y0, float u0, float v0, 
	int x1, int y1, float u1, float v1, 
	int x2, int y2, float u2, float v2, uint32_t* texture) {
	vec2_t a = { x0, y0 };
	vec2_t b = { x1, y1 };
	vec2_t c = { x2, y2 };

	tex2_t a_uv = { u0, v0 };
	tex2_t b_uv = { u1, v1 };
	tex2_t c_uv = { u2, v2 };

	int dx_1 = x1 - x0;
	int dy_1 = y1 - y0;
	float xstep_1 = (float) dx_1 / dy_1; // inverse slope, for every 1 increment in y, how much to step in x?

	int dx_2 = x2 - x0;
	int dy_2 = y2 - y0;
	float xstep_2 = (float) dx_2 / dy_2; // inverse slope, for every 1 increment in y, how much to step in x?
	
	float x_start = x0;
	float x_end = x0;
	for (int y = y0; y <= y1; y++) {
		// If we're rotated the other way, lets swap so we are still drawing left to right
		if (x_end < x_start) {
			float_swap(&x_start, &x_end);
			float_swap(&xstep_1, &xstep_2);
		}

		for (int x = roundf(x_start); x <= roundf(x_end); x++) {
			if (texture == NULL) {
				draw_pixel(x, y, (x % 2 && y % 2) ? 0xFFFF00FF : 0xFF000000);
			}
			else {
				draw_affine_texel(x, y, a, b, c, a_uv, b_uv, c_uv, texture);
			}
		}

		x_start += xstep_1;
		x_end += xstep_2;
	}
}

static void affine_texture_flat_top_triangle(
	int x0, int y0, float u0, float v0, 
	int x1, int y1, float u1, float v1, 
	int x2, int y2, float u2, float v2, 
	uint32_t* texture) {
	vec2_t a = { x0, y0 };
	vec2_t b = { x1, y1 };
	vec2_t c = { x2, y2 };

	tex2_t a_uv = { u0, v0 };
	tex2_t b_uv = { u1, v1 };
	tex2_t c_uv = { u2, v2 };

	int dx_1 = x2 - x1;
	int dy_1 = y2 - y1;
	float xstep_1 = (float) dx_1 / dy_1;

	int dx_2 = x2 - x0;
	int dy_2 = y2 - y0;
	float xstep_2 = (float) dx_2 / dy_2;

	float x_start = x2;
	float x_end = x2;
	for (int y = y2; y >= y1; y--) {
		// If we're rotated the other way, lets swap so we are still drawing left to right
		if (x_end < x_start) {
			float_swap(&x_start, &x_end);
			float_swap(&xstep_1, &xstep_2);
		}

		for (int x = roundf(x_start); x <= roundf(x_end); x++) {
			if (texture == NULL) {
				draw_pixel(x, y, (x % 2 && y % 2) ? 0xFFFF00FF : 0xFF000000);
			}
			else {
				draw_affine_texel(x, y, a, b, c, a_uv, b_uv, c_uv, texture);
			}
		}

		x_start -= xstep_1;
		x_end -= xstep_2;
	}
}

void draw_affine_textured_triangle(
	int x0, int y0, float u0, float v0, 
	int x1, int y1, float u1, float v1, 
	int x2, int y2, float u2, float v2, 
	uint32_t* texture) {
	sort_uv_and_coords_by_y(&x0, &y0, &u0, &v0, &x1, &y1, &u1, &v1, &x2, &y2, & u2, &v2);

	// already flat bottom
	if (y1 == y2) {
		affine_texture_flat_bottom_triangle(x0, y0, u0, v0, x1, y1, u1, v1, x2, y2, u2, v2, texture);
		return;
	}

	// already flat top
	if (y0 == y1) {
		affine_texture_flat_top_triangle(x0, y0, u0, v0, x1, y1, u1, v1, x2, y2, u2, v2, texture);
		return;
	}

	affine_texture_flat_bottom_triangle(x0, y0, u0, v0, x1, y1, u1, v1, x2, y2, u2, v2, texture);
	affine_texture_flat_top_triangle(x0, y0, u0, v0, x1, y1, u1, v1, x2, y2, u2, v2, texture);
}

static void texture_flat_bottom_triangle(
	int x0, int y0, float z0, float w0, float u0, float v0,
	int x1, int y1, float z1, float w1, float u1, float v1,
	int x2, int y2, float z2, float w2, float u2, float v2,
	uint32_t* texture) {
	vec4_t a = { x0, y0, z0, w0, };
	vec4_t b = { x1, y1, z1, w1, };
	vec4_t c = { x2, y2, z2, w2 };

	tex2_t a_uv = { u0, v0 };
	tex2_t b_uv = { u1, v1 };
	tex2_t c_uv = { u2, v2 };

	int dx_1 = x1 - x0;
	int dy_1 = y1 - y0;
	float xstep_1 = (float)dx_1 / dy_1; // inverse slope, for every 1 increment in y, how much to step in x?

	int dx_2 = x2 - x0;
	int dy_2 = y2 - y0;
	float xstep_2 = (float)dx_2 / dy_2; // inverse slope, for every 1 increment in y, how much to step in x?

	float x_start = x0;
	float x_end = x0;
	for (int y = y0; y <= y1; y++) {
		// If we're rotated the other way, lets swap so we are still drawing left to right
		if (x_end < x_start) {
			float_swap(&x_start, &x_end);
			float_swap(&xstep_1, &xstep_2);
		}

		for (int x = roundf(x_start); x <= roundf(x_end); x++) {
			if (texture == NULL) {
				draw_pixel(x, y, (x % 2 && y % 2) ? 0xFFFF00FF : 0xFF000000);
			}
			else {
				draw_texel(x, y, a, b, c, a_uv, b_uv, c_uv, texture);
			}
		}

		x_start += xstep_1;
		x_end += xstep_2;
	}
}

static void texture_flat_top_triangle(
	int x0, int y0, float z0, float w0, float u0, float v0,
	int x1, int y1, float z1, float w1, float u1, float v1,
	int x2, int y2, float z2, float w2, float u2, float v2,
	uint32_t* texture) {
	vec4_t a = { x0, y0, z0, w0 };
	vec4_t b = { x1, y1, z1, w1 };
	vec4_t c = { x2, y2, z2, w2 };

	tex2_t a_uv = { u0, v0 };
	tex2_t b_uv = { u1, v1 };
	tex2_t c_uv = { u2, v2 };

	int dx_1 = x2 - x1;
	int dy_1 = y2 - y1;
	float xstep_1 = (float)dx_1 / dy_1;

	int dx_2 = x2 - x0;
	int dy_2 = y2 - y0;
	float xstep_2 = (float)dx_2 / dy_2;

	float x_start = x2;
	float x_end = x2;
	for (int y = y2; y >= y1; y--) {
		// If we're rotated the other way, lets swap so we are still drawing left to right
		if (x_end < x_start) {
			float_swap(&x_start, &x_end);
			float_swap(&xstep_1, &xstep_2);
		}

		for (int x = roundf(x_start); x <= roundf(x_end); x++) {
			if (texture == NULL) {
				draw_pixel(x, y, (x % 2 && y % 2) ? 0xFFFF00FF : 0xFF000000);
			}
			else {
				draw_texel(x, y, a, b, c, a_uv, b_uv, c_uv, texture);
			}
		}

		x_start -= xstep_1;
		x_end -= xstep_2;
	}
}

void draw_textured_triangle(
	int x0, int y0, float z0, float w0, float u0, float v0, 
	int x1, int y1, float z1, float w1, float u1, float v1, 
	int x2, int y2, float z2, float w2, float u2, float v2, 
	uint32_t* texture) {
	sort_all_by_y(
		&x0, &y0, &z0, &w0, &u0, &v0,
		&x1, &y1, &z1, &w1, &u1, &v1,
		&x2, &y2, &z2, &w2, &u2, &v2);

	// already flat bottom
	if (y1 == y2) {
		texture_flat_bottom_triangle(
			x0, y0, z0, w0, u0, v0,
			x1, y1, z1, w1, u1, v1,
			x2, y2, z2, w2, u2, v2,
			texture);
		return;
	}

	// already flat top
	if (y0 == y1) {
		texture_flat_top_triangle(
			x0, y0, z0, w0, u0, v0,
			x1, y1, z1, w1, u1, v1,
			x2, y2, z2, w2, u2, v2,
			texture);
		return;
	}

	texture_flat_bottom_triangle(
		x0, y0, z0, w0, u0, v0,
		x1, y1, z1, w1, u1, v1,
		x2, y2, z2, w2, u2, v2,
		texture);
	texture_flat_top_triangle(
		x0, y0, z0, w0, u0, v0,
		x1, y1, z1, w1, u1, v1,
		x2, y2, z2, w2, u2, v2,
		texture);
}
