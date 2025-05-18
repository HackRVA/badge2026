#include <string.h>

#include "framebuffer.h"
#include "ui.h"

#define PI 3.14159265358979323846
#define CHAR_W 8
#define CHAR_H 8

static void draw_button_fill(struct ui_button button, unsigned short color)
{
	FbColor(color);
	FbMove(button.x + button.outline_size, button.y + button.outline_size);
	FbFilledRectangle(button.width - button.outline_size * 2,
		button.height - button.outline_size * 2);
}

static void draw_button_dither_fill(struct ui_button button,
	unsigned short color1, unsigned short color2, int dither_size)
{
	if (dither_size < 1) {
		dither_size = 1;
	}

	for (int y = 0; y < button.height - button.outline_size * 2;
		y += dither_size) {
		for (int x = 0; x < button.width - button.outline_size * 2;
			x += dither_size) {
			unsigned short current_color =
				(((x / dither_size) + (y / dither_size)) % 2 ==
					0)
				? color1
				: color2;

			FbMove(button.x + button.outline_size + x,
				button.y + button.outline_size + y);
			FbColor(current_color);
			FbFilledRectangle(dither_size, dither_size);
		}
	}
}

int ui_center_text_x(const char *text, int container_x, int container_width)
{
	int text_length = strlen(text);
	return container_x + (container_width / 2) - (text_length * 4);
}

int ui_center_text_y(int container_y, int container_height)
{
	return container_y + (container_height / 2) - 4;
}

static void draw_button_label(struct ui_button button, unsigned short color)
{
	int text_length = strlen(button.text);
	int text_x = ui_center_text_x(button.text, button.x, button.width);
	int text_y = ui_center_text_y(button.y, button.height);

	if (text_length == 1) {
		text_x = text_x + 1;
	}

	FbMove(text_x, text_y);
	FbColor(color);
	int prev_transparent_index = FbGetTransparentIndex();
	/* get the current tranparent index */
	FbTransparentIndex(0);
	FbWriteString(button.text);
	/* reset the transparent index to what it was before we changed it */
	FbTransparentIndex(prev_transparent_index);
}

static void draw_button_outline(struct ui_button button, unsigned short color)
{
	FbColor(color);
	FbMove(button.x, button.y);
	FbRoundedRect(button.width, button.height, button.outline_size);
}

void ui_button_fill(struct ui_button button, unsigned short color)
{
	draw_button_fill(button, color);
}

void ui_button_dither_fill(struct ui_button button, unsigned short color1,
	unsigned short color2, int dither_size)
{
	draw_button_dither_fill(button, color1, color2, dither_size);
}

void ui_button_draw_label(struct ui_button button, unsigned short color)
{
	draw_button_label(button, color);
}

void ui_button_draw_outline(struct ui_button button, unsigned short color)
{
	draw_button_outline(button, color);
}

void ui_button_draw(struct ui_button button)
{
	draw_button_fill(button, button.fill_color);
	draw_button_outline(button, button.outline_color);
	draw_button_label(button, button.text_color);
}

static void draw_progress_bar_fill(struct ui_progress_bar bar)
{
	if (bar.fill < 0)   bar.fill = 0;
	if (bar.fill > 256) bar.fill = 256;

	int inner_w = bar.width  - bar.outline_size * 2;
	int inner_h = bar.height - bar.outline_size * 2;

	int fill_px = (inner_w * bar.fill) / 256;

	FbColor(bar.fill_color);
	FbMove(bar.x + bar.outline_size,
	       bar.y + bar.outline_size);
	FbFilledRectangle(fill_px, inner_h);

	if (fill_px < inner_w) {
		FbColor(bar.empty_color);
		FbMove(bar.x + bar.outline_size + fill_px,
			bar.y + bar.outline_size);
		FbFilledRectangle(inner_w - fill_px, inner_h);
    }
}

static void draw_progress_bar_outline(struct ui_progress_bar bar)
{
	FbColor(bar.outline_color);
	FbMove(bar.x, bar.y);
	FbRoundedRect(bar.width, bar.height, bar.outline_size);
}

int ui_progress_bar_calculate_fill_percentage(int percent)
{
	if (percent < 0)   percent = 0;
	if (percent > 100) percent = 100;
	return (percent * 256 + 50) / 100;
}

void ui_progress_bar_draw_fill(struct ui_progress_bar bar)
{
	draw_progress_bar_fill(bar);
}

void ui_progress_bar_draw_outline(struct ui_progress_bar bar)
{
	draw_progress_bar_outline(bar);
}

void ui_progress_bar_draw(struct ui_progress_bar bar)
{
	draw_progress_bar_fill(bar);
	draw_progress_bar_outline(bar);
}

static void draw_text_box_fill(struct ui_text_box box) {
	FbColor(box.fill_color);
	FbMove(box.x + box.outline_size,
		box.y + box.outline_size);
	FbFilledRectangle(
		box.width  - box.outline_size * 2,
		box.height - box.outline_size * 2);
}

static void draw_text_box_outline(struct ui_text_box box) {
	FbColor(box.outline_color);
	FbMove(box.x, box.y);
	FbRoundedRect(box.width, box.height, box.outline_size);
}


/* draw_text_box_text attempts to word wrap and keep text inside the box */
static void draw_text_box_text(struct ui_text_box box) {
	int inner_w = box.width  - box.outline_size * 2;
	int inner_h = box.height - box.outline_size * 2;
	int max_cols = inner_w / CHAR_W;
	int max_lines = inner_h / CHAR_H;

	/* temporary buffer for one line */
	char linebuf[256];
	int line = 0;
	const char *p = box.text;
	while (*p && line < max_lines) {
		/* skip leading spaces */
		while (*p == ' ') p++;

		const char *word_start = p;
		int last_space_idx = -1;
		int chars = 0;
		/* accumulate until line full or end */
		while (*p && chars < max_cols) {
			if (*p == ' ') {
				last_space_idx = chars;
			}
			linebuf[chars++] = *p++;
		}

		int len;
		if (*p && chars == max_cols && last_space_idx >= 0) {
			/* break at last space */
			len = last_space_idx;
			/* rewind p to after that space */
			p = word_start + last_space_idx + 1;
		} else {
			len = chars;
		}
		linebuf[len] = '\0';

		FbMove(
			box.x + box.outline_size,
			box.y + box.outline_size + line * CHAR_H);
		FbColor(box.text_color);
		int prev_t = FbGetTransparentIndex();
		FbTransparentIndex(0);
		FbWriteString(linebuf);
		FbTransparentIndex(prev_t);

		line++;
	}
}

void ui_text_box_fill(struct ui_text_box box) {
	draw_text_box_fill(box);
}

void ui_text_box_draw_outline(struct ui_text_box box) {
	draw_text_box_outline(box);
}

void ui_text_box_draw_text(struct ui_text_box box) {
	draw_text_box_text(box);
}

void ui_text_box_draw(struct ui_text_box box) {
	draw_text_box_fill(box);
	draw_text_box_outline(box);
	draw_text_box_text(box);
}
