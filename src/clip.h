#ifndef CLIPPING_H
#define CLIPPING_H

#include "vector.h"
enum {
	LEFT_FRUSTUM,
	RIGHT_FRUSTUM,
	TOP_FRUSTUM,
	BOTTOM_FRUSTUM,
	NEAR_FRUSTUM,
	FAR_FRUSTUM,
	NUM_PLANES
};

typedef struct {
	vec3_t point;
	vec3_t normal;
} plane_t;

void init_frustum_planes(float fov, float z_near, float z_far);

#endif
