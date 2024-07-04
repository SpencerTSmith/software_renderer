#include <stdio.h>
#include <stdint.h>

#include <SDL.h>

#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "matrix.h"
#include "light.h"
#include "texture.h"
#include "triangle.h"
#include "camera.h"

float delta_time;

#define MAX_TRIANGLES 4096
triangle_t triangles_to_render[MAX_TRIANGLES];
int num_triangles;

mat4_t projection_matrix;
light_t global_light = { 0, 0, 1 };

static bool is_running = false;
static int previous_frame_time = 0;

// Color buffer initialization, other setups too
static void setup(void) {
	render_mode = RENDER_WIRE_FRAME;	// default render mode
	cull_mode = CULL_BACKFACE;			// default cull mode

	// Memory for color buffer
	color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);
	if (!color_buffer) {
		fprintf(stderr, "Error creating color buffer.\n");
		is_running = false;
	}

	// Memory for z buffer
	w_buffer = (float*) malloc(sizeof(float) * window_width * window_height);

	// SDL texture for rendering buffer from memory
	color_buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);
	if (!color_buffer) {
		fprintf(stderr, "Error creating color buffer texture.\n");
		is_running = false;
	}

	float fov = M_PI/3.0f; // radians
	float inv_aspect = (float) window_height / window_width;
	float znear = 0.1f;
	float zfar = 100.0f;
	projection_matrix = mat4_make_perspective(fov, inv_aspect, znear, zfar);

	//load_redbrick_mesh_texture();
	//load_cube_mesh_data();
	
	load_png_texture_data("./assets/crab.png");
	load_obj_file_data("./assets/crab.obj");
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
			
			
			if (event.key.keysym.sym == SDLK_p)
				render_mode = RENDER_TEXTURE_PS1;

			if (event.key.keysym.sym == SDLK_c)
				cull_mode = cull_mode == CULL_BACKFACE ? CULL_NONE : CULL_BACKFACE;
			break;
	}
}

// Might be thought of as our vertex shader
static void update(void) {
	// hit goal frame time
	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
	if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
		SDL_Delay(time_to_wait);
	}
	delta_time = (SDL_GetTicks() - previous_frame_time) / 1000.0f;	// factor by which we update data per frame
	
	previous_frame_time = SDL_GetTicks();


	// Clear our triangles to render
	num_triangles = 0;

	// Create transforms
	//mesh.rotation.x += 0.1f * delta_time;
	mesh.rotation.y += 0.1f * delta_time;
	//mesh.rotation.z += 0.1f * delta_time;

	//mesh.scale.x += 0.001f * delta_time;

	//mesh.translation.y += 0.01f * delta_time;
	mesh.translation.z = 5.0f;

	mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
	mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
	mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
	mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);
	mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);

	// Combine all transforms, scale, rotate, translate, in that order
	mat4_t world_matrix = mat4_identity();
	world_matrix = mat4_mul_mat4(&scale_matrix, &world_matrix);
	world_matrix = mat4_mul_mat4(&rotation_matrix_z, &world_matrix);
	world_matrix = mat4_mul_mat4(&rotation_matrix_y, &world_matrix);
	world_matrix = mat4_mul_mat4(&rotation_matrix_x, &world_matrix);
	world_matrix = mat4_mul_mat4(&translation_matrix, &world_matrix);

	// We need a new view matrix every frame
	camera.position.x += 0.1 * delta_time;
	camera.position.y += 0.1 * delta_time;
	mat4_t view_matrix = mat4_make_look_at(camera.position, mesh.translation, (vec3_t) { 0, 1, 0 });

	// Loop faces first, get vertices from faces, project triangle, add to array
	int num_faces = array_size(mesh.faces);
	for (int i = 0; i < num_faces; i++) {
		face_t mesh_face = mesh.faces[i];

		// Find vertices in face
		vec3_t face_vertices[3] = {
			mesh.vertices[mesh_face.a],
			mesh.vertices[mesh_face.b],
			mesh.vertices[mesh_face.c]
		};

		// Transform vertices to world space
		vec4_t transformed_vertices[3];
		for (int j = 0; j < 3; j++) {
			vec4_t transformed_vertex = vec3_to_vec4(face_vertices[j]);

			// To world space
			transformed_vertex = mat4_mul_vec4(&world_matrix, transformed_vertex);

			// To camera space
			transformed_vertex = mat4_mul_vec4(&view_matrix, transformed_vertex);

			transformed_vertices[j] = transformed_vertex;
		}

		// Calculate normal of face for purposes of lighting and culling
		vec3_t A = vec4_to_vec3(transformed_vertices[0]);
		vec3_t B = vec4_to_vec3(transformed_vertices[1]);
		vec3_t C = vec4_to_vec3(transformed_vertices[2]);

		vec3_t AB = vec3_sub(B, A);
		vec3_t AC = vec3_sub(C, A);

		vec3_t face_normal = vec3_cross(AB, AC);
		vec3_t camera_ray = vec3_sub((vec3_t){0, 0, 0}, A); // origin is now camera position after camera space transformation
		
		// Skip projecting and pushing this triangle to render, if face is looking away from camera
		if (cull_mode == CULL_BACKFACE) {
			if (vec3_dot(camera_ray, face_normal) < 0.0f)
				continue;
		}

		// Flat shading
		vec3_normalize(&global_light.direction);
		vec3_normalize(&face_normal);
		float light_alignment = -vec3_dot(global_light.direction, face_normal); // Negative because pointing at the light means more light
		uint32_t shaded_color = light_apply_intensity(mesh_face.color, light_alignment); 

		// Not necessary to divide by 3 here, does not change relative ordering
		float avg_z = transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z;

		// Project into 2d points, but still saving the new "adjusted" z, and original z in w
		vec4_t projected_vertices[3];
		for (int j = 0; j < 3; j++) {
			// Project to screen space
			vec4_t projected_vertex = mat4_mul_vec4_project(&projection_matrix, transformed_vertices[j]);

			// Scale it up
			projected_vertex.x *= (window_width / 2.f);
			projected_vertex.y *= -(window_height / 2.f); // mult by -1 to invert in screen space as models have opposite y axis

			// Translate point to middle of screen
			projected_vertex.x += (window_width / 2.f);
			projected_vertex.y += (window_height / 2.f);

			// Save that point
			projected_vertices[j] = projected_vertex;
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

		if (num_triangles < MAX_TRIANGLES) {
			triangles_to_render[num_triangles++] = projected_triangle;
		}
	}

	// Sort for painters algorithm, like the old days when memory was more expensive, just bubble sort
	if (render_mode == RENDER_TEXTURE_PS1) {
		qsort(triangles_to_render, num_triangles, sizeof(*triangles_to_render), triangle_painter_compare);
	}
}

// Might be thought of as our rasterizer and fragment shader
static void render(void) {
	//draw_grid(0xFF808080);

	for (int i = 0; i < num_triangles; i++) {
		triangle_t triangle = triangles_to_render[i];
		sort_triangle_by_y(&triangle); // Rasterization method requires vertices to run from top to bottom

		// Draw Textured Triangles
		if (render_mode == RENDER_TEXTURE || render_mode == RENDER_TEXTURE_WIRE) {
			draw_textured_triangle(triangle, mesh_texture); //passing triangle by reference causes some to not be rendered
		}

		// Draw Filled Triangles
		if (render_mode == RENDER_FILL || render_mode == RENDER_FILL_WIRE) {
			draw_filled_triangle(triangle);
		}

		// Draw Unfilled Triangles
		if (render_mode == RENDER_WIRE_FRAME || render_mode == RENDER_WIRE_VERTS || 
			render_mode == RENDER_FILL_WIRE || render_mode == RENDER_TEXTURE_WIRE) {
			draw_triangle(
				roundf(triangle.points[0].x),
				roundf(triangle.points[0].y),
				roundf(triangle.points[1].x),
				roundf(triangle.points[1].y),
				roundf(triangle.points[2].x),
				roundf(triangle.points[2].y), 
				0xFF0000FF);
		}

		// Draw Vertices
		if (render_mode == RENDER_WIRE_VERTS) {
			for (int j = 0; j < 3; j++) {
				draw_rectangle(roundf(triangle.points[j].x) - 3, roundf(triangle.points[j].y) - 3, 6, 6, 0xFF0000FF);
			}
		}

		// SECRET!
		if (render_mode == RENDER_TEXTURE_PS1) {
			draw_affine_textured_triangle(triangle, mesh_texture);
		}
	}

	render_color_buffer();

	clear_color_buffer(0xFF000000);
	clear_w_buffer();

	SDL_RenderPresent(renderer);
}

static void free_resources(void) {
	free(color_buffer);
	free(w_buffer);
	texture_free(mesh_texture);
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