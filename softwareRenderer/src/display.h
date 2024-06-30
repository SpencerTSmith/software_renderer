#ifndef DISPLAY_H
#define DISPLAY_H
#include <SDL.h>

#include <stdbool.h>
#include <stdint.h>

#define FPS 240
#define FRAME_TARGET_TIME (1000 / FPS) 

enum render_mode {
	RENDER_WIRE_FRAME,
	RENDER_WIRE_VERTS,
	RENDER_FILL,
	RENDER_FILL_WIRE,
	RENDER_TEXTURE,
	RENDER_TEXTURE_WIRE
} render_mode;

enum cull_mode {
	CULL_BACKFACE,
	CULL_NONE
} cull_mode;

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern int window_width;
extern int window_height;

extern uint32_t* color_buffer;
extern SDL_Texture* color_buffer_texture;

// initialize all SDL components for drawing on screen.
bool initialize_window(void);
void destroy_window(void);

void draw_pixel(int x, int y, uint32_t color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_grid(uint32_t color);
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void draw_rectangle(int xpos, int ypos, int width, int height, uint32_t color);

// draw color buffer to SDL texture
void render_color_buffer(void);
void clear_color_buffer(uint32_t color);

#endif