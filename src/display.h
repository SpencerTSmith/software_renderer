#ifndef DISPLAY_H
#define DISPLAY_H
#include <SDL2/SDL.h>

#include "color.h"
#include <stdbool.h>
#include <stdint.h>

#define FPS 60.0f
#define SECOND 1000.0f
#define FRAME_TARGET_TIME (SECOND / FPS)

// NOTE: might be better to switch this over to a bit mask
typedef enum {
    RENDER_WIRE_FRAME,
    RENDER_WIRE_VERTS,
    RENDER_FILL,
    RENDER_FILL_WIRE,
    RENDER_TEXTURE,
    RENDER_TEXTURE_WIRE,
    RENDER_TEXTURE_PS1
} render_mode_e;

typedef enum { CULL_BACKFACE, CULL_NONE } cull_mode_e;

// initialize all SDL components for drawing on screen.
bool window_init(void);

void get_window_size(int *width, int *height);
// free all resources related to window
void window_free(void);

void draw_pixel(int x, int y, color_t color);
void draw_line(int x0, int y0, int x1, int y1, color_t color);
void draw_grid(color_t color);
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, color_t color);
void draw_rectangle(int xpos, int ypos, int width, int height, color_t color);

float w_buffer_at(int x, int y);
void update_w_buffer(int x, int y, float w);

void set_render_mode(render_mode_e mode);
void switch_cull_mode();

bool should_cull_bface();
bool should_render_wire();
bool should_render_fill();
bool should_render_verts();
bool should_render_texture();
bool should_render_ps1();

// draw color buffer to SDL texture, show the texture
void render_color_buffer(void);

void clear_color_buffer(color_t color);
void clear_w_buffer(void);

#endif
