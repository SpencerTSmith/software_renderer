#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>

#include "display.h"
#include "vector.h"

typedef struct {
    float u;
    float v;
} tex2_t;

extern int texture_width;
extern int texture_height;

extern color_t *mesh_texture;

void texture_free(color_t *texture);

void load_redbrick_mesh_texture();

void load_png_texture_data(const char *filename);

// Return barycentric weights of vertices alpha, beta, gamma with respect to
// point p
vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p);

// Draw texel based not only on xy coords of point, but also it's interpolated
// uv coords from triangle vertice info We pass in x and y as int so that the we
// draw at the right discrete pixel, everything else is calculated using
// floating point
void draw_affine_texel(int x, int y, vec2_t a, vec2_t b, vec2_t c, tex2_t a_uv, tex2_t b_uv,
                       tex2_t c_uv, color_t *texture);

// Draw texel same as above but passing in extra information for perspective
// correct interpolation
void draw_texel(int x, int y, vec4_t a, vec4_t b, vec4_t c, tex2_t a_uv, tex2_t b_uv, tex2_t c_uv,
                color_t *texture);

extern const uint8_t REDBRICK_TEXTURE[];
#endif
