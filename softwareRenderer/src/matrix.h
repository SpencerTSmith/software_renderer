#ifndef MATRIX_H
#define MATRIX_H

#include "vector.h"

typedef struct {
	float m[4][4];
} mat4_t;

mat4_t mat4_identity(void);

mat4_t mat4_make_scale(float sx, float sy, float sz);
mat4_t mat4_make_rotation_x(float angle);
mat4_t mat4_make_rotation_y(float angle);
mat4_t mat4_make_rotation_z(float angle);
mat4_t mat4_make_translation(float tx, float ty, float tz);

mat4_t mat4_make_look_at(vec3_t eye, vec3_t target, vec3_t up);

mat4_t mat4_make_perspective(float fov, float inv_aspect, float znear, float zfar);

vec4_t mat4_mul_vec4(const mat4_t* m, vec4_t v);
vec4_t mat4_mul_vec4_project(const mat4_t* p, vec4_t v);
mat4_t mat4_mul_mat4(const mat4_t* a, const mat4_t* b);


#endif MATRIX_H
