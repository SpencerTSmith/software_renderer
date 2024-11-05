#include "camera.h"
#include "matrix.h"

void camera_init(camera_t *camera, vec3_t position, vec3_t up_direction, vec3_t forward_direction) {
    camera->position = position;
    camera->up_direction = up_direction;
    camera->forward_direction = forward_direction;
    camera->right_direction = vec3_cross(up_direction, forward_direction);
    camera->forward_vel = (vec3_t){0, 0, 0};
    camera->right_vel = (vec3_t){0, 0, 0};
    camera->yaw = 0.0f;
    camera->pitch = 0.0f;
}

vec3_t camera_update_target(camera_t *camera) {
    mat4_t camera_yaw_matrix = mat4_make_rotation_y(camera->yaw);
    mat4_t camera_pitch_matrix = mat4_make_rotation_x(camera->pitch);
    mat4_t rotation_matrix = mat4_mul_mat4(&camera_yaw_matrix, &camera_pitch_matrix);

    // looking forward in view space
    vec4_t view_space_target = {.x = 0.0f, .y = 0.0f, .z = 1.0f, .w = 1.0f};
    camera->forward_direction = vec4_to_vec3(mat4_mul_vec4(&rotation_matrix, view_space_target));
    vec3_normalize(&camera->forward_direction);

    camera->right_direction = vec3_cross(camera->up_direction, camera->forward_direction);
    vec3_normalize(&camera->right_direction);

    vec3_t target = vec3_add(camera->position, camera->forward_direction);
    return target;
}
