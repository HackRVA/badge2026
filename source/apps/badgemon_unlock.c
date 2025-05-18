#include <stdbool.h>
#include <stdint.h>

#include "menu.h"
#include "button.h"
#include "framebuffer.h"
#include "random.h"
#include "ui.h"
#include "xorshift.h"
#include "palette.h"
#include "colors.h"
#include "rtc.h"

enum badgemon_unlock_state_t {
	BADGEMON_UNLOCK_INIT = 0,
	BADGEMON_UNLOCK_RUN,
	BADGEMON_UNLOCK_EXIT,
};
static enum badgemon_unlock_state_t badgemon_unlock_state = BADGEMON_UNLOCK_INIT;
static struct palette default_palette = {
	.colors =
		{
			PACKRGB888(254, 0, 0),
			PACKRGB888(127, 36, 84),
			PACKRGB888(28, 43, 83),
			PACKRGB888(0, 135, 81),
			PACKRGB888(171, 82, 54),
			PACKRGB888(96, 88, 79),
			PACKRGB888(195, 195, 198),
			PACKRGB888(255, 241, 233),
			PACKRGB888(237, 27, 81),
			PACKRGB888(250, 162, 27),
			PACKRGB888(247, 236, 47),
			PACKRGB888(93, 187, 77),
			PACKRGB888(81, 166, 220),
			PACKRGB888(131, 118, 156),
			PACKRGB888(241, 118, 166),
			PACKRGB888(252, 204, 171),
		},
};

static uint8_t current_color_index = 0;

#define message_box_height 24
#define message_box_outline_size 3
#define message_box_x_padding 2
#define message_box_width (LCD_XSIZE - message_box_x_padding * 2)
#define message_box_x ((LCD_XSIZE - message_box_width) / 2)

static struct ui_button message_box = {
	.x = message_box_x,
	.y = (LCD_YSIZE / 2) - message_box_height + (message_box_outline_size * 2),
	.width = message_box_width,
	.height = message_box_height,
	.text = "badgemon unlocked!",
	.outline_size = message_box_outline_size,
};

static bool screen_changed = false;

static void check_buttons(void)
{
	int down_latches = button_down_latches();

	if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches))
		badgemon_unlock_state = BADGEMON_UNLOCK_EXIT;
	else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches))
		badgemon_unlock_state = BADGEMON_UNLOCK_EXIT;
	else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches))
		badgemon_unlock_state = BADGEMON_UNLOCK_EXIT;
	else if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down_latches))
		badgemon_unlock_state = BADGEMON_UNLOCK_EXIT;
	else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches))
		badgemon_unlock_state = BADGEMON_UNLOCK_EXIT;
	else if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches))
		badgemon_unlock_state = BADGEMON_UNLOCK_EXIT;
	else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches))
		badgemon_unlock_state = BADGEMON_UNLOCK_EXIT;
	else if (BUTTON_PRESSED(BADGE_BUTTON_RECORD, down_latches))
		badgemon_unlock_state = BADGEMON_UNLOCK_EXIT;
	else if (BUTTON_PRESSED(BADGE_BUTTON_PLAY, down_latches))
		badgemon_unlock_state = BADGEMON_UNLOCK_EXIT;
	else if (BUTTON_PRESSED(BADGE_BUTTON_FASTFORWARD, down_latches))
		badgemon_unlock_state = BADGEMON_UNLOCK_EXIT;
	else if (BUTTON_PRESSED(BADGE_BUTTON_REWIND, down_latches))
		badgemon_unlock_state = BADGEMON_UNLOCK_EXIT;

	return;
}

static void draw_screen(void)
{
	/* palette_draw_grid(default_palette,0, 0, 8); */

	ui_button_fill(message_box, PACKRGB888(250,250,250));
	ui_button_draw_label(message_box, palette_color_from_index(default_palette, current_color_index+1));
	ui_button_draw_outline(message_box, palette_color_from_index(default_palette, current_color_index));

	FbColor(WHITE);
	char *press_any = "press any button";
	char *to_continue = "to continue";

	FbMove(ui_center_text_x(press_any, 0, LCD_XSIZE), 100);
	FbWriteString(press_any);

	FbMove(ui_center_text_x(to_continue, 0, LCD_XSIZE), 112);
	FbWriteString(to_continue);

	if (!screen_changed)
		return;

	FbSwapBuffers();
	screen_changed = false;
}

static void badgemon_init(void)
{
	FbInit();
	FbClear();
	badgemon_unlock_state = BADGEMON_UNLOCK_RUN;
	screen_changed = 1;
}

void badgemon_unlock_cb(__attribute__((unused)) struct menu_t *m)
{
	screen_changed = true;
	check_buttons();
	switch (badgemon_unlock_state) {
	case BADGEMON_UNLOCK_INIT:
		badgemon_init();
		break;
	case BADGEMON_UNLOCK_RUN: {
		uint8_t next_color_index = (rtc_get_ms_since_boot() / 1000) % 16;
		if (next_color_index != current_color_index)
			screen_changed = 1;
 
		current_color_index = next_color_index;
		draw_screen();
		break;
	}
	case BADGEMON_UNLOCK_EXIT:
		badgemon_unlock_state = BADGEMON_UNLOCK_INIT;
		pop_app();
		break;
	default:
		break;
	}
}
