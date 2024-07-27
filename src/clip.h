#ifndef CLIPPING_H
#define CLIPPING_H

#include "vector.h"

#define MAX_NUM_POLY_VERTS 10

typedef enum {
	LEFT_FRUSTUM,
	RIGHT_FRUSTUM,
	TOP_FRUSTUM,
	BOTTOM_FRUSTUM,
	NEAR_FRUSTUM,
	FAR_FRUSTUM,
	NUM_PLANES
} frustum_side;

typedef struct {
	vec3_t point;
	vec3_t normal;
} plane_t;

typedef struct {
	vec3_t vertices[MAX_NUM_POLY_VERTS];
	int num_vertices;
} polygon_t;

void init_frustum_planes(float fov, float z_near, float z_far);

void clip_against_plane(polygon_t *polygon, const plane_t *frust_plane);
void clip_polygon(polygon_t *polygon);

#endif
