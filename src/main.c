#include <math.h>
#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>

#include "array.h"
#include "camera.h"
#include "clip.h"
#include "color.h"
#include "display.h"
#include "light.h"
#include "matrix.h"
#include "mesh.h"
#include "scene.h"
#include "triangle.h"
#include "vector.h"

static bool is_running = false;
static int previous_frame_time = 0;
static float delta_time;

// Poll for input while running
static void process_input(camera_t *camera) {
    // TODO: make input smoother
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
                is_running = false;

            if (event.key.keysym.sym == SDLK_1)
                set_render_mode(RENDER_WIRE_FRAME);
            if (event.key.keysym.sym == SDLK_2)
                set_render_mode(RENDER_WIRE_VERTS);
            if (event.key.keysym.sym == SDLK_3)
                set_render_mode(RENDER_FILL);
            if (event.key.keysym.sym == SDLK_4)
                set_render_mode(RENDER_FILL_WIRE);
            if (event.key.keysym.sym == SDLK_5)
                set_render_mode(RENDER_TEXTURE);
            if (event.key.keysym.sym == SDLK_6)
                set_render_mode(RENDER_TEXTURE_WIRE);

            // camera y control
            if (event.key.keysym.sym == SDLK_SPACE)
                camera->position.y += 3.0f * delta_time;
            if (event.key.keysym.sym == SDLK_c)
                camera->position.y -= 3.0f * delta_time;

            // yaw control
            if (event.key.keysym.sym == SDLK_RIGHT)
                camera->yaw += 3.0f * delta_time;
            if (event.key.keysym.sym == SDLK_LEFT)
                camera->yaw -= 3.0f * delta_time;

            // pitch control
            if (event.key.keysym.sym == SDLK_UP)
                camera->pitch -= 3.0f * delta_time;
            if (event.key.keysym.sym == SDLK_DOWN)
                camera->pitch += 3.0f * delta_time;

            // forward velocity control
            if (event.key.keysym.sym == SDLK_w) {
                camera->forward_vel = vec3_mul(camera->forward_direction, 5.0f * delta_time);
                camera->position = vec3_add(camera->position, camera->forward_vel);
            }
            if (event.key.keysym.sym == SDLK_s) {
                camera->forward_vel = vec3_mul(camera->forward_direction, 5.0f * delta_time);
                camera->position = vec3_sub(camera->position, camera->forward_vel);
            }

            // right velocity control
            if (event.key.keysym.sym == SDLK_d) {
                camera->right_vel = vec3_mul(camera->right_direction, 5.0f * delta_time);
                camera->position = vec3_add(camera->position, camera->right_vel);
            }
            if (event.key.keysym.sym == SDLK_a) {
                camera->right_vel = vec3_mul(camera->right_direction, 5.0f * delta_time);
                camera->position = vec3_sub(camera->position, camera->right_vel);
            }

            if (event.key.keysym.sym == SDLK_p)
                set_render_mode(RENDER_TEXTURE_PS1);

            if (event.key.keysym.sym == SDLK_b)
                switch_cull_mode();
            break;
        }
    }
}

// Might be thought of as our vertex shader, takes a scene and transforms all the meshes from model
// space into screen space
// stages:
// Model Space
// World Space (verts * world matrix, origin is now world origin)
// Camera Space (verts * view matrix, origin is now camera position)
// Clip (don't send triangles facing away from the camera to be rendered... or if outside frustum)
// Project (verts * projection matrix)
// Image Space (verts / og_z)
// Screen Space (transform to center of screen)
static void update(scene_t *scene) {
    // hit goal frame time
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }
    // factor by which we update data per frame
    delta_time = (SDL_GetTicks() - previous_frame_time) / SECOND;

    previous_frame_time = SDL_GetTicks();

    int window_width, window_height;
    get_window_size(&window_width, &window_height);

    // update the camera and see where its looking
    vec3_t target = camera_update_target(&scene->camera);
    mat4_t view_matrix =
        mat4_make_look_at(scene->camera.position, target, scene->camera.up_direction);

    int num_meshes = array_size(scene->meshes);
    for (int m = 0; m < num_meshes; m++) {
        // get pointers since we are going to modify these
        mesh_t *mesh = &scene->meshes[m];

        // reset the triangles each frame
        array_reset(mesh->raster_tris);

        mat4_t scale_matrix = mat4_make_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
        mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh->rotation.x);
        mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh->rotation.y);
        mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh->rotation.z);
        mat4_t translation_matrix =
            mat4_make_translation(mesh->translation.x, mesh->translation.y, mesh->translation.z);

        // Combine all transforms, scale, rotate, translate, in that order
        mat4_t world_matrix = mat4_identity();
        world_matrix = mat4_mul_mat4(&scale_matrix, &world_matrix);
        world_matrix = mat4_mul_mat4(&rotation_matrix_z, &world_matrix);
        world_matrix = mat4_mul_mat4(&rotation_matrix_y, &world_matrix);
        world_matrix = mat4_mul_mat4(&rotation_matrix_x, &world_matrix);
        world_matrix = mat4_mul_mat4(&translation_matrix, &world_matrix);

        // Loop faces first, get vertices from faces, project triangle, add to array
        int num_faces = array_size(mesh->faces);
        for (int i = 0; i < num_faces; i++) {

            // Find vertices in face
            vec3_t face_vertices[3] = {
                mesh->vertices[mesh->faces[i].a],
                mesh->vertices[mesh->faces[i].b],
                mesh->vertices[mesh->faces[i].c],
            };

            // Transform vertices to view space
            vec3_t transformed_vertices[3];
            for (int j = 0; j < 3; j++) {
                vec4_t transformed_vertex = vec3_to_vec4(face_vertices[j]);

                // To world space
                transformed_vertex = mat4_mul_vec4(&world_matrix, transformed_vertex);

                // To camera space
                transformed_vertex = mat4_mul_vec4(&view_matrix, transformed_vertex);

                transformed_vertices[j] = vec4_to_vec3(transformed_vertex);
            }

            // Shading and backface culling need triangle normal
            vec3_t face_normal = triangle_normal(transformed_vertices);

            // origin is now camera position after camera space transformation
            vec3_t camera_ray = vec3_sub((vec3_t){0, 0, 0}, transformed_vertices[1]);

            // Skip projecting and pushing this triangle to render, if face is
            // looking away from camera
            if (should_cull_bface()) {
                if (vec3_dot(camera_ray, face_normal) < 0.0f)
                    continue;
            }

            // Perform frustum clipping
            polygon_t clip_poly = {
                .vertices =
                    {
                        transformed_vertices[0],
                        transformed_vertices[1],
                        transformed_vertices[2],
                    },
                .tex_coords =
                    {
                        mesh->faces[i].a_uv,
                        mesh->faces[i].b_uv,
                        mesh->faces[i].c_uv,
                    },
                .num_vertices = 3,
            };
            clip_polygon_to_planes(scene->frustum_planes, &clip_poly);

            // Back to triangles
            triangle_t clipped_tris[MAX_NUM_POLY_TRIS];
            int num_clipped_tris = polygon_to_tris(&clip_poly, clipped_tris);

            // For all the new triangles do projection
            for (int t = 0; t < num_clipped_tris; t++) {
                triangle_t clipped_triangle = clipped_tris[t];

                // Project into 2d points, but still saving the new "adjusted" z,
                // and original z in w
                vec4_t projected_vertices[3];
                for (int j = 0; j < 3; j++) {
                    // Project to screen space, also performs perspective divide
                    projected_vertices[j] = mat4_mul_vec4_project(&scene->projection_matrix,
                                                                  clipped_triangle.points[j]);

                    // Scale it up
                    projected_vertices[j].x *= (window_width / 2.f);
                    // mult by -1 to invert in screen space as models have opposite
                    // y axis
                    projected_vertices[j].y *= -(window_height / 2.f);

                    // Translate point to middle of screen
                    projected_vertices[j].x += (window_width / 2.f);
                    projected_vertices[j].y += (window_height / 2.f);
                }

                // Flat shading
                vec3_normalize(&scene->light.direction);
                vec3_normalize(&face_normal);
                // Negative because pointing at the light means more light
                float light_alignment = -vec3_dot(scene->light.direction, face_normal);
                color_t shaded_color = light_apply_intensity(mesh->faces[i].color, light_alignment);

                // Not necessary to divide by 3 here, does not change relative ordering
                float avg_z = transformed_vertices[0].z + transformed_vertices[1].z +
                              transformed_vertices[2].z;

                triangle_t triangle_to_render = {
                    .points =
                        {
                            projected_vertices[0],
                            projected_vertices[1],
                            projected_vertices[2],
                        },
                    .tex_coords =
                        {
                            clipped_triangle.a_uv,
                            clipped_triangle.b_uv,
                            clipped_triangle.c_uv,
                        },
                    .color = shaded_color,
                    .avg_depth = avg_z,
                };

                array_push(mesh->raster_tris, triangle_to_render);
            }
        }

        // Sorting painters algorithm, like old days when memory was more
        // expensive
        if (should_render_ps1()) {
            qsort(mesh->raster_tris, array_size(mesh->raster_tris), sizeof(*(mesh->raster_tris)),
                  triangle_painter_compare);
        }
    }
}

// Might be thought of as our rasterizer and fragment shader, takes the screen meshes and draws them
static void render(scene_t *scene) {
    clear_color_buffer(BLACK);
    clear_w_buffer();
    draw_grid(GREY);

    int num_meshes = array_size(scene->meshes);
    for (int m = 0; m < num_meshes; m++) {
        mesh_t *mesh = &scene->meshes[m];

        int num_triangles = array_size(mesh->raster_tris);
        for (int i = 0; i < num_triangles; i++) {
            triangle_t triangle = mesh->raster_tris[i];
            // Rasterization method requires vertices to run from top to bottom
            sort_triangle_by_y(&triangle);

            // FIXME: passing triangle by reference causes some to not be rendered

            // Draw Textured Triangles
            if (should_render_texture()) {
                draw_textured_triangle(triangle, &mesh->texture);
            }

            // Draw Filled Triangles
            if (should_render_fill()) {
                draw_filled_triangle(triangle);
            }

            // Draw Unfilled Triangles
            if (should_render_wire()) {
                draw_triangle(roundf(triangle.points[0].x), roundf(triangle.points[0].y),
                              roundf(triangle.points[1].x), roundf(triangle.points[1].y),
                              roundf(triangle.points[2].x), roundf(triangle.points[2].y), GREEN);
            }

            // Draw Vertices
            if (should_render_verts()) {
                for (int j = 0; j < 3; j++) {
                    draw_rectangle(roundf(triangle.points[j].x) - 3,
                                   roundf(triangle.points[j].y) - 3, 6, 6, GREEN);
                }
            }

            // SECRET!
            if (should_render_ps1()) {
                draw_affine_textured_triangle(triangle, &mesh->texture);
            }
        }
    }

    render_color_buffer();
}

int main(int argc, char *args[]) {
    is_running = window_init();

    scene_t scene = {0};
    scene_init(&scene);

    while (is_running) {
        process_input(&scene.camera);
        update(&scene);
        render(&scene);
    }

    scene_free(&scene);
    window_free();

    return 0;
}
