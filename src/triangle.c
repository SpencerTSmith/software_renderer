#include "triangle.h"

#include "display.h"
#include "texture.h"

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

static void draw_w_pixel(int x, int y, vec4_t a, vec4_t b, vec4_t c, uint32_t color) {
	vec2_t p = { x, y };
	vec2_t a_2 = vec4_to_vec2(a);
	vec2_t b_2 = vec4_to_vec2(b);
	vec2_t c_2 = vec4_to_vec2(c);

	vec3_t weights = barycentric_weights(a_2, b_2, c_2, p);
	float alpha = weights.x;
	float beta = weights.y;
	float gamma = weights.z;

	float interp_inv_w = (1 / a.w) * alpha + (1 / b.w) * beta + (1 / c.w) * gamma;

	if (interp_inv_w > w_buffer[window_width * y + x]) {
		draw_pixel(x, y, color);
		w_buffer[window_width * y + x] = interp_inv_w;
	}
}

static void fill_flat_bottom_triangle(const triangle_t* triangle) {
	vec4_t a = triangle->points[0];
	vec4_t b = triangle->points[1];
	vec4_t c = triangle->points[2];

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
				draw_w_pixel(x, y, a, b, c, triangle->color);
		}

		x_start += xstep_1;
		x_end += xstep_2;
	}
}

static void fill_flat_top_triangle(const triangle_t* triangle) {
	vec4_t a = triangle->points[0];
	vec4_t b = triangle->points[1];
	vec4_t c = triangle->points[2];

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
			draw_w_pixel(x, y, a, b, c, triangle->color);
		}

		x_start -= xstep_1;
		x_end -= xstep_2;
	}
}

// flat bottom flat top algorithm
void draw_filled_triangle(triangle_t triangle) {
	// already flat bottom
	if (roundf(triangle.points[1].y) == roundf(triangle.points[2].y)) {
		fill_flat_bottom_triangle(&triangle);
		return;
	}

	// already flat top
	if (roundf(triangle.points[0].y) == roundf(triangle.points[1].y)) {
		fill_flat_top_triangle(&triangle);
		return;
	}

	fill_flat_bottom_triangle(&triangle);
	fill_flat_top_triangle(&triangle);
}

static void affine_texture_flat_bottom_triangle(const triangle_t* triangle, uint32_t* texture) {
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
				draw_affine_texel(x, y, vec4_to_vec2(a), vec4_to_vec2(b), vec4_to_vec2(c), a_uv, b_uv, c_uv, texture);
			}
		}

		x_start += xstep_1;
		x_end += xstep_2;
	}
}

static void affine_texture_flat_top_triangle(const triangle_t* triangle, uint32_t* texture) {
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
				draw_affine_texel(x, y, vec4_to_vec2(a), vec4_to_vec2(b), vec4_to_vec2(c), a_uv, b_uv, c_uv, texture);
			}
		}

		x_start -= xstep_1;
		x_end -= xstep_2;
	}
}

void draw_affine_textured_triangle(triangle_t triangle, uint32_t* texture) {
	// already flat bottom
	if (roundf(triangle.points[1].y) == roundf(triangle.points[2].y)) {
		affine_texture_flat_bottom_triangle(&triangle, texture);
		return;
	}

	// already flat top
	if (roundf(triangle.points[0].y) == roundf(triangle.points[1].y)) {
		affine_texture_flat_top_triangle(&triangle, texture);
		return;
	}

	affine_texture_flat_bottom_triangle(&triangle, texture);
	affine_texture_flat_top_triangle(&triangle, texture);
}

static void texture_flat_bottom_triangle(const triangle_t* triangle, uint32_t* texture) {
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

static void texture_flat_top_triangle(const triangle_t* triangle, uint32_t* texture) {
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
	if (roundf(triangle.points[1].y) == roundf(triangle.points[2].y)) {
		texture_flat_bottom_triangle(&triangle, texture);
		return;
	}

	// already flat top
	if (roundf(triangle.points[0].y) == roundf(triangle.points[1].y)) {
		texture_flat_top_triangle(&triangle, texture);
		return;
	}

	texture_flat_bottom_triangle(&triangle, texture);
	texture_flat_top_triangle(&triangle, texture);
}
