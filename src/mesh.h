#ifndef MESH_H
#define MESH_H

#include "texture.h"
#include "triangle.h"
#include "vector.h"

#define N_CUBE_VERTICES 8
#define N_CUBE_FACES (6 * 2)

// Dynamic size mesh
typedef struct {
    vec3_t rotation, scale, translation;
    vec3_t *vertices; // dynamic array of vertices
    face_t *faces;    // dynamic array of faces
    texture_t texture;
} mesh_t;

void mesh_init(mesh_t *mesh, const char *obj_file_name, const char *png_file_name, vec3_t rotation,
               vec3_t scale, vec3_t translation);

void mesh_free(mesh_t *mesh);

#endif
