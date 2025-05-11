#include <stdio.h>
#include <string.h>

#include "colors.h"
#include "button.h"
#include "framebuffer.h"
#include "carousel_menu_app.h"
#include "default_menu_app.h"
#include "menu_icon.h"
#include "badge.h"

#define CAROUSEL_MENU_FG_COLOR GREEN
#define CAROUSEL_MENU_BG_COLOR BLACK

static struct default_menu_app_context context_stack[MAX_APP_STACK_DEPTH];
static int current_menu_stack_idx = -1;
static struct default_menu_app_context *current_context = NULL;

void default_menu_app_cb(struct badge_app *app);

struct badge_app carousel_menu_app = {
	.app_func = carousel_menu_app_cb,
	.app_context = 0,
	.wake_up = 1,
	/* these are set in do_selection just before calling app_func() */
	.menu = 0,
	.current_selection = 0,
};

/* Program states.  Initial state is DEFAULT_MENU_APP_INIT */
enum carousel_menu_app_state_t {
	CAROUSEL_MENU_APP_INIT,
	CAROUSEL_MENU_APP_RUN,
	CAROUSEL_MENU_APP_EXIT,
};

static enum carousel_menu_app_state_t carousel_menu_app_state = CAROUSEL_MENU_APP_INIT;

static void carousel_menu_app_init(void)
{
	FbInit();
	FbClear();
	carousel_menu_app_state = CAROUSEL_MENU_APP_RUN;
	current_context->screen_changed = 1;
}

static int count_menu_items(struct menu_t *m)
{
	for (int i = 0; ; i++) {
		if (m[i].attrib & LAST_ITEM)
			return i + 1;
	}
}

static int menu_has_icons(struct menu_t *m)
{
	for (int i = 0; ; i++) {
		if (m[i].icon)
			return 1;
		if (m[i].attrib & LAST_ITEM)
			break;
	}
	return 0;
}

static void move_left(void)
{
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
		if (current_menu_stack_idx >= 0)
			current_context = &context_stack[current_menu_stack_idx];
		else
			current_context = carousel_menu_app.app_context;
	}
}

static void display_menu_item_description(struct badge_app *app)
{
	static int screen_changed = 1;

	if (app->wake_up)
		screen_changed = 1;

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
		if (current_menu_stack_idx < MAX_APP_STACK_DEPTH - 1) {
			struct menu_t *submenu = (struct menu_t *)
				&m[current_context->current_item].data.menu[0];
			if (menu_has_icons(submenu))
				app.app_func = carousel_menu_app_cb;
			else
				app.app_func = default_menu_app_cb;
			current_menu_stack_idx++;
			app.wake_up = 1;
			app.app_context = &context_stack[current_menu_stack_idx];
			init_default_menu_app_context(app.app_context, submenu);
			app.menu = m;
			app.current_selection = current_context->current_item;
			push_app(app);
		}
		break;
	case BACK:
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
	if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down_latches)) {
		move_left();
	} else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches)) {
		move_right();
	} else if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches)) {
		go_back();
	} else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches)) {
		move_down();
	} else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
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
	struct menu_icon *icon = item->icon;

	if (!current_context->screen_changed && !screensaver_was_active())
		return;
	screensaver_activity_reset();

	FbColor(CAROUSEL_MENU_FG_COLOR);
	FbBackgroundColor(CAROUSEL_MENU_BG_COLOR);
	FbClear();

	if (icon) {
		FbDrawObject(icon->points, icon->npoints, icon->color, LCD_XSIZE / 2, LCD_YSIZE / 2, 512);
	} else {
		FbMove(LCD_XSIZE / 2 - 40, LCD_YSIZE / 2 - 40);
		FbRectangle(80, 80);
	}
	int len = strlen(item->name);
	FbMove((LCD_XSIZE - 8 * len) / 2, LCD_YSIZE - 16);
	FbWriteString(item->name);

	FbSwapBuffers();
	current_context->screen_changed = 0;
}

static void carousel_menu_app_run(void)
{
	check_buttons();
	draw_screen();
}

static void carousel_menu_app_exit(void)
{
	carousel_menu_app_state = CAROUSEL_MENU_APP_INIT; /* So that when we start again, we do not immediately exit */
	(void) pop_app();
}

void carousel_menu_app_cb(struct badge_app *app)
{
	current_context = app->app_context;
	if (app->wake_up)
		current_context->screen_changed = 1;

	switch (carousel_menu_app_state) {
	case CAROUSEL_MENU_APP_INIT:
		carousel_menu_app_init();
		break;
	case CAROUSEL_MENU_APP_RUN:
		carousel_menu_app_run();
		break;
	case CAROUSEL_MENU_APP_EXIT:
		carousel_menu_app_exit();
		break;
	default:
		break;
	}
}

