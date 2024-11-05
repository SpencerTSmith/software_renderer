#ifndef CLIPPING_H
#define CLIPPING_H

#include "triangle.h"
#include "vector.h"

#define MAX_NUM_POLY_VERTS 10
#define MAX_NUM_POLY_TRIS (MAX_NUM_POLY_VERTS - 2)

typedef enum {
    LEFT_FRUSTUM,
    RIGHT_FRUSTUM,
    TOP_FRUSTUM,
    BOTTOM_FRUSTUM,
    NEAR_FRUSTUM,
    FAR_FRUSTUM,
    NUM_PLANES
} frustum_side_e;

typedef struct {
    vec3_t point;
    vec3_t normal;
} plane_t;

typedef struct {
    vec3_t vertices[MAX_NUM_POLY_VERTS];
    tex2_t tex_coords[MAX_NUM_POLY_VERTS];
    int num_vertices;
} polygon_t;

void frustum_planes_init(plane_t planes[NUM_PLANES], float fov_x, float fov_y, float z_near,
                         float z_far);

// Will also adjust texture coords, mutates the polygon passed in
void clip_polygon_to_planes(const plane_t planes[NUM_PLANES], polygon_t *polygon);

// Returns number of triangles created, outputs triangles into array
int polygon_to_tris(const polygon_t *polygon, triangle_t triangles[MAX_NUM_POLY_TRIS]);

#endif
