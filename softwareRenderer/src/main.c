#include <stdio.h>
#include <stdint.h>
#include <SDL.h>
#include <stdbool.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "matrix.h"
#include "light.h"
#include "texture.h"
#include "triangle.h"

triangle_t* triangles_to_render = NULL;

vec3_t camera_position = { 0, 0, 0 };
mat4_t projection_matrix;
light_t global_light = { 0, 0, 1 };

static bool is_running = false;
static int previous_frame_time = 0;

// Color buffer initialization, other setups too
static void setup(void) {
	render_mode = RENDER_TEXTURE;	// default render mode
	cull_mode = CULL_BACKFACE;	// default cull mode

	// Memory for color buffer
	color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);
	if (!color_buffer) {
		fprintf(stderr, "Error creating color buffer.\n");
		is_running = false;
	}

	// SDL texture for rendering buffer from memory
	color_buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);
	if (!color_buffer) {
		fprintf(stderr, "Error creating color buffer texture.\n");
		is_running = false;
	}

	float fov = M_PI/3.0f; // radians
	float inv_aspect = (float) window_height / window_width;
	float znear = 0.1f;
	float zfar = 100.0f;
	projection_matrix = mat4_make_perspective(fov, inv_aspect, znear, zfar);

	// Load hard coded redbrick texture
	mesh_texture = (uint32_t*) REDBRICK_TEXTURE;
	texture_height = 64;
	texture_width = 64;

	load_cube_mesh_data();
	//load_obj_file_data("./assets/f22.obj");
}

// Poll for input while running
static void process_input(void) {
	SDL_Event event;
	SDL_PollEvent(&event);

	switch (event.type)
	{
		case SDL_QUIT:
			is_running = false;
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
				is_running = false;
			if (event.key.keysym.sym == SDLK_1)
				render_mode = RENDER_WIRE_FRAME;
			if (event.key.keysym.sym == SDLK_2)
				render_mode = RENDER_WIRE_VERTS;
			if (event.key.keysym.sym == SDLK_3)
				render_mode = RENDER_FILL;
			if (event.key.keysym.sym == SDLK_4)
				render_mode = RENDER_FILL_WIRE;
			if (event.key.keysym.sym == SDLK_5)
				render_mode = RENDER_TEXTURE_WIRE;
			if (event.key.keysym.sym == SDLK_6)
				render_mode = RENDER_TEXTURE;

			if (event.key.keysym.sym == SDLK_c)
				cull_mode = cull_mode == CULL_BACKFACE ? CULL_NONE : CULL_BACKFACE;
			break;
	}
}

// Naive initial perspective projection
//vec2_t project(vec3_t point) {
//	vec2_t projected_point = {
//		.x = (fov_factor * point.x) / point.z,
//		.y = (fov_factor * point.y) / point.z
//	};
//	return projected_point;
//}

static void update(void) {
	// wait for goal frame times, stupid style
	//while (!SDL_TICKS_PASSED(SDL_GetTicks(), previous_frame_time + FRAME_TARGET_TIME));
	
	// hit goal frame time
	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
	if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
		SDL_Delay(time_to_wait);
	}
	previous_frame_time = SDL_GetTicks();

	// Reset triangles
	triangles_to_render = NULL;

	mesh.rotation.x += 0.01f;
	mesh.rotation.y += 0.01f;
	//mesh.rotation.z += 0.01f;

	//mesh.scale.x += 0.001f;

	//mesh.translation.y += 0.01f;
	mesh.translation.z = 5;

	// Create transforms
	mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
	mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
	mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
	mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);
	mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);

	// Combine all transforms, scale, rotate, translate, in that order
	mat4_t world_matrix = mat4_identity();
	world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
	world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
	world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
	world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
	world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

	// Loop faces first, get vertices from faces, project triangle, add to array
	int num_faces = array_size(mesh.faces);
	for (int i = 0; i < num_faces; i++) {
		face_t mesh_face = mesh.faces[i];

		// Find vertices in face
		vec3_t face_vertices[3] = {
			mesh.vertices[mesh_face.a - 1],
			mesh.vertices[mesh_face.b - 1],
			mesh.vertices[mesh_face.c - 1]
		};

		// Transform vertices
		vec4_t transformed_vertices[3];
		for (int j = 0; j < 3; j++) {
			vec4_t transformed_vertex = vec3_to_vec4(face_vertices[j]);

			transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

			transformed_vertices[j] = transformed_vertex;
		}

		float avg_z = (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z) / 3.0f; // average z value of this triangle
		
		// Calculate normal of face for purposes of lighting and culling
		vec3_t A = vec4_to_vec3(transformed_vertices[0]);
		vec3_t B = vec4_to_vec3(transformed_vertices[1]);
		vec3_t C = vec4_to_vec3(transformed_vertices[2]);

		vec3_t AB = vec3_sub(B, A);
		vec3_t AC = vec3_sub(C, A);

		vec3_t face_normal = vec3_cross(AB, AC);
		vec3_t camera_ray = vec3_sub(camera_position, A);
		
		// Skip projecting and pushing this triangle to render, if face is looking away from camera
		if (cull_mode == CULL_BACKFACE) {
			if (vec3_dot(camera_ray, face_normal) < 0.0f)
				continue;
		}

		// Flat shading
		vec3_normalize(&global_light.direction);
		vec3_normalize(&face_normal);
		float light_alignment = -(vec3_dot(global_light.direction, face_normal)); // Negative because pointing at the light means more light
		uint32_t shaded_color = light_apply_intensity(mesh_face.color, light_alignment); 

		// Project into 2d points, but still saving the new "adjusted" z and original z in w
		vec4_t projected_vertices[3];
		for (int j = 0; j < 3; j++) {
			// Project to screen space
			vec4_t projected_vertex = mat4_mul_vec4_project(projection_matrix, transformed_vertices[j]);

			// Scale it up
			projected_vertex.x *= (window_width / 2.f);
			projected_vertex.y *= -(window_height / 2.f); // mult by -1 to invert in screen space as models have opposite y axis

			// Translate point to middle of screen
			projected_vertex.x += (window_width / 2.f);
			projected_vertex.y += (window_height / 2.f);

			// Save that point into triangle
			projected_vertices[j].x = projected_vertex.x;
			projected_vertices[j].y = projected_vertex.y;
		}

		triangle_t projected_triangle = {
			.points = {
				{ projected_vertices[0].x, projected_vertices[0].y, projected_vertices[0].z, projected_vertices[0].w },
				{ projected_vertices[1].x, projected_vertices[1].y, projected_vertices[1].z, projected_vertices[1].w },
				{ projected_vertices[2].x, projected_vertices[2].y, projected_vertices[2].z, projected_vertices[2].w }
			},
			.tex_coords = {
				{ mesh_face.a_uv.u, mesh_face.a_uv.v },
				{ mesh_face.b_uv.u, mesh_face.b_uv.v },
				{ mesh_face.c_uv.u, mesh_face.c_uv.v },
			},
			.color = shaded_color,
			.avg_depth = avg_z
		};

		// Save that triangle for rendering, in dynamic
		array_push(triangles_to_render, projected_triangle);
	}

	// Sort for painters algorithm, like the old days when memory was more expensive, just bubble sort
	int num_triangles = array_size(triangles_to_render);
	for (int i = 0; i < num_triangles; i++) {
		for (int j = 0; j < num_triangles; j++) {
			if (triangles_to_render[i].avg_depth < triangles_to_render[j].avg_depth) {
				triangle_t temp = triangles_to_render[i];
				triangles_to_render[i] = triangles_to_render[j];
				triangles_to_render[j] = temp;
			}
		}
	}
}

// Draw to screen
static void render(void) {
	draw_grid(0xFF808080);

	int num_triangles = array_size(triangles_to_render);
	for (int i = 0; i < num_triangles; i++) {
		triangle_t triangle = triangles_to_render[i];
		
		// Draw Textured Triangles
		if (render_mode == RENDER_TEXTURE || render_mode == RENDER_TEXTURE_WIRE) {
			draw_textured_triangle(
				triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.tex_coords[0].u, triangle.tex_coords[0].v,
				triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.tex_coords[1].u, triangle.tex_coords[1].v,
				triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.tex_coords[2].u, triangle.tex_coords[2].v,
				mesh_texture);
		}

		// Draw Filled Triangles
		if (render_mode == RENDER_FILL || render_mode == RENDER_FILL_WIRE) {
			draw_filled_triangle(
				triangle.points[0].x,
				triangle.points[0].y,
				triangle.points[1].x,
				triangle.points[1].y,
				triangle.points[2].x,
				triangle.points[2].y, 
				triangle.color);
		}

		// Draw Unfilled Triangles
		if (render_mode == RENDER_WIRE_FRAME || render_mode == RENDER_WIRE_VERTS || render_mode == RENDER_FILL_WIRE || render_mode == RENDER_TEXTURE_WIRE) {
			draw_triangle(
				triangle.points[0].x,
				triangle.points[0].y,
				triangle.points[1].x,
				triangle.points[1].y,
				triangle.points[2].x,
				triangle.points[2].y, 
				0xFF00FF00);
		}

		// Draw Vertices
		if (render_mode == RENDER_WIRE_VERTS) {
			for (int j = 0; j < 3; j++) {
				draw_rectangle(triangle.points[j].x - 3, triangle.points[j].y - 3, 6, 6, 0xFF00FF00);
			}
		}
	}

	// Done with this set of triangles
	array_free(triangles_to_render);

	render_color_buffer();
	clear_color_buffer(0xFF000000);

	SDL_RenderPresent(renderer);
}

static void free_resources(void) {
	free(color_buffer);
	array_free(mesh.faces);
	array_free(mesh.vertices);
}

int main(int argc, char* args[]) {
	is_running = initialize_window();

	setup();

	while (is_running) 
	{
		process_input();
		update();
		render();
	}

	destroy_window();
	free_resources();

	return 0;
}