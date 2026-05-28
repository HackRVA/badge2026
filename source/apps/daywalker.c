/*
 * Daywalker - an attempt at a vampire survivors style auto-battler.
 *
 * You move. Weapons fire automatically. Enemies swarm.
 * Kill enemies to drop XP gems. Collect gems to level up.
 * Pick upgrades. Survive as long as you can.
 */

#include <stdbool.h>
#include <stdint.h>

#include "audio.h"
#include "badge.h"
#include "button.h"
#include "colors.h"
#include "framebuffer.h"
#include "menu.h"
#include "palette.h"
#include "ui.h"

#include "daywalker.h"

static const struct palette daywalker_palette = {
	.colors =
	{
		PACKRGB888(0, 0, 0),
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

#define PC(i) palette_color_from_index(daywalker_palette, (i))

#define DEBUG_BEEP_ENABLED 0
static void sfx_debug_beep(uint16_t freq, uint16_t duration)
{
#if DEBUG_BEEP_ENABLED
	audio_out_beep(freq, duration);
#endif
}
static void sfx_menu_select(void)  { sfx_debug_beep(900,  40); }

static void write_string(const char *string)
{
	int prev_transparent_index = FbGetTransparentIndex();
	FbTransparentIndex(0);
	FbWriteString(string);
	FbTransparentIndex(prev_transparent_index);
}

static void draw_title(void)
{
	FbClear();

	struct ui_button title = {
		.x = 8,
		.y = 10,
		.width = LCD_XSIZE - 16,
		.height = 20,
		.outline_size = 1,
		.outline_color = PC(8),
		.fill_color = PC(0),
		.text_color = PC(8),
		.text = "DAYWALKER",
	};
	ui_button_draw(title);

	FbColor(PC(6));
	FbMove(ui_center_text_x("Move to survive.", 0, LCD_XSIZE), 38);
	write_string("Move to survive.");
	FbMove(ui_center_text_x("Weapons auto-fire.", 0, LCD_XSIZE), 50);
	write_string("Weapons auto-fire.");
	FbMove(ui_center_text_x("Collect XP to grow.", 0, LCD_XSIZE), 62);
	write_string("Collect XP to grow.");

	struct ui_button start_btn = {
		.x = 30,
		.y = 100,
		.width = LCD_XSIZE - 60,
		.height = 18,
		.text = "[A] START",
		.outline_size = 1,
		.outline_color = PC(10),
		.fill_color = PC(2),
		.text_color = PC(10),
	};
	ui_button_draw(start_btn);
}

static enum {
	DAYWALKER_INIT = 0,
	DAYWALKER_TITLE,
	DAYWALKER_EXIT,
} daywalker_state;

void daywalker_cb(struct badge_app *app)
{
	if (app->wake_up)
		app->wake_up = 0;

	int down_latches = button_down_latches();

	switch (daywalker_state) {
	case DAYWALKER_INIT:
		FbInit();
		daywalker_state = DAYWALKER_TITLE;
		break;

	case DAYWALKER_TITLE:
		if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
			sfx_menu_select();
			daywalker_state = DAYWALKER_EXIT;
			break;
		}
		if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
			daywalker_state = DAYWALKER_EXIT;
			break;
		}
		draw_title();
		FbSwapBuffers();
		break;

	case DAYWALKER_EXIT:
		daywalker_state = DAYWALKER_INIT;
		pop_app();
		break;
	}
}
