#include "display.h"

#include <stdio.h>

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int window_width = 800;
int window_height = 600;

uint32_t* color_buffer = NULL;
float* w_buffer = NULL;
SDL_Texture* color_buffer_texture = NULL;

// initialize all SDL components for drawing on screen.
bool initialize_window(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "Error initializing SDL.\n");
		return false;
	}

	// Fullscreen it
	SDL_DisplayMode display_mode;
	SDL_GetCurrentDisplayMode(0, &display_mode);

	window_width = display_mode.w;
	window_height = display_mode.h;

	window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		window_width, window_height, SDL_WINDOW_BORDERLESS);

	if (!window) {
		fprintf(stderr, "Error creating SDL window.\n");
		return false;
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		fprintf(stderr, "Error creating SDL renderer.\n");
		return false;
	}
	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

	return true;
}

void clear_color_buffer(uint32_t color) {
	for (int y = 0; y < window_height; y++) {
		for (int x = 0; x < window_width; x++) {
			color_buffer[(y * window_width) + x] = color;
		}
	}
}

void clear_w_buffer(void) {
	for (int y = 0; y < window_height; y++) {
		for (int x = 0; x < window_width; x++) {
			w_buffer[(y * window_width) + x] = 0.0f;
		}
	}
}

void draw_pixel(int x, int y, uint32_t color) {
	if (x >= 0 && x < window_width && y >= 0 && y < window_height)
		color_buffer[(window_width * y) + x] = color;
}

void draw_grid(uint32_t color) {
	for (int y = 0; y < window_height; y+=10)
	{
		for (int x = 0; x < window_width; x+=10)
		{
			draw_pixel(x, y, color);
		}
	}
}

// Naive "DDA" implementation
void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
	int delta_x = x1 - x0;
	int delta_y = y1 - y0;
	
	// Only works when longest "side-length" is the one by which you travel the entire length, 
	//	comment out ternary to see what it looks like without... gaps in steep lines
	int side_length = abs(delta_x) >= abs(delta_y) ? abs(delta_x) : abs(delta_y);

	// Per step increase
	float x_inc = delta_x / (float)side_length; // either one or the slope
	float y_inc = delta_y / (float)side_length; // same as above, if above is one this is float, vice versa

	float current_x = x0;
	float current_y = y0;
	for (int i = 0; i <= side_length; i++) {
		draw_pixel(roundf(current_x), roundf(current_y), color);
		current_x += x_inc;
		current_y += y_inc;
	}
}

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
	draw_line(x0, y0, x1, y1, color);
	draw_line(x1, y1, x2, y2, color);
	draw_line(x2, y2, x0, y0, color);
}

// Solid
void draw_rectangle(int xpos, int ypos, int width, int height, uint32_t color) {
	if (xpos + width < window_width && ypos + height < window_height) {
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				// adjust by offset
				draw_pixel(x + xpos, y + ypos, color);
			}
		}
	}
}

// draw color buffer to SDL texture
void render_color_buffer(void) {
	SDL_UpdateTexture(color_buffer_texture, NULL, color_buffer, (int)window_width * sizeof(uint32_t));
	SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

// Free dynamically allocated memory
void destroy_window(void) {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_DestroyTexture(color_buffer_texture);
	SDL_Quit();
}
