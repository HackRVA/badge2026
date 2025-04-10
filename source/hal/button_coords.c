#include "sim_lcd_params.h"
#include "button_coords.h"

/* "Landscape" and "Portrait" here refer to the LCD screen, not the badge as a whole */
#define PORTRAITXY(x, y) { ((float) (x) / 647.0f), ((float) (y) / 1024.0f) }
#define LANDSCAPEXY(x, y) { ((float) (x) / 1024.0f), ((float) (y) / 647.0f) }
static const struct button_coord_list rotated_button_coords = {
	.a_button = PORTRAITXY(65, 795),
	.b_button = PORTRAITXY(93, 920),
#if BADGE_HAS_ROTARY_SWITCHES
	.left_rotary = PORTRAITXY(112, 96),
	.right_rotary = PORTRAITXY(960, 88),
#endif
	.dpad_up = PORTRAITXY(227, 117),
	.dpad_down = PORTRAITXY(56, 117),
	.dpad_left = PORTRAITXY(152, 30),
	.dpad_right = PORTRAITXY(152, 224),
	.led = PORTRAITXY(438, 942),
};

static const struct button_coord_list unrotated_button_coords = {
	.a_button = LANDSCAPEXY(798, 583),
	.b_button = LANDSCAPEXY(922, 557),
#if BADGE_HAS_ROTARY_SWITCHES
	.left_rotary = LANDSCAPEXY(91, 912),
	.right_rotary = LANDSCAPEXY(84, 64),
#endif
	.dpad_right = LANDSCAPEXY(228, 503),
	.dpad_left = LANDSCAPEXY(17, 498),
	.dpad_up = LANDSCAPEXY(121, 402),
	.dpad_down = LANDSCAPEXY(118, 603),
	.led = LANDSCAPEXY(937, 207),
};

static struct button_coord_list current_button_coords;

static void adjust_button_coords(struct button_coord *b, int bx1, int by1, float f, int w, int h)
{
	b->x = (int) ((float) bx1 + f * (float) b->x * w);
	b->y = (int) ((float) by1 + f * (float) b->y * h);
}

struct button_coord_list get_button_coords(struct sim_lcd_params *slp, int badge_image_width, int badge_image_height)
{
	float f, sx1, sy1, sx2, bx1, by1;
	struct lcd_to_circuit_board_relation lcdp;


	/* Get corners of the screen inside the badge image */
	if (slp->orientation == SIM_LCD_ORIENTATION_UNROTATED)
		lcdp = unrotated_lcd_to_board();
	else
		lcdp = rotated_lcd_to_board();

	/* coords of sim screen on computer screen */
	sx1 = slp->xoffset;
	sy1 = slp->yoffset;
	sx2 = sx1 + slp->width;

	/* f scales from badge image coords to screen cooords */
        f = (sx2 - sx1) / (lcdp.x2 - lcdp.x1);
        bx1 =   sx1 - f * lcdp.x1;
        by1 =   sy1 - f * lcdp.y1;

	if (slp->orientation == SIM_LCD_ORIENTATION_ROTATED)
		current_button_coords = rotated_button_coords;
	else
		current_button_coords = unrotated_button_coords;
	adjust_button_coords(&current_button_coords.a_button, bx1, by1, f, badge_image_width, badge_image_height);
	adjust_button_coords(&current_button_coords.b_button, bx1, by1, f, badge_image_width, badge_image_height);
#if BADGE_HAS_ROTARY_SWITCHES
	adjust_button_coords(&current_button_coords.left_rotary, bx1, by1, f, badge_image_width, badge_image_height);
	adjust_button_coords(&current_button_coords.right_rotary, bx1, by1, f, badge_image_width, badge_image_height);
#endif
	adjust_button_coords(&current_button_coords.dpad_up, bx1, by1, f, badge_image_width, badge_image_height);
	adjust_button_coords(&current_button_coords.dpad_down, bx1, by1, f, badge_image_width, badge_image_height);
	adjust_button_coords(&current_button_coords.dpad_left, bx1, by1, f, badge_image_width, badge_image_height);
	adjust_button_coords(&current_button_coords.dpad_right, bx1, by1, f, badge_image_width, badge_image_height);
	adjust_button_coords(&current_button_coords.led, bx1, by1, f, badge_image_width, badge_image_height);
	return current_button_coords;
}

