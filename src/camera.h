#ifndef CAMERA_H
#define CAMERA_H

#include "vector.h"

typedef struct {
    vec3_t position, up_direction, forward_direction, right_direction, forward_vel, right_vel;
    float yaw, pitch;
} camera_t;

// Initialize the camera with position, and direction it is looking at
void camera_init(camera_t *camera, vec3_t position, vec3_t up_direction, vec3_t forward_direction);

// Updates camera and returns the target of camera
vec3_t camera_update_target(camera_t *camera);

#endif
