#ifndef SCENE_H
#define SCENE_H

#include "camera.h"
#include "clip.h"
#include "light.h"
#include "matrix.h"
#include "mesh.h"

#define MAX_TRIANGLES 16384

// collection of transformed and projected triangles, from scene meshes
typedef struct {
    triangle_t *triangles; // dynamic array of triangles
} screen_mesh_t;

typedef struct {
    mesh_t *meshes; // dynamic array of meshes
    light_t light;

    camera_t camera;
    mat4_t projection_matrix;
    plane_t frustum_planes[NUM_PLANES];

    screen_mesh_t *screen_meshes; // dynamic array of screen meshes
} scene_t;

// Assumes 0 initialization
void scene_init(scene_t *scene);
void scene_free(scene_t *scene);
#endif
