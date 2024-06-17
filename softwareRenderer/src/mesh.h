#ifndef MESH_H
#define MESH_H

#include "vector.h"
#include "triangle.h"

#define N_CUBE_VERTICES 8
#define N_CUBE_FACES (6*2)

// Dynamic size mesh
typedef struct {
	vec3_t* vertices;		// dynamic array of vertices
	face_t* faces;			// dynamic array of faces
	vec3_t rotation;		// x,y,z rotation
	vec3_t scale;			// x,y,z scale
	vec3_t translation;		// x,y,z translation
} mesh_t;

extern mesh_t mesh;

// Read contents of obj file into mesh
void load_obj_file_data(const char* file_name);

void load_cube_mesh_data(void);

extern vec3_t cube_vertices[N_CUBE_VERTICES];
extern face_t cube_faces[N_CUBE_FACES];

#endif