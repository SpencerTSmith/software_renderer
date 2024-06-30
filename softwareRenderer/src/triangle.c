#include "triangle.h"

#include <SDL_thread.h>

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

static void vec4_swap(vec4_t* a, vec4_t* b) {
	vec4_t temp = *a;
	*a = *b;
	*b = temp;
}

static void tex2_swap(tex2_t* a, tex2_t* b) {
	tex2_t temp = *a;
	*a = *b;
	*b = temp;
}

int triangle_painter_compare(const void* t1, const void* t2) {
	float avg1 = ((triangle_t*)t1)->avg_depth;
	float avg2 = ((triangle_t*)t2)->avg_depth;
	return (avg1 > avg2) ? -1 : (avg2 > avg1);
}

// Does change the triangle, not const
void sort_triangle_by_y(triangle_t* triangle) {
	// sort vertex coordinates by "ascending" y's, remember y grows downwards in screen space
	if (triangle->points[0].y > triangle->points[1].y) {
		vec4_swap(&(triangle->points[0]), &(triangle->points[1]));
		tex2_swap(&(triangle->tex_coords[0]), &(triangle->tex_coords[1]));
	}
	if (triangle->points[1].y > triangle->points[2].y) {
		vec4_swap(&(triangle->points[1]), &(triangle->points[2]));
		tex2_swap(&(triangle->tex_coords[1]), &(triangle->tex_coords[2]));
	}
	if (triangle->points[0].y > triangle->points[1].y) { // check again, previous may still not be in order
		vec4_swap(&(triangle->points[0]), &(triangle->points[1]));
		tex2_swap(&(triangle->tex_coords[0]), &(triangle->tex_coords[1]));
	}
}

// top to flat bottom, (x0, y0) at top of triangle, other two being the bottom
static void fill_flat_bottom_triangle(const triangle_t* triangle) {
	float dx_1 = roundf(triangle->points[1].x) - roundf(triangle->points[0].x);
	float dy_1 = roundf(triangle->points[1].y) - roundf(triangle->points[0].y);
	float xstep_1 = dx_1 / dy_1; // inverse slope, for every 1 increment in y, how much to step in x?

	float dx_2 = roundf(triangle->points[2].x) - roundf(triangle->points[0].x);
	float dy_2 = roundf(triangle->points[2].y) - roundf(triangle->points[0].y);
	float xstep_2 = dx_2 / dy_2; // inverse slope, for every 1 increment in y, how much to step in x?

	float x_start = triangle->points[0].x;
	float x_end = x_start;

	int y_start = roundf(triangle->points[0].y);
	int y_end = roundf(triangle->points[1].y);

	for (int y = y_start; y <= y_end; y++) {
		draw_line(roundf(x_start), y, roundf(x_end), y, triangle->color);
		x_start += xstep_1;
		x_end += xstep_2;
	}
}

// (x0, y0) and (x1, y1) being the two top vertices, (x2, y2) being the bottom vertex
static void fill_flat_top_triangle(const triangle_t* triangle) {
	float dx_1 = roundf(triangle->points[2].x) - roundf(triangle->points[0].x);
	float dy_1 = roundf(triangle->points[2].y) - roundf(triangle->points[0].y);
	float xstep_1 = dx_1 / dy_1;

	float dx_2 = roundf(triangle->points[2].x) - roundf(triangle->points[1].x);
	float dy_2 = roundf(triangle->points[2].y) - roundf(triangle->points[1].y);
	float xstep_2 = dx_2 / dy_2;

	float x_start = triangle->points[2].x;
	float x_end = x_start;

	int y_start = roundf(triangle->points[2].y);
	int y_end = roundf(triangle->points[0].y);

	for (int y = y_start; y >= y_end; y--) {
		draw_line(roundf(x_start), y, roundf(x_end), y, triangle->color);

		x_start -= xstep_1;
		x_end -= xstep_2;
	}
}

// flat bottom flat top algorithm
void draw_filled_triangle(triangle_t triangle) {
	// already a flat bottom triangle 
	if (roundf(triangle.points[0].y) == roundf(triangle.points[2].y)) {
		fill_flat_bottom_triangle(&triangle);
		return;
	}

	// already a flat top triangle
	if (roundf(triangle.points[0].y) == roundf(triangle.points[1].y)) {
		fill_flat_top_triangle(&triangle);
		return;
	}

	// Find that midpoint to divide into two triangles
	float mx = triangle.points[0].x + (triangle.points[2].x - triangle.points[0].x)
		* (triangle.points[1].y - triangle.points[0].y) / (triangle.points[2].y - triangle.points[0].y); 
	float my = triangle.points[1].y;

	triangle_t flat_bottom = triangle;
	flat_bottom.points[2].x = mx;
	flat_bottom.points[2].y = my;

	triangle_t flat_top = triangle;
	flat_top.points[0].x = triangle.points[1].x;
	flat_top.points[0].y = triangle.points[1].y;
	flat_top.points[1].x = mx;
	flat_top.points[1].y = my;

	fill_flat_bottom_triangle(&flat_bottom);
	fill_flat_top_triangle(&flat_top);

	// Draw both at same time... small test of multithreading... after testing: this is literally slower
	//SDL_Thread* bottom_thread = SDL_CreateThread(fill_flat_bottom_triangle, "flat_bottom", &flat_bottom);
	//SDL_Thread* top_thread = SDL_CreateThread(fill_flat_top_triangle, "flat_top", &flat_top);
	//SDL_WaitThread(bottom_thread, NULL);
	//SDL_WaitThread(top_thread, NULL);
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
	//sort_uv_and_coords_by_y(&x0, &y0, &u0, &v0, &x1, &y1, &u1, &v1, &x2, &y2, & u2, &v2);

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

typedef struct {
	triangle_t triangle;
	uint32_t* texture;
} tex_thread_data;

static void texture_flat_bottom_triangle(const tex_thread_data* data) {
	const triangle_t* triangle = &(data->triangle);
	const uint32_t* texture = data->texture;

	vec4_t a = triangle->points[0];
	vec4_t b = triangle->points[1];
	vec4_t c = triangle->points[2];

	tex2_t a_uv = triangle->tex_coords[0];
	tex2_t b_uv = triangle->tex_coords[1];
	tex2_t c_uv = triangle->tex_coords[2];

	float dx_1 = roundf(b.x) - roundf(a.x);
	float dy_1 = roundf(b.y) - roundf(a.y);
	float xstep_1 = dx_1 / dy_1; // inverse slope, for every 1 increment in y, how much to step in x?

	float dx_2 = roundf(c.x) - roundf(a.x);
	float dy_2 = roundf(c.y) - roundf(a.y);
	float xstep_2 = dx_2 / dy_2; // inverse slope, for every 1 increment in y, how much to step in x?

	float x_start = roundf(a.x);
	float x_end = x_start;
	for (int y = roundf(a.y); y <= roundf(b.y); y++) {
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

static void texture_flat_top_triangle(const tex_thread_data* data) {
	const triangle_t* triangle = &(data->triangle);
	const uint32_t* texture = data->texture;

	vec4_t a = triangle->points[0]; 
	vec4_t b = triangle->points[1];
	vec4_t c = triangle->points[2];

	tex2_t a_uv = triangle->tex_coords[0];
	tex2_t b_uv = triangle->tex_coords[1];
	tex2_t c_uv = triangle->tex_coords[2];

	float dx_1 = roundf(c.x) - roundf(b.x);
	float dy_1 = roundf(c.y) - roundf(b.y);
	float xstep_1 = dx_1 / dy_1;

	float dx_2 = roundf(c.x) - roundf(a.x);
	float dy_2 = roundf(c.y) - roundf(a.y);
	float xstep_2 = dx_2 / dy_2;

	float x_start = roundf(c.x);
	float x_end = x_start;
	for (int y = roundf(c.y); y >= roundf(b.y); y--) {
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

void draw_textured_triangle(triangle_t triangle, uint32_t* texture) {
	// already flat bottom
	tex_thread_data args = { .triangle = triangle, .texture = texture };
	if (roundf(triangle.points[1].y) == roundf(triangle.points[2].y)) {
		texture_flat_bottom_triangle(&args);
		return;
	}

	// already flat top
	if (roundf(triangle.points[0].y) == roundf(triangle.points[1].y)) {
		texture_flat_top_triangle(&args);
		return;
	}

	texture_flat_bottom_triangle(&args);
	texture_flat_top_triangle(&args);

	//SDL_Thread* bottom_thread = SDL_CreateThread(texture_flat_bottom_triangle, "flat_bottom_thread", &args);
	//SDL_Thread* top_thread = SDL_CreateThread(texture_flat_top_triangle, "flat_top_thread", &args);
	//SDL_WaitThread(bottom_thread, NULL);
	//SDL_WaitThread(top_thread, NULL);
}
