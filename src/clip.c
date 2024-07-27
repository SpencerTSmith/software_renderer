#include "clip.h"

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

plane_t frustum_planes[NUM_PLANES];

void init_frustum_planes(float fov, float z_near, float z_far) {
	float cos_half_fov = cosf(fov / 2);
	float sin_half_fov = sinf(fov / 2);
	vec3_t origin = {0, 0, 0};

	frustum_planes[LEFT_FRUSTUM].point = origin;
	frustum_planes[LEFT_FRUSTUM].normal =
		(vec3_t){cos_half_fov, 0, sin_half_fov};

	frustum_planes[RIGHT_FRUSTUM].point = origin;
	frustum_planes[RIGHT_FRUSTUM].normal =
		(vec3_t){-cos_half_fov, 0, sin_half_fov};

	frustum_planes[TOP_FRUSTUM].point = origin;
	frustum_planes[TOP_FRUSTUM].normal =
		(vec3_t){0, -cos_half_fov, sin_half_fov};

	frustum_planes[BOTTOM_FRUSTUM].point = origin;
	frustum_planes[BOTTOM_FRUSTUM].normal =
		(vec3_t){0, cos_half_fov, sin_half_fov};

	frustum_planes[NEAR_FRUSTUM].point = (vec3_t){0, 0, z_near};
	frustum_planes[NEAR_FRUSTUM].normal = (vec3_t){0, 0, 1};

	frustum_planes[FAR_FRUSTUM].point = (vec3_t){0, 0, z_far};
	frustum_planes[FAR_FRUSTUM].normal = (vec3_t){0, 0, -1};
}

vec3_t lerp_new_vert(vec3_t prev_vert, vec3_t curr_vert, float prev_dot,
					 float curr_dot) {
	float lerp_factor = (prev_dot) / (prev_dot - curr_dot);

	// new = prev + factor * (curr - prev)
	vec3_t new_vert = vec3_add(
		prev_vert, vec3_mul(vec3_sub(curr_vert, prev_vert), lerp_factor));

	return new_vert;
}

void clip_against_plane(polygon_t *polygon, const plane_t *frust_plane) {
	vec3_t plane_point = frust_plane->point;
	vec3_t plane_norm = frust_plane->normal;

	vec3_t inside_plane[MAX_NUM_POLY_VERTS];
	int num_in = 0;

	vec3_t prev_vert = polygon->vertices[polygon->num_vertices - 1];
	float prev_dot = vec3_dot(vec3_sub(prev_vert, plane_point), plane_norm);
	for (int i = 0; i < polygon->num_vertices; i++) {
		vec3_t curr_vert = polygon->vertices[i];
		float curr_dot = vec3_dot(vec3_sub(curr_vert, plane_point), plane_norm);

		if (curr_dot * prev_dot < 0.0f) { // From in-vert to out-vert/vice versa
			vec3_t new_vert =
				lerp_new_vert(curr_vert, prev_vert, prev_dot, curr_dot);
			inside_plane[num_in++] = new_vert;
		}

		if (curr_dot > 0.0f) { // Inside plane
			inside_plane[num_in++] = curr_vert;
		}
		prev_dot = curr_dot;
		prev_vert = curr_vert;
	}

	memcpy(polygon->vertices, inside_plane, sizeof(inside_plane));
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
