#include "clip.h"

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

plane_t frustum_planes[NUM_PLANES];

void init_frustum_planes(float fov_x, float fov_y, float z_near, float z_far) {
    float cos_half_fov_x = cosf(fov_x / 2);
    float sin_half_fov_x = sinf(fov_x / 2);
    float cos_half_fov_y = cosf(fov_y / 2);
    float sin_half_fov_y = sinf(fov_y / 2);
    vec3_t origin = {0, 0, 0};

    frustum_planes[LEFT_FRUSTUM].point = origin;
    frustum_planes[LEFT_FRUSTUM].normal = (vec3_t){cos_half_fov_x, 0, sin_half_fov_x};

    frustum_planes[RIGHT_FRUSTUM].point = origin;
    frustum_planes[RIGHT_FRUSTUM].normal = (vec3_t){-cos_half_fov_x, 0, sin_half_fov_x};

    frustum_planes[TOP_FRUSTUM].point = origin;
    frustum_planes[TOP_FRUSTUM].normal = (vec3_t){0, -cos_half_fov_y, sin_half_fov_y};

    frustum_planes[BOTTOM_FRUSTUM].point = origin;
    frustum_planes[BOTTOM_FRUSTUM].normal = (vec3_t){0, cos_half_fov_y, sin_half_fov_y};

    frustum_planes[NEAR_FRUSTUM].point = (vec3_t){0, 0, z_near};
    frustum_planes[NEAR_FRUSTUM].normal = (vec3_t){0, 0, 1};

    frustum_planes[FAR_FRUSTUM].point = (vec3_t){0, 0, z_far};
    frustum_planes[FAR_FRUSTUM].normal = (vec3_t){0, 0, -1};
}

static float lerp_float(float a, float b, float lerp_factor) {
    // new = factor * (b - a) + a
    return lerp_factor * (b - a) + a;
}

static void clip_against_plane(polygon_t *polygon, const plane_t *frust_plane) {
    vec3_t plane_point = frust_plane->point;
    vec3_t plane_norm = frust_plane->normal;

    vec3_t inside_verts[MAX_NUM_POLY_VERTS];
    tex2_t inside_texcoords[MAX_NUM_POLY_VERTS];
    int num_in = 0;

    vec3_t prev_vert = polygon->vertices[polygon->num_vertices - 1];
    tex2_t prev_texcoord = polygon->tex_coords[polygon->num_vertices - 1];
    float prev_dot = vec3_dot(vec3_sub(prev_vert, plane_point), plane_norm);

    for (int i = 0; i < polygon->num_vertices; i++) {
        vec3_t curr_vert = polygon->vertices[i];
        tex2_t curr_texcoord = polygon->tex_coords[i];
        float curr_dot = vec3_dot(vec3_sub(curr_vert, plane_point), plane_norm);

        // From in-vert to out-vert/vice versa, so we need a new vert on the plane
        if (curr_dot * prev_dot < 0.0f) {
            float lerp_factor = prev_dot / (prev_dot - curr_dot);
            vec3_t new_vert = {
                .x = lerp_float(prev_vert.x, curr_vert.x, lerp_factor),
                .y = lerp_float(prev_vert.y, curr_vert.y, lerp_factor),
                .z = lerp_float(prev_vert.z, curr_vert.z, lerp_factor),
            };
            tex2_t new_texcoord = {
                .u = lerp_float(prev_texcoord.u, curr_texcoord.u, lerp_factor),
                .v = lerp_float(prev_texcoord.v, curr_texcoord.v, lerp_factor),
            };

            inside_verts[num_in] = new_vert;
            inside_texcoords[num_in] = new_texcoord;
            num_in++;
        }

        // Inside plane
        if (curr_dot > 0.0f) {
            inside_verts[num_in] = curr_vert;
            inside_texcoords[num_in] = curr_texcoord;
            num_in++;
        }

        prev_vert = curr_vert;
        prev_texcoord = curr_texcoord;
        prev_dot = curr_dot;
    }

    memcpy(polygon->vertices, inside_verts, sizeof(inside_verts));
    memcpy(polygon->tex_coords, inside_texcoords, sizeof(inside_texcoords));
    polygon->num_vertices = num_in;
}

void clip_polygon(polygon_t *polygon) {
    clip_against_plane(polygon, &frustum_planes[LEFT_FRUSTUM]);
    clip_against_plane(polygon, &frustum_planes[RIGHT_FRUSTUM]);
    clip_against_plane(polygon, &frustum_planes[TOP_FRUSTUM]);
    clip_against_plane(polygon, &frustum_planes[BOTTOM_FRUSTUM]);
    clip_against_plane(polygon, &frustum_planes[NEAR_FRUSTUM]);
    clip_against_plane(polygon, &frustum_planes[FAR_FRUSTUM]);
}

int polygon_to_tris(polygon_t *polygon, triangle_t triangles[MAX_NUM_POLY_TRIS]) {
    // can make (num_vert - 2) tris from any poly
    int num_triangles = polygon->num_vertices - 2;
    for (int i = 0; i < num_triangles; i++) {
        vec3_t v1 = polygon->vertices[0];
        vec3_t v2 = polygon->vertices[i + 1];
        vec3_t v3 = polygon->vertices[i + 2];

        triangles[i].points[0] = vec3_to_vec4(v1);
        triangles[i].points[1] = vec3_to_vec4(v2);
        triangles[i].points[2] = vec3_to_vec4(v3);
        triangles[i].tex_coords[0] = polygon->tex_coords[0];
        triangles[i].tex_coords[1] = polygon->tex_coords[i + 1];
        triangles[i].tex_coords[2] = polygon->tex_coords[i + 2];
    }

    return num_triangles;
}
