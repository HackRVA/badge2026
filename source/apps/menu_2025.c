#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "colors.h"
#include "button.h"
#include "framebuffer.h"
#include "menu_2025.h"
#include "default_menu_app.h"
#include "menu_icon.h"
#include "cassettepixel.h"
#include "cassettedrawer.h"

#define TAPE_DECK_MENU_FG_COLOR WHITE
#define TAPE_DECK_MENU_BG_COLOR x11_red2
#define TAPE_DECK_SHELL PACKRGB888(170, 171, 163)

static struct default_menu_app_context context_stack[MAX_APP_STACK_DEPTH];
static int current_menu_stack_idx = -1;
static struct default_menu_app_context *current_context = NULL;

void default_menu_app_cb(struct badge_app *app);

struct badge_app tape_deck_menu_app = {
	.app_func = tape_deck_menu_app_cb,
	.app_context = 0,
	.wake_up = 1,
	/* these are set in do_selection just before calling app_func() */
	.menu = 0,
	.current_selection = 0,
};

/* Program states.  Initial state is DEFAULT_MENU_APP_INIT */
enum tape_deck_menu_app_state_t {
	TAPE_DECK_MENU_APP_INIT,
	TAPE_DECK_MENU_APP_RUN,
	TAPE_DECK_MENU_APP_EXIT,
};


static const uint16_t tape_deck_bg_colors[16] = {
	PACKRGB888(226, 160, 78),
	PACKRGB888(208, 230, 77),
	PACKRGB888(42, 97, 230),
	PACKRGB888(44, 55, 55),
	PACKRGB888(237, 93, 169),
	PACKRGB888(204, 79, 79),
	PACKRGB888(243, 219, 87),
	PACKRGB888(14, 24, 83),
	PACKRGB888(243, 188, 65),
	PACKRGB888(79, 150, 96),
	PACKRGB888(151, 195, 104),
	PACKRGB888(208, 70, 46),
	PACKRGB888(21, 37, 118),
	PACKRGB888(52, 120, 68),
	PACKRGB888(62, 136, 127),
	PACKRGB888(236, 122, 100),
};

static enum tape_deck_menu_app_state_t tape_deck_menu_app_state = TAPE_DECK_MENU_APP_INIT;
static int animation_step = 5;
static enum tape_deck_animation_direction  {
	anim_up,
	anim_right,
	anim_down,
	anim_left,
} anim_direction = anim_right;

static void tape_deck_menu_app_init(void)
{
	FbInit();
	FbClear();
	tape_deck_menu_app_state = TAPE_DECK_MENU_APP_RUN;
	current_context->screen_changed = 1;
}

static int count_menu_items(struct menu_t *m)
{
	for (int i = 0; ; i++) {
		if (m[i].attrib & LAST_ITEM)
			return i + 1;
	}
}

static int is_tape_menu(struct menu_t *m)
{
	for (int i = 0; ; i++) {
		if (m[i].attrib & TAPE_DECK)
			return 1;
		if (m[i].attrib & LAST_ITEM)
			break;
	}
	return 0;
}

static void move_left(void)
{
	anim_direction = anim_left;
	animation_step = 0;
	if (current_context->current_item > 0) {
		current_context->current_item--;
		/* skip "back" items */
		if (current_context->menu[current_context->current_item].type == BACK)
			current_context->current_item--;
		if (current_context->top_item > current_context->current_item)
			current_context->top_item--;
		current_context->screen_changed = 1;
	} else { /* wrap around to the end */
		int nitems = count_menu_items(current_context->menu);
		current_context->current_item = nitems - 1;
		/* skip "back" items */
		if (current_context->menu[current_context->current_item].type == BACK)
			current_context->current_item--;
	}
}

static void move_right(void)
{
	int nitems = count_menu_items(current_context->menu);
	anim_direction = anim_right;
	animation_step = 0;
	if (current_context->current_item < nitems - 1) {
		current_context->current_item++;

		/* skip "back" items */
		if (current_context->menu[current_context->current_item].type == BACK)
			current_context->current_item++;
		if (current_context->current_item >= nitems)
			current_context->current_item = 0; /* wrap around */

		if (current_context->top_item < current_context->current_item - 15)
			current_context->top_item++;
		current_context->screen_changed = 1;
	} else {
		current_context->current_item = 0;
		/* skip "back" items */
		if (current_context->menu[current_context->current_item].type == BACK)
			current_context->current_item++;
		if (current_context->current_item >= nitems)
			current_context->current_item = 0; /* wrap around */
		current_context->screen_changed = 1;
	}
}

static void go_back(void)
{
	if (current_menu_stack_idx > -1) {
		current_menu_stack_idx--;
		pop_app();
		anim_direction = anim_up;
		animation_step = 0;
		if (current_menu_stack_idx >= 0)
			current_context = &context_stack[current_menu_stack_idx];
		else
			current_context = tape_deck_menu_app.app_context;
	}
}

static void display_menu_item_description(struct badge_app *app)
{
	static int screen_changed = 1;

	if (app->wake_up) {
		screen_changed = 1;
		app->wake_up = 0;
	}

	if (screen_changed) {
		struct menu_t *m = app->app_context;
		FbColor(CYAN);
		FbBackgroundColor(BLACK);
		FbClear();
		FbMove(0, 0);
		FbWriteString(m->data.description);
		FbSwapBuffers();
		screen_changed = 0;
	}

	int down_latches = button_down_latches();
	if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches) ||
		BUTTON_PRESSED(BADGE_BUTTON_B, down_latches) ||
		BUTTON_PRESSED(BADGE_BUTTON_LEFT, down_latches) ||
		BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches) ||
		BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches) ||
		BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches)) {
		screen_changed = 1;
		pop_app();
	}
}

static void do_selection(void)
{
	struct menu_t *m = current_context->menu;
	enum menu_item_type t = m[current_context->current_item].type;
	struct badge_app app;

	switch (t) {
	case MENU:
		anim_direction = anim_down;
		animation_step = 0;
		if (current_menu_stack_idx < MAX_APP_STACK_DEPTH - 1) {
			struct menu_t *submenu = (struct menu_t *)
				&m[current_context->current_item].data.menu[0];
			if (is_tape_menu(submenu)) {
				app.app_func = tape_deck_menu_app_cb;
				current_menu_stack_idx++;
				app.app_context = &context_stack[current_menu_stack_idx];
			} else {
				app.app_func = default_menu_app_cb;
				/* Don't increment current_menu_stack_idx in this case
				 * because the "back" option will be done in another app
				 * and won't decrement it when it's done, so we want it
				 * to remain the same when we are back via pop_app().
				 */
				app.app_context = &context_stack[current_menu_stack_idx + 1];
			}
			app.wake_up = 1;
			init_default_menu_app_context(app.app_context, submenu);
			app.menu = m;
			app.current_selection = current_context->current_item;
			push_app(app);
		}
		break;
	case BACK:
		anim_direction = anim_up;
		animation_step = 0;
		go_back();
		break;
	case FUNCTION:

		app.app_func = m[current_context->current_item].data.func;
		app.app_context = 0;
		app.wake_up = 1;
		app.menu = m;
		app.current_selection = current_context->current_item;
		push_app(app);
		break;
	case ITEM_DESC:
		app.app_func = display_menu_item_description;
		app.app_context = &m[current_context->current_item];
		app.menu = m;
		app.current_selection = current_context->current_item;
		push_app(app);
		break;
	case TEXT:
		/* Doesn't do anything if selected. */
		/* This is only used in conjunction with SKIP_ITEM attribute anyway. */
		break;
	}
}

/* Select, but only if what is being selected is a menu. */
static void move_down(void)
{
	struct menu_t *m = current_context->menu;
	enum menu_item_type t = m[current_context->current_item].type;

	if (t == MENU)
		do_selection();
}

static void check_buttons(void)
{
    int down_latches = button_down_latches();
	if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down_latches) ||
		BUTTON_PRESSED(BADGE_BUTTON_REWIND, down_latches)) {
		move_left();
	} else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches) ||
		BUTTON_PRESSED(BADGE_BUTTON_FASTFORWARD, down_latches)) {
		move_right();
	} else if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches) ||
		BUTTON_PRESSED(BADGE_BUTTON_STOP_EJECT, down_latches)) {
		go_back();
	} else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches)) {
		move_down();
	} else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches) ||
		BUTTON_PRESSED(BADGE_BUTTON_PLAY, down_latches)) {
		do_selection();
	} else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
		go_back();
	}
	/* skip items tagged to be skipped */
	while (current_context->menu[current_context->current_item].attrib & SKIP_ITEM)
		move_right();
}

static void draw_screen(void)
{
	struct menu_t *m = current_context->menu;
	struct menu_t *item = &m[current_context->current_item];
	int source_x, source_y;

	if (!current_context->screen_changed)
		return;

	FbColor(TAPE_DECK_MENU_FG_COLOR);
	FbBackgroundColor(tape_deck_bg_colors[current_context->current_item % 16]);
	FbClear();

	if (animation_step == 5) {
		FbMove(0, 0);
		int text_x;
		int text_y;

		char s_upper[sizeof(item->name)];
		memcpy(s_upper, item->name, sizeof(item->name));
		int len = strnlen(s_upper, sizeof(s_upper));
		for (int i = 0; i < len; i++) {
			s_upper[i] = toupper(s_upper[i]);
		}
		if (current_menu_stack_idx == -1) { 
			FbImageRect(&cassettedrawer, 0, 0, -14, 0, LCD_XSIZE, LCD_YSIZE, MAGENTA);
			text_x = (LCD_XSIZE - 8 * len) / 2;
			text_y = 87;
			FbBackgroundColor(G_Fb.transIndex);

			FbColor(PACKRGB888(200, 200, 200));
			FbMove(text_x - 1, text_y);
			FbWriteString(s_upper);

			FbColor(WHITE);
			FbMove(text_x + 1, text_y);
			FbWriteString(s_upper);

			FbColor(BLACK);
			FbMove(text_x, text_y);
			FbWriteString(s_upper);
		} else {
			FbImageRect(&cassettepixel, 0, 0, -8, 0, LCD_XSIZE, LCD_YSIZE, MAGENTA);
			text_x = (LCD_XSIZE - 8 * len) / 2;
			text_y = 21;
			FbMove(text_x, text_y);
			FbBackgroundColor(TAPE_DECK_MENU_BG_COLOR);
			FbColor(TAPE_DECK_MENU_FG_COLOR);
			FbWriteString(s_upper);
		}
	} else {
		switch (anim_direction) {
		case anim_up: /* camera moving up, tape moving down */
			source_x = 0;
			source_y = (5 - animation_step) * (LCD_YSIZE / 5);
			break;
		case anim_right: /* camera moving right, tape moving left */
			source_x = (animation_step) * (LCD_XSIZE / 5);
			source_y = 0;
			break;
		case anim_down: /* camera moving down, tape moving up */
			source_x = 0;
			source_y = (animation_step) * (LCD_YSIZE / 5);
			break;
		case anim_left: /* camera moving left, tape moving right */
			source_x = (5 - animation_step) * (LCD_XSIZE / 5);
			source_y = 0;
			break;
		}
		if (current_menu_stack_idx == -1) {
			FbImageRect(&cassettedrawer, 0, 0, source_x + 8, source_y, LCD_XSIZE, LCD_YSIZE, MAGENTA);
		} else {
			FbImageRect(&cassettepixel, 0, 0, source_x + 8, source_y, LCD_XSIZE, LCD_YSIZE, MAGENTA);
		}
		animation_step++;
	}

	FbSwapBuffers();
	current_context->screen_changed = 0;
}

static void tape_deck_menu_app_run(void)
{
	check_buttons();
	draw_screen();
}

static void tape_deck_menu_app_exit(void)
{
	tape_deck_menu_app_state = TAPE_DECK_MENU_APP_INIT; /* So that when we start again, we do not immediately exit */
	(void) pop_app();
}

void tape_deck_menu_app_cb(struct badge_app *app)
{
	current_context = app->app_context;
	if (app->wake_up)
		current_context->screen_changed = 1;

	switch (tape_deck_menu_app_state) {
	case TAPE_DECK_MENU_APP_INIT:
		tape_deck_menu_app_init();
		break;
	case TAPE_DECK_MENU_APP_RUN:
		tape_deck_menu_app_run();
		break;
	case TAPE_DECK_MENU_APP_EXIT:
		tape_deck_menu_app_exit();
		break;
	default:
		break;
	}
}

