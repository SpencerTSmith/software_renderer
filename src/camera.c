#include "camera.h"

camera_t camera = {.position = {0, 0, 0},
				   .direction = {0, 0, 1},
				   .forward_vel = {0, 0, 0},
				   .yaw = 0.0f,
				   .pitch = 0.0f};
