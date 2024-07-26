#include "clip.h"
#include <math.h>

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

void clip_against_plane(polygon_t *polygon, int frust_plane) {


}

void clip_polygon(polygon_t *polygon) {
	clip_against_plane(polygon, LEFT_FRUSTUM);
	clip_against_plane(polygon, RIGHT_FRUSTUM);
	clip_against_plane(polygon, TOP_FRUSTUM);
	clip_against_plane(polygon, BOTTOM_FRUSTUM);
	clip_against_plane(polygon, NEAR_FRUSTUM);
	clip_against_plane(polygon, FAR_FRUSTUM);
}

