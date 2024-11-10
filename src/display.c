#include "display.h"

#include <SDL2/SDL_video.h>
#include <stdio.h>

#define PIXEL_SCALING_FACTOR 2

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static int window_width = 1280;
static int window_height = 720;

static color_t *color_buffer = NULL;
static float *w_buffer = NULL;
static SDL_Texture *color_buffer_texture = NULL;
static render_mode_e render_mode = RENDER_WIRE_FRAME;
static cull_mode_e cull_mode = CULL_BACKFACE;

// initialize all SDL components for drawing on screen.
bool window_init(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }

    // Fullscreen it
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);

    int full_window_width = display_mode.w;
    int full_window_height = display_mode.h;
    window_width = full_window_width / PIXEL_SCALING_FACTOR;
    window_height = full_window_height / PIXEL_SCALING_FACTOR;

    window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              full_window_width, full_window_height, SDL_WINDOW_BORDERLESS);

    if (!window) {
        fprintf(stderr, "Error creating SDL window.\n");
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error creating SDL renderer.\n");
        return false;
    }

    // Memory for color buffer
    color_buffer = (color_t *)malloc(sizeof(color_t) * window_width * window_height);
    if (!color_buffer) {
        fprintf(stderr, "Error creating color buffer.\n");
        return false;
    }

    // Memory for depth (inverse w) buffer
    w_buffer = (float *)malloc(sizeof(float) * window_width * window_height);
    if (!w_buffer) {
        fprintf(stderr, "Error creating depth buffer.\n");
        return false;
    }

    // SDL texture for rendering buffer from memory
    color_buffer_texture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);
    if (!color_buffer) {
        fprintf(stderr, "Error creating color buffer texture.\n");
        return false;
    }

    return true;
}

void get_window_size(int *width, int *height) {
    *width = window_width;
    *height = window_height;
}

void clear_color_buffer(color_t color) {
    for (int i = 0; i < window_height * window_width; i++) {
        color_buffer[i] = color;
    }
}

void clear_w_buffer(void) {
    for (int i = 0; i < window_height * window_width; i++) {
        w_buffer[i] = 0.0f;
    }
}

void draw_pixel(int x, int y, color_t color) {
    if (x < 0 || x >= window_width || y < 0 || y >= window_height)
        return;

    color_buffer[(window_width * y) + x] = color;
}

void draw_grid(color_t color) {
    for (int y = 0; y < window_height; y += 10) {
        for (int x = 0; x < window_width; x += 10) {
            draw_pixel(x, y, color);
        }
    }
}

// Naive "DDA" implementation
void draw_line(int x0, int y0, int x1, int y1, color_t color) {
    int delta_x = x1 - x0;
    int delta_y = y1 - y0;

    // Only works when longest "side-length" is the one by which you travel the entire length,
    // comment out ternary to see what it looks like without... gaps in steep lines
    int side_length = abs(delta_x) >= abs(delta_y) ? abs(delta_x) : abs(delta_y);

    // Per step increase
    float x_inc = delta_x / (float)side_length; // either one or the slope
    float y_inc = delta_y / (float)side_length; // same as above

    float current_x = x0;
    float current_y = y0;
    for (int i = 0; i <= side_length; i++) {
        draw_pixel(roundf(current_x), roundf(current_y), color);
        current_x += x_inc;
        current_y += y_inc;
    }
}

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, color_t color) {
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color);
}

// Solid
void draw_rectangle(int xpos, int ypos, int width, int height, color_t color) {
    if (xpos + width >= window_width || ypos + height >= window_height)
        return;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // adjust by offset
            draw_pixel(x + xpos, y + ypos, color);
        }
    }
}

float w_buffer_at(int x, int y) {
    if (x >= window_width || y >= window_height)
        return 0.0f;
    return w_buffer[(y * window_width) + x];
}

void update_w_buffer(int x, int y, float new_value) {
    if (x >= window_width || y >= window_height)
        return;

    w_buffer[(y * window_width) + x] = new_value;
}

void render_color_buffer(void) {
    SDL_UpdateTexture(color_buffer_texture, NULL, color_buffer, window_width * sizeof(color_t));
    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void set_render_mode(render_mode_e mode) { render_mode = mode; }

void switch_cull_mode() { cull_mode = cull_mode == CULL_BACKFACE ? CULL_NONE : CULL_BACKFACE; }

bool should_cull_bface() { return cull_mode == CULL_BACKFACE; }
bool should_render_wire() {
    return (render_mode == RENDER_WIRE_FRAME || render_mode == RENDER_WIRE_VERTS ||
            render_mode == RENDER_FILL_WIRE || render_mode == RENDER_TEXTURE_WIRE);
}
bool should_render_fill() {
    return (render_mode == RENDER_FILL_WIRE || render_mode == RENDER_FILL);
}
bool should_render_verts() { return (render_mode == RENDER_WIRE_VERTS); }
bool should_render_texture() {
    return (render_mode == RENDER_TEXTURE || render_mode == RENDER_TEXTURE_WIRE);
}
bool should_render_ps1() { return (render_mode == RENDER_TEXTURE_PS1); }

// Free all window related resources
void window_free(void) {
    free(color_buffer);
    free(w_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(color_buffer_texture);
    SDL_Quit();
}
