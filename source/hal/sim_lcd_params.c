#include "sim_lcd_params.h"
#include "framebuffer.h"
#include <stdio.h>

static const struct lcd_to_circuit_board_relation landscape_lcd_mapping = {
	/* corners of the screen inside the badge image */
	.x1 = 316,
	.y1 = 57,
	.x2 = 721,
	.y2 = 319,
};

static const struct lcd_to_circuit_board_relation portrait_lcd_mapping = {
	/* corners of the screen inside the badge image */
	.x1 = 205,
	.y1 = 321,
	.x2 = 497,
	.y2 = 706,
};

static const struct sim_lcd_params initial_default_sim_lcd_params = {
	.orientation = SIM_LCD_ORIENTATION_PORTRAIT,
	/* Landscape and portrait refer to the orientation of the LCD screen, not the badge as a whole */
	.xoffset = 1,
	.yoffset = 1,
	.width = LCD_YSIZE * 3,
	.height = LCD_XSIZE * 3,
};

static const struct sim_lcd_params initial_default_landscape_sim_lcd_params = {
	.orientation = SIM_LCD_ORIENTATION_LANDSCAPE, /* LCD screen orientation */
	.xoffset = 1,
	.yoffset = 1,
	.width = LCD_XSIZE * 3,
	.height = LCD_YSIZE * 3,
};

static struct sim_lcd_params default_sim_lcd_params;
static struct sim_lcd_params default_rotated_sim_lcd_params;
static struct sim_lcd_params sim_lcd_params;

void init_sim_lcd_params(void)
{
	default_sim_lcd_params = initial_default_landscape_sim_lcd_params;
	default_rotated_sim_lcd_params = initial_default_sim_lcd_params;
	sim_lcd_params = default_sim_lcd_params;
}

void set_sim_lcd_params(struct sim_lcd_params *params)
{
	sim_lcd_params = *params;
}

struct sim_lcd_params get_sim_lcd_params(void)
{
	return sim_lcd_params;
}

void set_sim_lcd_params_default(void)
{
	sim_lcd_params = default_sim_lcd_params;
}

void adjust_sim_lcd_params_defaults(int sdl_window_width, int sdl_window_height)
{
	if (default_sim_lcd_params.width < sdl_window_width) {
		int extra = sdl_window_width - default_sim_lcd_params.width;
		default_sim_lcd_params.xoffset = (extra / 2);
	}

	if (default_sim_lcd_params.height < sdl_window_height) {
		int extra = sdl_window_height - default_sim_lcd_params.height;
		default_sim_lcd_params.yoffset = extra / 2;
	}

	if (default_rotated_sim_lcd_params.width < sdl_window_width) {
		int extra = sdl_window_width - default_rotated_sim_lcd_params.width;
		default_rotated_sim_lcd_params.xoffset = (extra / 2);
	}

	if (default_rotated_sim_lcd_params.height < sdl_window_height) {
		int extra = sdl_window_height - default_rotated_sim_lcd_params.height;
		default_rotated_sim_lcd_params.yoffset = extra / 2;
	}
}

void set_sim_lcd_params_unrotated(void)
{
	sim_lcd_params = default_sim_lcd_params;
}

void set_sim_lcd_params_rotated(void)
{
	sim_lcd_params = default_rotated_sim_lcd_params;
}

struct lcd_to_circuit_board_relation rotated_lcd_to_board(void)
{
	return portrait_lcd_mapping;
}

struct lcd_to_circuit_board_relation unrotated_lcd_to_board(void)
{
	return landscape_lcd_mapping;
}

