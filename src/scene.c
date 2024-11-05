#include "scene.h"
#include "array.h"

#define M_PI 3.14159265358979323846

// Initialize all scene elements:
// meshes, lights, the camera, projection matrix, frustum planes
void scene_init(scene_t *scene) {
    scene->light = (light_t){
        .direction = {.x = 0.0f, .y = 0.0f, .z = 1.0f},
    };

    // load all meshes, and allocate memory for screen meshes
    for (int i = 0; i < 1; i++) {
        mesh_t temp_mesh = {0};
        mesh_init(&temp_mesh, "./assets/f22.obj", "./assets/f22.png", (vec3_t){0.0f, 0.0f, 0.0f},
                  (vec3_t){1.0f, 1.0f, 1.0f}, (vec3_t){0.0f, 0.0f, 5.0f});
        array_push(scene->meshes, temp_mesh);

        // will hold all projected meshes
        screen_mesh_t temp_screen_mesh = {0};
        array_push(scene->screen_meshes, temp_screen_mesh);
    }

    camera_init(&scene->camera, (vec3_t){0, 0, 0}, (vec3_t){0, 1, 0}, (vec3_t){0, 0, 1});

    int window_width, window_height;
    get_window_size(&window_width, &window_height);

    float aspect = (float)window_width / window_height;
    float inv_aspect = (float)window_height / window_width;
    float fov_y = M_PI / 3.0f; // radians
    float fov_x = 2 * atanf(tanf(fov_y / 2) * aspect);
    float z_near = 0.8f;
    float z_far = 20.0f;

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

    // free dynamic array of screen_meshes
    array_free(scene->screen_meshes);
}
