#ifndef SCENE_H
#define SCENE_H

#include "camera.h"
#include "clip.h"
#include "light.h"
#include "matrix.h"
#include "mesh.h"

typedef struct {
    mesh_t mesh;
    light_t light;
    camera_t camera;
    mat4_t projection_matrix;
    plane_t frustum_planes[NUM_PLANES];
    // dynamic array that is filled in the update function
    triangle_t *triangles_to_render;
} scene_t;

#endif
