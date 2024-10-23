#include "camera.h"

camera_t camera = {
    .position = {0, 0, 0},
    .forward_direction = {0, 0, 1},
    .right_direction = {1, 0, 0},
    .forward_vel = {0, 0, 0},
    .right_vel = {0, 0, 0},
    .yaw = 0.0f,
    .pitch = 0.0f,
};
