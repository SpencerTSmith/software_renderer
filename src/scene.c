#include <time.h>

#include "array.h"
#include "scene.h"

#define M_PI 3.14159265358979323846

// Initialize all scene elements:
// meshes, lights, the camera, projection matrix, frustum planes
void scene_init(scene_t *scene) {
    scene->light = (light_t){
        .direction = {.x = 0.0f, .y = 0.0f, .z = 1.0f},
    };

    srand(time(NULL));

    // load all meshes, and allocate memory for screen meshes
    for (int i = 0; i < 5; i++) {
        float random = rand() / (float)RAND_MAX; // between 0.0 and 1.0
        random -= 0.5f;                          // between -0.5 and 0.5
        random *= 5.0f;                          // between -2.5 and 2.5

        mesh_t temp_mesh = {0};
        vec3_t rotation = {(i * random), (i * random), (i * random)};
        vec3_t scale = {1.0f, 1.0f, 1.0f};
        vec3_t position = {(i * random), (i * random), (i * random)};
        mesh_init(&temp_mesh, "./assets/crab.obj", "./assets/crab.png", rotation, scale, position);
        array_push(scene->meshes, temp_mesh);
    }

    camera_init(&scene->camera, (vec3_t){0, 0, 0}, (vec3_t){0, 1, 0}, (vec3_t){0, 0, 1});

    int window_width, window_height;
    get_window_size(&window_width, &window_height);

    float aspect = (float)window_width / window_height;
    float inv_aspect = (float)window_height / window_width;
    float fov_y = M_PI / 2.0f; // radians
    float fov_x = 2 * atanf(tanf(fov_y / 2) * aspect);
    float z_near = 1.0f;
    float z_far = 50.0f;

    scene->projection_matrix = mat4_make_perspective(fov_y, inv_aspect, z_near, z_far);
    frustum_planes_init(scene->frustum_planes, fov_x, fov_y, z_near, z_far);
}

void scene_free(scene_t *scene) {
    // free all the meshes dynamic memory
    int num_meshes = array_size(scene->meshes);
    for (int i = 0; i < num_meshes; i++) {
        mesh_free(&scene->meshes[i]);
    }

    // free the dynamic list of meshes
    array_free(scene->meshes);
    *scene = (scene_t){0};

    memset(scene, 0, sizeof(scene_t));
}
