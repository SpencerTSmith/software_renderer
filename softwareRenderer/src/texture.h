#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>
#include "vector.h"

typedef struct {
	float u;
	float v;
} tex2_t;

extern int texture_width;
extern int texture_height;

extern uint32_t* mesh_texture;

// Return barycentric weights of vertices alpha, beta, gamma with respect to point p
vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p);

extern const uint8_t REDBRICK_TEXTURE[];
#endif