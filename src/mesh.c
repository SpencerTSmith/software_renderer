#include "mesh.h"

#include "array.h"
#include "texture.h"

#include <stdio.h>
#include <stdlib.h>

void load_obj_file_data(mesh_t *mesh, const char *file_name) {
    FILE *obj_file = fopen(file_name, "r");

    if (obj_file == NULL) {
        fprintf(stderr, "Error opening .obj file");
        return;
    }

    char line[512];

    tex2_t *texcoords = NULL; // dynamic array

    while (fgets(line, 512, obj_file)) {

        // Vertices
        if (line[0] == 'v' && line[1] == ' ') {
            vec3_t obj_vertex;
            if (!sscanf(line, "v %f %f %f", &obj_vertex.x, &obj_vertex.y, &obj_vertex.z)) {
                fprintf(stderr, "Error reading vertex data .obj file");
            }
            array_push(mesh->vertices, obj_vertex);
        } else if (line[0] == 'v' && line[1] == 't') {
            tex2_t coord;
            if (!sscanf(line, "vt %f %f", &coord.u, &coord.v)) {
                fprintf(stderr, "Error reading vertex texture coordinate data .obj file");
            }
            // adjust because .obj files have an inverted v compared to the renderer
            coord.v = 1 - coord.v;
            array_push(texcoords, coord);
        }
        // Faces
        else if (line[0] == 'f' && line[1] == ' ') {
            int vertex_indices[3];
            int texture_indices[3];
            int normal_indices[3];

            // Temporary fix for models without normals
            /*sscanf(line, "f %d/%d %d/%d %d/%d ",
                &vertex_indices[0], &texture_indices[0],
                &vertex_indices[1], &texture_indices[1],
                &vertex_indices[2], &texture_indices[2]
                );*/

            if (!sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d ", &vertex_indices[0],
                        &texture_indices[0], &normal_indices[0], &vertex_indices[1],
                        &texture_indices[1], &normal_indices[1], &vertex_indices[2],
                        &texture_indices[2], &normal_indices[2])) {
                fprintf(stderr, "Error reading vertex indice data from .obj file");
            }

            face_t obj_face = {
                .a = vertex_indices[0] - 1, // adjust by 1, indices start at 1 in .obj format
                .b = vertex_indices[1] - 1,
                .c = vertex_indices[2] - 1,
                .a_uv = texcoords[texture_indices[0] - 1],
                .b_uv = texcoords[texture_indices[1] - 1],
                .c_uv = texcoords[texture_indices[2] - 1],
                .color = 0xFFFFFFFF,
            };
            array_push(mesh->faces, obj_face);
        }
    }
    array_free(texcoords);
    fclose(obj_file);
}

void mesh_init(mesh_t *mesh, const char *obj_file_name, const char *png_file_name) {
    mesh->rotation = (vec3_t){0, 0, 0};
    mesh->scale = (vec3_t){1, 1, 1};
    mesh->translation = (vec3_t){0, 0, 0};

    load_obj_file_data(mesh, obj_file_name);
    load_png_texture_data(&mesh->texture, png_file_name);
}

void mesh_free(mesh_t *mesh) {
    mesh->scale = (vec3_t){0, 0, 0};
    mesh->rotation = (vec3_t){0, 0, 0};
    mesh->translation = (vec3_t){0, 0, 0};
    array_free(mesh->vertices);
    array_free(mesh->faces);
    texture_free(&mesh->texture);
}

void load_cube_mesh_data(mesh_t *mesh) {
    // Cube Vertices
    vec3_t cube_vertices[N_CUBE_VERTICES] = {
        {.x = -1, .y = -1, .z = -1}, // 1
        {.x = -1, .y = 1, .z = -1},  // 2
        {.x = 1, .y = 1, .z = -1},   // 3
        {.x = 1, .y = -1, .z = -1},  // 4
        {.x = 1, .y = 1, .z = 1},    // 5
        {.x = 1, .y = -1, .z = 1},   // 6
        {.x = -1, .y = 1, .z = 1},   // 7
        {.x = -1, .y = -1, .z = 1}   // 8
    };

    // Cube faces, by index of vertices
    face_t cube_faces[N_CUBE_FACES] = {
        {.a = 1,
         .b = 2,
         .c = 3,
         .a_uv = {0, 1},
         .b_uv = {0, 0},
         .c_uv = {1, 0},
         .color = 0xFFFFFFFF}, // tri 1 of front
        {.a = 1,
         .b = 3,
         .c = 4,
         .a_uv = {0, 1},
         .b_uv = {1, 0},
         .c_uv = {1, 1},
         .color = 0xFFFFFFFF}, // tri 2 of front

        {.a = 4,
         .b = 3,
         .c = 5,
         .a_uv = {0, 1},
         .b_uv = {0, 0},
         .c_uv = {1, 0},
         .color = 0xFFFFFFFF}, // tri 1 of east
        {.a = 4,
         .b = 5,
         .c = 6,
         .a_uv = {0, 1},
         .b_uv = {1, 0},
         .c_uv = {1, 1},
         .color = 0xFFFFFFFF}, // tri 2 of east

        {.a = 6,
         .b = 5,
         .c = 7,
         .a_uv = {0, 1},
         .b_uv = {0, 0},
         .c_uv = {1, 0},
         .color = 0xFFFFFFFF}, // tri 1 of back
        {.a = 6,
         .b = 7,
         .c = 8,
         .a_uv = {0, 1},
         .b_uv = {1, 0},
         .c_uv = {1, 1},
         .color = 0xFFFFFFFF}, // tri 2 of back

        {.a = 8,
         .b = 7,
         .c = 2,
         .a_uv = {0, 1},
         .b_uv = {0, 0},
         .c_uv = {1, 0},
         .color = 0xFFFFFFFF}, // tri 1 of west
        {.a = 8,
         .b = 2,
         .c = 1,
         .a_uv = {0, 1},
         .b_uv = {1, 0},
         .c_uv = {1, 1},
         .color = 0xFFFFFFFF}, // tri 2 of west

        {.a = 2,
         .b = 7,
         .c = 5,
         .a_uv = {0, 1},
         .b_uv = {0, 0},
         .c_uv = {1, 0},
         .color = 0xFFFFFFFF}, // tri 1 of top
        {.a = 2,
         .b = 5,
         .c = 3,
         .a_uv = {0, 1},
         .b_uv = {1, 0},
         .c_uv = {1, 1},
         .color = 0xFFFFFFFF}, // tri 2 of top

        {.a = 8,
         .b = 1,
         .c = 4,
         .a_uv = {0, 1},
         .b_uv = {0, 0},
         .c_uv = {1, 0},
         .color = 0xFFFFFFFF}, // tri 1 of bottom
        {.a = 8,
         .b = 4,
         .c = 6,
         .a_uv = {0, 1},
         .b_uv = {1, 0},
         .c_uv = {1, 1},
         .color = 0xFFFFFFFF} // tri 2 of bottom
    };
    for (int v = 0; v < N_CUBE_VERTICES; v++) {
        array_push(mesh->vertices, cube_vertices[v]);
    }
    for (int f = 0; f < N_CUBE_FACES; f++) {
        array_push(mesh->faces, cube_faces[f]);
    }

    load_redbrick_mesh_texture(&mesh->texture);
}
