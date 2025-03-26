#include "externals.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 160

#define BUTTON_UP 0x1
#define BUTTON_DOWN 0x2
#define BUTTON_LEFT 0x4
#define BUTTON_RIGHT 0x8
#define BUTTON_A 0x10
#define BUTTON_B 0x20

static uint32_t current_input = 0;

__attribute__((export_name("update"))) void update(int button_mask)
{
	current_input = button_mask;

	if (current_input & BUTTON_B) {
		close_app();
	}
}

__attribute__((export_name("draw"))) void draw(void)
{
	fb_clear();
	palette_draw_grid(0, 0, 8);

	fb_move(80, 50);
	fb_color(palette_get_color_from_index(2));
	fb_rounded_rectangle(40, 40, 4);

	if (current_input & BUTTON_UP) {
		fb_color(palette_get_color_from_index(8));
		fb_filled_circle(64, 20, 10);
	}

	if (current_input & BUTTON_DOWN) {
		fb_color(palette_get_color_from_index(9));
		fb_filled_circle(64, 140, 10);
	}

	if (current_input & BUTTON_LEFT) {
		fb_color(palette_get_color_from_index(10));
		fb_filled_circle(20, 80, 10);
	}

	if (current_input & BUTTON_RIGHT) {
		fb_color(palette_get_color_from_index(11));
		fb_filled_circle(108, 80, 10);
	}

	if (current_input & BUTTON_A) {
		fb_color(palette_get_color_from_index(12));
		fb_filled_circle(30, 30, 8);
	}

	if (current_input & BUTTON_B) {
		fb_color(palette_get_color_from_index(13));
		fb_filled_circle(98, 130, 8);
	}

	fb_swap_buffers();
}

