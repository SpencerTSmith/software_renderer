#ifndef CLIPPING_H
#define CLIPPING_H

#include "vector.h"
enum {
	LEFT_FRUSTUM,
	RIGHT_FRUSTUM,
	TOP_FRUSTUM,
	BOTTOM_FRUSTUM,
	NEAR_FRUSTUM,
	FAR_FRUSTUM
};

typedef struct {
	vec3_t point;
	vec3_t normal;
} plane_t;
#endif
