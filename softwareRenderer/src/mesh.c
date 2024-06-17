#include "mesh.h"
#include "array.h"
#include <stdlib.h>
#include <stdio.h>

mesh_t mesh = {
	.vertices = NULL,
	.faces = NULL,
	.rotation = {0, 0, 0},
	.scale = {1.0f, 1.0f, 1.0f},
	.translation = {0, 0, 0}
};

void load_obj_file_data(const char* file_name) {
	FILE* obj_file = fopen(file_name, "r");

	if (obj_file == NULL) {
		fprintf(stderr, "Error opening .obj file");
		return;
	}

	char line[512];

	while (fgets(line, 512, obj_file)) {

		// Vertices
		if (line[0] == 'v' && line[1] == ' ') {
			vec3_t obj_vertex;
			sscanf_s(line, "v %f %f %f", &obj_vertex.x, &obj_vertex.y, &obj_vertex.z);
			array_push(mesh.vertices, obj_vertex);
		}
		// Faces
		else if (line[0] == 'f' && line[1] == ' ') {
			int vertex_indices[3];
			int texture_indices[3];
			int normal_indices[3];

			// Temporary fix for models without normals
			/*sscanf_s(line, "f %d/%d %d/%d %d/%d ",
				&vertex_indices[0], &texture_indices[0], 
				&vertex_indices[1], &texture_indices[1],
				&vertex_indices[2], &texture_indices[2]
				);*/

			sscanf_s(line, "f %d/%d/%d %d/%d/%d %d/%d/%d ",
				&vertex_indices[0], &texture_indices[0], &normal_indices[0],
				&vertex_indices[1], &texture_indices[1], &normal_indices[1],
				&vertex_indices[2], &texture_indices[2], &normal_indices[2]
				);
			
			face_t obj_face = {
				.a = vertex_indices[0], 
				.b = vertex_indices[1], 
				.c = vertex_indices[2], 
				.color = 0xFFFFFFFF};
			array_push(mesh.faces, obj_face);
		}
	}


	fclose(obj_file);
}

// Cube Vertices
vec3_t cube_vertices[N_CUBE_VERTICES] = {
	{.x = -1, .y = -1, .z = -1}, // 1
	{.x = -1, .y =  1, .z = -1}, // 2
	{.x =  1, .y =  1, .z = -1}, // 3
	{.x =  1, .y = -1, .z = -1}, // 4
	{.x =  1, .y =  1, .z =  1}, // 5
	{.x =  1, .y = -1, .z =  1}, // 6
	{.x = -1, .y =  1, .z =  1}, // 7
	{.x = -1, .y = -1, .z =  1}	 // 8
};

// Cube faces, by index of vertices
face_t cube_faces[N_CUBE_FACES] = {
	{.a = 1, .b = 2, .c = 3, .a_uv = { 0, 0 }, .b_uv = { 0, 1 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF}, // tri 1 of front
	{.a = 1, .b = 3, .c = 4, .a_uv = { 0, 0 }, .b_uv = { 1, 1 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF}, // tri 2 of front

	{.a = 4, .b = 3, .c = 5, .a_uv = { 0, 0 }, .b_uv = { 0, 1 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF}, // tri 1 of east
	{.a = 4, .b = 5, .c = 6, .a_uv = { 0, 0 }, .b_uv = { 1, 1 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF}, // tri 2 of east
							  
	{.a = 6, .b = 5, .c = 7, .a_uv = { 0, 0 }, .b_uv = { 0, 1 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF}, // tri 1 of back
	{.a = 6, .b = 7, .c = 8, .a_uv = { 0, 0 }, .b_uv = { 1, 1 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF}, // tri 2 of back
							 
	{.a = 8, .b = 7, .c = 2, .a_uv = { 0, 0 }, .b_uv = { 0, 1 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF}, // tri 1 of west
	{.a = 8, .b = 2, .c = 1, .a_uv = { 0, 0 }, .b_uv = { 1, 1 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF}, // tri 2 of west
							  
	{.a = 2, .b = 7, .c = 5, .a_uv = { 0, 0 }, .b_uv = { 0, 1 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF}, // tri 1 of top
	{.a = 2, .b = 5, .c = 3, .a_uv = { 0, 0 }, .b_uv = { 1, 1 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF}, // tri 2 of top
							 
	{.a = 8, .b = 1, .c = 4, .a_uv = { 0, 0 }, .b_uv = { 0, 1 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF}, // tri 1 of bottom
	{.a = 8, .b = 4, .c = 6, .a_uv = { 0, 0 }, .b_uv = { 1, 1 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF}  // tri 2 of bottom
};

void load_cube_mesh_data(void) {
	for (int v = 0; v < N_CUBE_VERTICES; v++) {
		array_push(mesh.vertices, cube_vertices[v]);
	}
	for (int f = 0; f < N_CUBE_FACES; f++) {
		array_push(mesh.faces, cube_faces[f]);
	}
}
