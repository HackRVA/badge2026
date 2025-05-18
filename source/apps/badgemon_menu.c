#include <stdbool.h>
#include <stdio.h>

#include "menu.h"
#include "button.h"
#include "framebuffer.h"
#include "random.h"
#include "ui.h"
#include "xorshift.h"
#include "palette.h"
#include "colors.h"

enum badgemon_state_t {
	BADGEMON_INIT = 0,
	BADGEMON_MONSTER_AVATAR,
	BADGEMON_PROGRESS,
	BADGEMON_TRADE_MONSTERS,
	BADGEMON_MONSTER_INFO,
	BADGEMON_TOP_MENU,
	BADGEMON_HELP_SCREEN,
	BADGEMON_EXIT,
};
static enum badgemon_state_t badgemon_state = BADGEMON_INIT;
static struct palette default_palette = {
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

static bool screen_changed = false;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define MENU_ITEM_SPACING 30
#define MENU_ITEM_WIDTH 120
#define MENU_ITEM_HEIGHT 20
#define MENU_X (LCD_XSIZE / 2 - MENU_ITEM_WIDTH / 2)
#define MENU_Y (LCD_YSIZE / 2 - MENU_ITEM_HEIGHT / 2)

static int current_menu_item = 0;
static bool current_menu_item_selected = false;
static bool  scan_animating   = false;
static uint8_t scanline_animation_y =  0;

static void start_scanline_animation(void);

static void top_menu_action_monsters(void)
{
	badgemon_state = BADGEMON_MONSTER_AVATAR;
}
static void top_menu_action_show_progress_page(void)
{
	badgemon_state = BADGEMON_PROGRESS;
}
static void top_menu_action_trade_monsters(void)
{
	badgemon_state = BADGEMON_TRADE_MONSTERS;
	start_scanline_animation();
}
static void top_menu_action_help_screen(void)
{
	badgemon_state = BADGEMON_HELP_SCREEN;
}
static void top_menu_action_exit(void)
{
	badgemon_state = BADGEMON_EXIT;
}

static const char *menu_items[] = {
	"monsters",
	"progress",
	"trade monsters",
	"how to play",
	"exit",
};

static void (*menu_actions[])(void) = {
	top_menu_action_monsters,
	top_menu_action_show_progress_page,
	top_menu_action_trade_monsters,
	top_menu_action_help_screen,
	top_menu_action_exit,
};
#define NUM_MENU_ITEMS ARRAY_SIZE(menu_items)
static void previous_menu_item(void)
{
	current_menu_item--;
	screen_changed = true;
	if (current_menu_item < 0)
		current_menu_item = NUM_MENU_ITEMS - 1;
}
static void next_menu_item(void)
{
	current_menu_item++;
	screen_changed = true;
	if (current_menu_item >= (int)NUM_MENU_ITEMS)
		current_menu_item = 0;
}
static void handle_options_top_menu(void)
{
	if (current_menu_item >= 0 && current_menu_item < (int)NUM_MENU_ITEMS) {
		menu_actions[current_menu_item]();
	}
}

static void return_to_top_menu(void)
{
	badgemon_state = BADGEMON_TOP_MENU;
	screen_changed = true;
	scan_animating = false;
}

static void check_buttons_top_menu(void)
{
	int down_latches = button_down_latches();

	current_menu_item_selected = false;
	if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches)) {
		previous_menu_item();
	} else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches)) {
		next_menu_item();
	} else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
		current_menu_item_selected = true;
		handle_options_top_menu();
	} else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
		badgemon_state = BADGEMON_EXIT;
	}
	return;
}

static void check_buttons_noop_screen(void)
{
	int down_latches = button_down_latches();

	current_menu_item_selected = false;
	if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches))
		return_to_top_menu();
	else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches))
		return_to_top_menu();
	else if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down_latches))
		return_to_top_menu();
	else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches))
		return_to_top_menu();
	else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches))
		return_to_top_menu();
	else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches))
		return_to_top_menu();
	return;
}

static void draw_top_menu(void)
{
	for (int i = 0; i < (int)NUM_MENU_ITEMS; i++) {
		int y_offset = (i - current_menu_item) * MENU_ITEM_SPACING;
		struct ui_button button = {
			.x = MENU_X,
			.y = MENU_Y + y_offset,
			.width = MENU_ITEM_WIDTH,
			.height = MENU_ITEM_HEIGHT,
			.text = menu_items[i],
			.outline_size = 3,
			.outline_color = palette_color_from_index(default_palette, 13),
			.fill_color = palette_color_from_index(default_palette, 0),
			.text_color = palette_color_from_index(default_palette, 11),
		};

		if (i == current_menu_item) {
			button.outline_color = palette_color_from_index(default_palette, 6);
			if (current_menu_item_selected)
				button.fill_color = palette_color_from_index(default_palette, 5);
		}

		ui_button_dither_fill(button, button.fill_color,
		palette_color_from_index(default_palette, 0), 1);
		ui_button_draw_outline(button, button.outline_color);
		ui_button_draw_label(button, button.text_color);
	}
}

static void start_scanline_animation(void)
{
	scan_animating = true;
	scanline_animation_y = 0;
	screen_changed = true;
}

static void draw_scanline_animation(void)
{
	if (!scan_animating) return;

	FbHorizontalLine(0, scanline_animation_y, LCD_XSIZE - 1, scanline_animation_y);

	if (++scanline_animation_y >= LCD_YSIZE)
		scanline_animation_y = 0;

	screen_changed = true;
}

static void draw_trade_monsters_screen(void)
{
	/* maybe get rid of this evil */
	FbClear();

	const char *lines[] = {
		"trading monsters",
		"",
		"be brave",
		"",
		"point your badge",
		"at another badge",
		"to send/receive",
		"",
		
		"",
		"",
		"<---------->",
		"",
		"",
	};
	/* TODO: could do a cool animation here */

	int y = 16;
	for (size_t i = 0; i < sizeof(lines) / sizeof(lines[0]); i++) {
		y += 8;
		if (lines[i][0] == '\0') {
			/* Skip empty lines for spacing */
			continue;
		}
		FbMove(ui_center_text_x(lines[i], 0, LCD_XSIZE), y);
		FbWriteString(lines[i]);
	}

	if (scan_animating)
		draw_scanline_animation();
}

static void draw_centered_text_page(const char *lines[], size_t num_lines, int start_y, int line_height)
{
	int y = start_y;
	for (size_t i = 0; i < num_lines; i++) {
		y += line_height;
		if (lines[i][0] == '\0') {
			/* Skip empty lines as spacing */
			continue;
		}
		FbMove(ui_center_text_x(lines[i], 0, LCD_XSIZE), y);
		FbWriteString(lines[i]);
	}
}

static void draw_help_screen(void)
{
	FbClear();

	const char *lines[] = {
		"unlock monsters by",
		"interacting with",
		"other badge apps",
		"",
		"you can also",
		"share your starter",
		"with other attendees",
		"",
		"do your best to",
		"collect them all",
		"",
		"can you acquire",
		"most of them?",
		"",
	};
	draw_centered_text_page(lines, sizeof(lines) / sizeof(lines[0]), 0, 8);
}
/*
 * progress screen is possibly an optional screen to
 * how many monsters the player has collected
*/
static void draw_progress_menu(void)
{
	FbClear();

	const char *lines[] = {
		"",
		"",
		"",
		"",
		"",
		"progress menu",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
	};
	draw_centered_text_page(lines, sizeof(lines) / sizeof(lines[0]), 16, 8);
}

static void draw_screen(void)
{
	/* palette_draw_grid(default_palette,0, 0, 8); */
	if (!screen_changed)
		return;
	if (scan_animating) {
		draw_scanline_animation();
		FbSwapBuffers();
		return;
	}

	FbSwapBuffers();
	screen_changed = false;
}

static void badgemon_init(void)
{
	FbInit();
	FbClear();
	badgemon_state = BADGEMON_TOP_MENU;
}

void badgemon_cb(__attribute__((unused)) struct menu_t *m);
static struct badge_app badgemon_avatar_menu = {
	.app_func = (void (*)(struct badge_app *))badgemon_cb,
	.app_context = 0,
	.menu = NULL,
	.current_selection = 0,
	.wake_up = 0,
};

void badgemon_menu_cb(__attribute__((unused)) struct menu_t *m)
{
	screen_changed = true;
	switch (badgemon_state) {
	case BADGEMON_INIT:
		badgemon_init();
		break;
	case BADGEMON_MONSTER_AVATAR:
		screen_changed = true;
		push_app(badgemon_avatar_menu);
		badgemon_state = BADGEMON_TOP_MENU;
		break;
	case BADGEMON_PROGRESS:
		check_buttons_noop_screen();
		draw_progress_menu();
		draw_screen();
		break;
	case BADGEMON_MONSTER_INFO:
		check_buttons_noop_screen();
		draw_help_screen();
		draw_screen();
		break;
	case BADGEMON_TRADE_MONSTERS:
		check_buttons_noop_screen();
		draw_trade_monsters_screen();
		draw_screen();
		break;
	case BADGEMON_HELP_SCREEN:
		check_buttons_noop_screen();
		draw_help_screen();
		draw_screen();
		break;
	case BADGEMON_TOP_MENU:
		check_buttons_top_menu();
		draw_top_menu();
		draw_screen();
		break;
	case BADGEMON_EXIT:
		badgemon_state = BADGEMON_INIT;
		current_menu_item = 0;
		pop_app();
		break;
	default:
		break;
	}
}
