#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "duktape.h"

#include "button.h"
#include "colors.h"
#include "framebuffer.h"
#include "menu.h"
#include "palette.h"
#include "ui.h"

/*
 * maybe read up on this more
 * https://github.com/svaarala/duktape/blob/master/doc/low-memory.rst#optimizing-code-footprint
 */
#define DUK_USE_LIGHTFUNC_BUILTINS

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

static duk_ret_t native_print(duk_context *ctx)
{
	printf("%s\n", duk_to_string(ctx, 0));
	return 0; /* no return value (= undefined) */
}

static duk_ret_t js_fb_clear(__attribute__((unused)) duk_context *ctx)
{
	FbClear();
	return 0;
}

static duk_ret_t js_fb_color(duk_context *ctx)
{
	int color = duk_require_int(ctx, 0);
	FbColor((uint16_t)color);
	return 0;
}

static duk_ret_t js_fb_move(duk_context *ctx)
{
	int x = duk_require_int(ctx, 0);
	int y = duk_require_int(ctx, 1);
	FbMove(x, y);
	return 0;
}

static duk_ret_t js_fb_point(duk_context *ctx)
{
	int x = duk_require_int(ctx, 0);
	int y = duk_require_int(ctx, 1);
	FbPoint(x, y);
	return 0;
}

static duk_ret_t js_fb_write_string(duk_context *ctx)
{
	const char *str = duk_require_string(ctx, 0);
	FbWriteString(str);
	return 0;
}

static duk_ret_t js_fb_move_relative(duk_context *ctx)
{
	int x = duk_require_int(ctx, 0);
	int y = duk_require_int(ctx, 1);
	FbMoveRelative(x, y);
	return 0;
}

static duk_ret_t js_fb_move_x(duk_context *ctx)
{
	int x = duk_require_int(ctx, 0);
	FbMoveX(x);
	return 0;
}

static duk_ret_t js_fb_move_y(duk_context *ctx)
{
	int y = duk_require_int(ctx, 0);
	FbMoveY(y);
	return 0;
}

static duk_ret_t js_fb_background_color(duk_context *ctx)
{
	int color = duk_require_int(ctx, 0);
	FbBackgroundColor((uint16_t)color);
	return 0;
}

static duk_ret_t js_fb_transparency(duk_context *ctx)
{
	int transparencyMask = duk_require_int(ctx, 0);
	FbTransparency((uint16_t)transparencyMask);
	return 0;
}

static duk_ret_t js_fb_transparent_index(duk_context *ctx)
{
	int color = duk_require_int(ctx, 0);
	FbTransparentIndex((uint16_t)color);
	return 0;
}

static duk_ret_t js_fb_filled_rectangle(duk_context *ctx)
{
	int width = duk_require_int(ctx, 0);
	int height = duk_require_int(ctx, 1);
	FbFilledRectangle(width, height);
	return 0;
}

static duk_ret_t js_fb_horizontal_line(duk_context *ctx)
{
	int x1 = duk_require_int(ctx, 0);
	int y1 = duk_require_int(ctx, 1);
	int x2 = duk_require_int(ctx, 2);
	int y2 = duk_require_int(ctx, 3);
	FbHorizontalLine(x1, y1, x2, y2);
	return 0;
}

static duk_ret_t js_fb_vertical_line(duk_context *ctx)
{
	int x1 = duk_require_int(ctx, 0);
	int y1 = duk_require_int(ctx, 1);
	int x2 = duk_require_int(ctx, 2);
	int y2 = duk_require_int(ctx, 3);
	FbVerticalLine(x1, y1, x2, y2);
	return 0;
}

static duk_ret_t js_fb_line(duk_context *ctx)
{
	int x0 = duk_require_int(ctx, 0);
	int y0 = duk_require_int(ctx, 1);
	int x1 = duk_require_int(ctx, 2);
	int y1 = duk_require_int(ctx, 3);
	FbLine(x0, y0, x1, y1);
	return 0;
}

static duk_ret_t js_fb_rectangle(duk_context *ctx)
{
	int width = duk_require_int(ctx, 0);
	int height = duk_require_int(ctx, 1);
	FbRectangle(width, height);
	return 0;
}

static duk_ret_t js_fb_circle(duk_context *ctx)
{
	int x = duk_require_int(ctx, 0);
	int y = duk_require_int(ctx, 1);
	int r = duk_require_int(ctx, 2);
	FbCircle(x, y, r);
	return 0;
}

static duk_ret_t js_fb_rounded_rect(duk_context *ctx)
{
	int width = duk_require_int(ctx, 0);
	int height = duk_require_int(ctx, 1);
	int stroke = duk_require_int(ctx, 2);
	FbRoundedRect(width, height, stroke);
	return 0;
}

static duk_ret_t js_palette_color(duk_context *ctx)
{
	int index = duk_require_int(ctx, 0);
	duk_push_int(
		ctx, palette_color_from_index(default_palette, (uint8_t)index));
	return 1;
}

static duk_ret_t js_palette_draw_grid(duk_context *ctx)
{
	int grid_x = duk_require_int(ctx, 0);
	int grid_y = duk_require_int(ctx, 1);
	int tile_size = duk_require_int(ctx, 2);
	palette_draw_grid(default_palette, grid_x, grid_y, tile_size);
	return 0;
}

static duk_ret_t js_ui_button_draw(duk_context *ctx)
{
	struct ui_button button;
	duk_get_prop_string(ctx, 0, "x");
	button.x = duk_get_int(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, 0, "y");
	button.y = duk_get_int(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, 0, "width");
	button.width = duk_get_int(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, 0, "height");
	button.height = duk_get_int(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, 0, "text");
	button.text = duk_get_string(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, 0, "outlineSize");
	button.outline_size = duk_get_int(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, 0, "outlineColor");
	button.outline_color = duk_get_int(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, 0, "fillColor");
	button.fill_color = duk_get_int(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, 0, "textColor");
	button.text_color = duk_get_int(ctx, -1);
	duk_pop(ctx);

	ui_button_draw(button);
	return 0;
}

static duk_ret_t js_ui_progress_bar_draw(duk_context *ctx)
{
	struct ui_progress_bar bar;
	duk_get_prop_string(ctx, 0, "x");
	bar.x = duk_get_int(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, 0, "y");
	bar.y = duk_get_int(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, 0, "width");
	bar.width = duk_get_int(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, 0, "height");
	bar.height = duk_get_int(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, 0, "outlineSize");
	bar.outline_size = duk_get_int(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, 0, "fillColor");
	bar.fill_color = duk_get_int(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, 0, "emptyColor");
	bar.empty_color = duk_get_int(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, 0, "outlineColor");
	bar.outline_color = duk_get_int(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, 0, "fillPercentage");
	bar.fill_percentage = duk_get_number(ctx, -1);
	duk_pop(ctx);

	ui_progress_bar_draw(bar);
	return 0;
}

static duk_ret_t js_get_button_state(duk_context *ctx)
{
	int down_latches = button_down_latches();
	duk_push_int(ctx, down_latches);
	return 1;
}

/* TODO: this isn't behaving how i want it to */
static duk_ret_t js_is_button_pressed(duk_context *ctx)
{
	int js_button_mask = 0;
	const char *button_name = duk_require_string(ctx, 0);

	if (strcmp(button_name, "A") == 0) {
		js_button_mask = BADGE_BUTTON_A;
	} else if (strcmp(button_name, "B") == 0) {
		js_button_mask = BADGE_BUTTON_B;
	} else if (strcmp(button_name, "LEFT") == 0) {
		js_button_mask = BADGE_BUTTON_LEFT;
	} else if (strcmp(button_name, "RIGHT") == 0) {
		js_button_mask = BADGE_BUTTON_RIGHT;
	} else if (strcmp(button_name, "UP") == 0) {
		js_button_mask = BADGE_BUTTON_UP;
	} else if (strcmp(button_name, "DOWN") == 0) {
		js_button_mask = BADGE_BUTTON_DOWN;
	} else {
		duk_push_boolean(ctx, 0);
		return 1;
	}

	int down_latches = button_down_latches();
	duk_push_boolean(ctx, BUTTON_PRESSED(js_button_mask, down_latches));
	return 1;
}

static duk_ret_t js_pop_app(__attribute__((unused)) duk_context *ctx)
{
	pop_app();
	return 1;
}

static void duktape_register(duk_context *ctx)
{
	duk_push_global_object(ctx);

	duk_push_c_function(ctx, native_print, 1 /*nargs*/);
	duk_put_global_string(ctx, "print");

	duk_push_object(ctx);
	duk_push_c_function(ctx, js_fb_clear, 0);
	duk_put_prop_string(ctx, -2, "clear");
	duk_push_c_function(ctx, js_fb_color, 1);
	duk_put_prop_string(ctx, -2, "color");
	duk_push_c_function(ctx, js_fb_move, 2);
	duk_put_prop_string(ctx, -2, "move");
	duk_push_c_function(ctx, js_fb_point, 2);
	duk_put_prop_string(ctx, -2, "point");
	duk_push_c_function(ctx, js_fb_write_string, 1);
	duk_put_prop_string(ctx, -2, "writeString");
	duk_push_c_function(ctx, js_fb_move_relative, 2);
	duk_put_prop_string(ctx, -2, "moveRelative");
	duk_push_c_function(ctx, js_fb_move_x, 1);
	duk_put_prop_string(ctx, -2, "moveX");
	duk_push_c_function(ctx, js_fb_move_y, 1);
	duk_put_prop_string(ctx, -2, "moveY");
	duk_push_c_function(ctx, js_fb_background_color, 1);
	duk_put_prop_string(ctx, -2, "backgroundColor");
	duk_push_c_function(ctx, js_fb_transparency, 1);
	duk_put_prop_string(ctx, -2, "transparency");
	duk_push_c_function(ctx, js_fb_transparent_index, 1);
	duk_put_prop_string(ctx, -2, "transparentIndex");
	duk_push_c_function(ctx, js_fb_filled_rectangle, 2);
	duk_put_prop_string(ctx, -2, "filledRectangle");
	duk_push_c_function(ctx, js_fb_horizontal_line, 4);
	duk_put_prop_string(ctx, -2, "horizontalLine");
	duk_push_c_function(ctx, js_fb_vertical_line, 4);
	duk_put_prop_string(ctx, -2, "verticalLine");
	duk_push_c_function(ctx, js_fb_line, 4);
	duk_put_prop_string(ctx, -2, "line");
	duk_push_c_function(ctx, js_fb_rectangle, 2);
	duk_put_prop_string(ctx, -2, "rectangle");
	duk_push_c_function(ctx, js_fb_circle, 3);
	duk_put_prop_string(ctx, -2, "circle");
	duk_push_c_function(ctx, js_fb_rounded_rect, 3);
	duk_put_prop_string(ctx, -2, "roundedRect");
	duk_put_prop_string(ctx, -2, "Fb");
	duk_push_object(ctx);
	duk_push_c_function(ctx, js_palette_color, 1);
	duk_put_prop_string(ctx, -2, "colorFromIndex");
	duk_push_c_function(ctx, js_palette_draw_grid, 3);
	duk_put_prop_string(ctx, -2, "drawGrid");
	duk_put_prop_string(ctx, -2, "Palette");

	duk_push_object(ctx);
	duk_push_c_function(ctx, js_ui_button_draw, 1);
	duk_put_prop_string(ctx, -2, "buttonDraw");
	duk_push_c_function(ctx, js_ui_progress_bar_draw, 1);
	duk_put_prop_string(ctx, -2, "progressBarDraw");
	duk_put_prop_string(ctx, -2, "UI");

	duk_push_object(ctx);
	duk_push_c_function(ctx, js_is_button_pressed, 1);
	duk_put_prop_string(ctx, -2, "isButtonPressed");

	duk_push_c_function(ctx, js_get_button_state, 0);
	duk_put_prop_string(ctx, -2, "getState");

	duk_push_int(ctx, BADGE_BUTTON_A);
	duk_put_prop_string(ctx, -2, "A");
	duk_push_int(ctx, BADGE_BUTTON_B);
	duk_put_prop_string(ctx, -2, "B");
	duk_push_int(ctx, BADGE_BUTTON_LEFT);
	duk_put_prop_string(ctx, -2, "LEFT");
	duk_push_int(ctx, BADGE_BUTTON_RIGHT);
	duk_put_prop_string(ctx, -2, "RIGHT");
	duk_push_int(ctx, BADGE_BUTTON_UP);
	duk_put_prop_string(ctx, -2, "UP");
	duk_push_int(ctx, BADGE_BUTTON_DOWN);
	duk_put_prop_string(ctx, -2, "DOWN");

	duk_put_prop_string(ctx, -2, "Input");

	duk_push_object(ctx);
	duk_push_c_function(ctx, js_pop_app, 0);
	duk_put_prop_string(ctx, -2, "pop");
	duk_put_prop_string(ctx, -2, "App");

	duk_pop(ctx);
}

/*static void check_buttons(void)*/
/*{*/
/*	int down_latches = button_down_latches();*/
/**/
/*	if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down_latches)) {*/
/*	} else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches)) {*/
/*	} else if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches)) {*/
/*	} else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches)) {*/
/*	} else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {*/
/*	} else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {*/
/*	}*/
/*}*/

duk_context *dukrt_create(void)
{
	duk_context *ctx = duk_create_heap_default();
	if (!ctx) {
		printf("Failed to initialize Duktape runtime\n");
		exit(1);
	}

	duktape_register(ctx);
	return ctx;
}

void dukrt_destroy(duk_context *ctx)
{
	if (ctx) {
		duk_destroy_heap(ctx);
	}
}

void dukrt_load_script(duk_context *ctx, const char *js_code)
{
	if (duk_peval_string(ctx, js_code) != 0) {
		printf("Script error: %s\n", duk_safe_to_string(ctx, -1));
	} else {
		if (!duk_is_undefined(ctx, -1)) {
			const char *res_str = duk_safe_to_string(ctx, -1);
			if (res_str && *res_str) {
				printf("Script result: %s\n", res_str);
			}
		}
	}
	duk_pop(ctx);
}

void dukrt_load_script_bytes(
	duk_context *ctx, const unsigned char *js_code, unsigned int length)
{
	if (duk_peval_lstring(ctx, (const char *)js_code, length) != 0) {
		printf("Script error: %s\n", duk_safe_to_string(ctx, -1));
	} else {
		if (!duk_is_undefined(ctx, -1)) {
			const char *res_str = duk_safe_to_string(ctx, -1);
			if (res_str && *res_str) {
				printf("Script result: %s\n", res_str);
			}
		}
	}
	duk_pop(ctx);
}

void dukrt_update(duk_context *ctx)
{
	duk_eval_string(ctx, "if (typeof update === 'function') update();");
	/*check_buttons();*/
}
