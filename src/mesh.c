#include "mesh.h"

#include "array.h"
#include "texture.h"

#include <stdio.h>
#include <stdlib.h>

static void load_obj_file_data(mesh_t *mesh, const char *file_name) {
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
                .color = WHITE,
            };
            array_push(mesh->faces, obj_face);
        }
    }
    array_free(texcoords);
    fclose(obj_file);
}

void mesh_init(mesh_t *mesh, const char *obj_file_name, const char *png_file_name, vec3_t rotation,
               vec3_t scale, vec3_t translation) {
    mesh->rotation = rotation;
    mesh->scale = scale;
    mesh->translation = translation;

    load_obj_file_data(mesh, obj_file_name);
    load_png_texture_data(&mesh->texture, png_file_name);

    int num_faces = array_size(mesh->faces);
    // we'll allocate an array of all the faces in a mesh, most likely it won't need it all but just
    // to be safe it should create a bit of a buffer from reallocating if we make new triangles when
    // clipping
    mesh->raster_tris = array_hold(mesh->raster_tris, num_faces, sizeof(triangle_t));
}

void mesh_free(mesh_t *mesh) {
    array_free(mesh->vertices);
    array_free(mesh->faces);
    texture_free(&mesh->texture);
    array_free(mesh->raster_tris);

    *mesh = (mesh_t){0};
}
