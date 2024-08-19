#ifndef CAMERA_H
#define CAMERA_H

#include "vector.h"

typedef struct {
	vec3_t position;
	vec3_t forward_direction;
	vec3_t right_direction;
	vec3_t forward_vel;
	vec3_t right_vel;
	float yaw;
	float pitch;
} camera_t;

extern camera_t camera;

#endif
