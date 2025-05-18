#ifndef UI_H
#define UI_H

struct ui_button {
	int x, y;
	int width, height;
	const char *text;
	unsigned short outline_size;
	unsigned short outline_color;
	unsigned short fill_color;
	unsigned short text_color;
};

struct ui_progress_bar {
	int x;
	int y;
	int width;
	int height;
	int outline_size;
	unsigned short fill_color;
	unsigned short empty_color;
	unsigned short outline_color;
	int fill;
};

struct ui_text_box {
	int x, y;
	int width, height;
	unsigned short outline_size;
	unsigned short outline_color;
	unsigned short fill_color;
	unsigned short text_color;
	const char *text;
};

void ui_text_box_fill(struct ui_text_box box);
void ui_text_box_draw_outline(struct ui_text_box box);
void ui_text_box_draw_text(struct ui_text_box box);
void ui_text_box_draw(struct ui_text_box box);

int ui_center_text_x(const char *text, int container_x, int container_width);
int ui_center_text_y(int container_y, int container_height);
/**
 * ui_button_draw is a stateless button element.
 * it's up to the caller to manage state of the button.
 *
 * ui_button_draw just calls ui_button_fill, ui_button_label, and
 * ui_button_outline (in that order).  You can build your own button with the
 * similar function calls.
 *
 * given that it has no internal state, it's effectively a rounded rect with an
 * outline and a label (i.e. text that tries to center itself).
 */
void ui_button_draw(struct ui_button button);
void ui_button_fill(struct ui_button button, unsigned short color);
void ui_button_dither_fill(struct ui_button button, unsigned short color1,
	unsigned short color2, int dither_size);
void ui_button_draw_label(struct ui_button button, unsigned short color);
void ui_button_draw_outline(struct ui_button button, unsigned short color);

void ui_button_draw_label(struct ui_button button, unsigned short color);
void ui_button_draw_outline(struct ui_button button, unsigned short color);
void ui_button_draw(struct ui_button button);

/**
 * ui_progress_bar_calculate_fill_percentage takes an int between 0-100
 * it returns the proper value that should be set in ui_progress_bar.fill
 * ... basically this handles the fixed point math for you.
 *
 * If the argument passed in is something above 100, the progress bar will
 * be entirely filled.
 */
int ui_progress_bar_calculate_fill_percentage(int percent);
void ui_progress_bar_draw_fill(struct ui_progress_bar bar);
void ui_progress_bar_draw_outline(struct ui_progress_bar bar);
void ui_progress_bar_draw(struct ui_progress_bar bar);

#endif
