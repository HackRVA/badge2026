#include <assert.h>
#include <stdio.h>

#include "button.h"
#include "colors.h"
#include "framebuffer.h"
#include "badge.h"
#include "palette.h"
#include "ui.h"
#include "xorshift.h"
#include "rtc.h"
#include "key_value_storage.h"

#define BEST_SCORE_KEY "2048_BEST"

static enum twenty_forty_state_t {
	TWENTY_FORTY_EIGHT_INIT = 0,
	TWENTY_FORTY_EIGHT_RUN,
	TWENTY_FORTY_EIGHT_SHOW_HELP,
	TWENTY_FORTY_EIGHT_MENU,
	TWENTY_FORTY_EIGHT_POLL_INPUT,
	TWENTY_FORTY_EIGHT_DRAW_SCREEN,
	TWENTY_FORTY_EIGHT_WIN,
	TWENTY_FORTY_EIGHT_GAME_OVER,
	TWENTY_FORTY_EIGHT_EXIT,
} twenty_forty_eight_state = TWENTY_FORTY_EIGHT_INIT;

#define NUM_MENU_ITEMS 4
#define MENU_ITEM_SPACING 30
#define MENU_ITEM_WIDTH 100
#define MENU_ITEM_HEIGHT 20
#define MENU_X (LCD_XSIZE / 2 - MENU_ITEM_WIDTH / 2)
#define MENU_Y (LCD_YSIZE / 2 - MENU_ITEM_HEIGHT / 2)
#define GRID_SIZE 4
#define TILE_MASK 0xF

static bool first_launch = true;
static const char *menu_items[NUM_MENU_ITEMS] = {
	"play",
	"reset",
	"how to play",
	"exit",
};

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

static uint64_t board = 0;
static uint64_t prev_board = 0;
static unsigned int random_num_state = 0;
static unsigned long score = 0;
static unsigned long best_score = 0;
static unsigned long saved_best = 0;	/* last value written to flash */
static bool won_game = false;
static bool keep_playing_after_win = false;

static int current_menu_item = 0;
static bool current_menu_item_selected = false;

static int tile_size;
static int tile_spacing = 2;
static int grid_x;
static int grid_y;

static int screen_changed = 0;

static int tile_scale[GRID_SIZE][GRID_SIZE] = {
	{100,100,100,100},
	{100,100,100,100},
	{100,100,100,100},
	{100,100,100,100},
};

static bool moved_tiles[GRID_SIZE][GRID_SIZE] = {false};

/* per-tile pixel offset from its final cell, decayed to 0 to animate the slide */
static int slide_off_x[GRID_SIZE][GRID_SIZE] = {{0}};
static int slide_off_y[GRID_SIZE][GRID_SIZE] = {{0}};

static int get_tile(int row, int col)
{
	int shift = (row * GRID_SIZE + col) * 4;
	return (board >> shift) & TILE_MASK;
}

static void set_tile(int row, int col, int value)
{
	int shift = (row * GRID_SIZE + col) * 4;
	board &= ~((uint64_t)TILE_MASK << shift);
	board |= (uint64_t)value << shift;
}

static int find_empty_positions(int empty_positions[GRID_SIZE * GRID_SIZE])
{
	int empty_count = 0;
	for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
		if (((board >> (i * 4)) & TILE_MASK) == 0) {
			empty_positions[empty_count++] = i;
		}
	}
	return empty_count;
}

static int random_num(int n)
{
	unsigned int x;

	assert(n != 0);
	x = xorshift(&random_num_state);
	return (int)(x % (unsigned int)n);
}

static void spawn_tile(void)
{
	int empty_positions[GRID_SIZE * GRID_SIZE];
	int empty_count = find_empty_positions(empty_positions);
	if (empty_count == 0)
		return;
	int pos = empty_positions[random_num(empty_count)];
	int value = (random_num(10) < 9) ? 1 : 2;
	board |= (uint64_t)value << (pos * 4);
}

static void reset_moved_tiles(void)
{
	for (int row = 0; row < GRID_SIZE; row++) {
		for (int col = 0; col < GRID_SIZE; col++) {
			moved_tiles[row][col] = false;
			tile_scale[row][col] = 100;
			slide_off_x[row][col] = 0;
			slide_off_y[row][col] = 0;
		}
	}
}

static void reset_game(void)
{
	board = 0;
	score = 0;
	won_game = false;
	keep_playing_after_win = false;
	first_launch = false;
	spawn_tile();
	spawn_tile();
	reset_moved_tiles();
}

/* Slide/merge one line (index 0 == leading edge).  Also reports, for each
 * final line position, which source position the tile there came from, so the
 * caller can animate the slide.  src_pos[i] == -1 for an empty final cell. */
static void slide_and_merge(int *tiles, int *src_pos)
{
	int comp[GRID_SIZE], origin[GRID_SIZE], n = 0;
	int fn = 0, i;

	for (i = 0; i < GRID_SIZE; i++) {
		if (tiles[i] != 0) {
			comp[n] = tiles[i];
			origin[n] = i;
			n++;
		}
	}

	for (i = 0; i < GRID_SIZE; i++) {
		tiles[i] = 0;
		src_pos[i] = -1;
	}

	i = 0;
	while (i < n) {
		if (i + 1 < n && comp[i] == comp[i + 1]) {
			tiles[fn] = comp[i] + 1;
			src_pos[fn] = origin[i + 1];	/* the tile that slides farther */
			score += 1UL << tiles[fn];
			if (score > best_score)
				best_score = score;
			if (tiles[fn] >= 11)
				won_game = true;
			fn++;
			i += 2;
		} else {
			tiles[fn] = comp[i];
			src_pos[fn] = origin[i];
			fn++;
			i += 1;
		}
	}
}

static int tile_pitch(void)
{
	return tile_size + tile_spacing;
}

static void move_left(void)
{
	for (int row = 0; row < GRID_SIZE; row++) {
		int tiles[GRID_SIZE], src[GRID_SIZE];
		for (int col = 0; col < GRID_SIZE; col++)
			tiles[col] = get_tile(row, col);
		slide_and_merge(tiles, src);
		for (int col = 0; col < GRID_SIZE; col++) {
			set_tile(row, col, tiles[col]);
			if (tiles[col] && src[col] != col)
				slide_off_x[row][col] = (src[col] - col) * tile_pitch();
		}
	}
}

static void move_right(void)
{
	for (int row = 0; row < GRID_SIZE; row++) {
		int tiles[GRID_SIZE], src[GRID_SIZE];
		for (int col = 0; col < GRID_SIZE; col++)
			tiles[GRID_SIZE - 1 - col] = get_tile(row, col);
		slide_and_merge(tiles, src);
		for (int line = 0; line < GRID_SIZE; line++) {
			int col = GRID_SIZE - 1 - line;
			set_tile(row, col, tiles[line]);
			if (tiles[line] && src[line] != line) {
				int src_col = GRID_SIZE - 1 - src[line];
				slide_off_x[row][col] = (src_col - col) * tile_pitch();
			}
		}
	}
}

static void move_up(void)
{
	for (int col = 0; col < GRID_SIZE; col++) {
		int tiles[GRID_SIZE], src[GRID_SIZE];
		for (int row = 0; row < GRID_SIZE; row++)
			tiles[row] = get_tile(row, col);
		slide_and_merge(tiles, src);
		for (int row = 0; row < GRID_SIZE; row++) {
			set_tile(row, col, tiles[row]);
			if (tiles[row] && src[row] != row)
				slide_off_y[row][col] = (src[row] - row) * tile_pitch();
		}
	}
}

static void move_down(void)
{
	for (int col = 0; col < GRID_SIZE; col++) {
		int tiles[GRID_SIZE], src[GRID_SIZE];
		for (int row = 0; row < GRID_SIZE; row++)
			tiles[GRID_SIZE - 1 - row] = get_tile(row, col);
		slide_and_merge(tiles, src);
		for (int line = 0; line < GRID_SIZE; line++) {
			int row = GRID_SIZE - 1 - line;
			set_tile(row, col, tiles[line]);
			if (tiles[line] && src[line] != line) {
				int src_row = GRID_SIZE - 1 - src[line];
				slide_off_y[row][col] = (src_row - row) * tile_pitch();
			}
		}
	}
}

static bool is_game_over(void)
{
	for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
		if (((board >> (i * 4)) & TILE_MASK) == 0)
			return false;
		int row = i / GRID_SIZE, col = i % GRID_SIZE;
		if (col < GRID_SIZE - 1 &&
			get_tile(row, col) == get_tile(row, col + 1))
			return false;
		if (row < GRID_SIZE - 1 &&
			get_tile(row, col) == get_tile(row + 1, col))
			return false;
	}
	return true;
}

static void previous_menu_item(void)
{
	current_menu_item--;
	if (current_menu_item < 0)
		current_menu_item = NUM_MENU_ITEMS - 1;
}

static void next_menu_item(void)
{
	current_menu_item++;
	if (current_menu_item >= NUM_MENU_ITEMS)
		current_menu_item = 0;
}

static void handle_menu_options(void)
{
	switch (current_menu_item) {
	case 0:
		twenty_forty_eight_state = TWENTY_FORTY_EIGHT_RUN;
		if (first_launch)
			reset_game();
		break;
	case 1:
		reset_game();
		twenty_forty_eight_state = TWENTY_FORTY_EIGHT_RUN;
		break;
	case 2:
		twenty_forty_eight_state = TWENTY_FORTY_EIGHT_SHOW_HELP;
		break;
	case 3:
		twenty_forty_eight_state = TWENTY_FORTY_EIGHT_EXIT;
		break;
	default:
		break;
	}
}

/* Persist the best score to flash, but only when it actually changed -- flash
 * wears out if written every frame, so this is called at game-end boundaries. */
static void persist_best_score(void)
{
	if (best_score != saved_best) {
		flash_kv_store_int(BEST_SCORE_KEY, (int)best_score);
		saved_best = best_score;
	}
}

static void twenty_forty_eight_init(void)
{
	int stored;

	FbInit();
	FbClear();
	current_menu_item = 0;
	random_num_state = (unsigned int)rtc_get_ms_since_boot();
	if (random_num_state == 0)
		random_num_state = 0x20482048;

	if (flash_kv_get_int(BEST_SCORE_KEY, &stored) && stored > 0) {
		best_score = (unsigned long)stored;
		saved_best = best_score;
	}

	twenty_forty_eight_state = TWENTY_FORTY_EIGHT_MENU;
	screen_changed = 1;
	first_launch = true;

	tile_size = 24;
	grid_x = (LCD_XSIZE -
			 (tile_size * GRID_SIZE +
				 tile_spacing * (GRID_SIZE - 1))) /
		2;
	grid_y = 22;
}

static void move_tiles(void (*move_func)(void))
{
	reset_moved_tiles();
	prev_board = board;
	move_func();

	if (board != prev_board) {
		spawn_tile();
		for (int row = 0; row < GRID_SIZE; row++) {
			for (int col = 0; col < GRID_SIZE; col++) {
				if ((int)get_tile(row, col) !=
					(int)((prev_board >>
						      ((row * GRID_SIZE + col) *
							      4)) &
						TILE_MASK)) {
					moved_tiles[row][col] = true;
					tile_scale[row][col] = 82;
				}
			}
		}
		screen_changed = 1;
	} else {
		screen_changed = 1;
	}
}

static void check_buttons(void)
{
	int down_latches = button_down_latches();

	if (twenty_forty_eight_state == TWENTY_FORTY_EIGHT_SHOW_HELP) {
		if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down_latches)) {
			twenty_forty_eight_state = TWENTY_FORTY_EIGHT_MENU;
			screen_changed = 1;
		} else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches)) {
			twenty_forty_eight_state = TWENTY_FORTY_EIGHT_MENU;
			screen_changed = 1;
		} else if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches)) {
			twenty_forty_eight_state = TWENTY_FORTY_EIGHT_MENU;
			screen_changed = 1;
		} else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches)) {
			twenty_forty_eight_state = TWENTY_FORTY_EIGHT_MENU;
			screen_changed = 1;
		} else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
			twenty_forty_eight_state = TWENTY_FORTY_EIGHT_MENU;
			screen_changed = 1;
		} else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
			twenty_forty_eight_state = TWENTY_FORTY_EIGHT_MENU;
			screen_changed = 1;
		}
		return;
	}
	if (twenty_forty_eight_state == TWENTY_FORTY_EIGHT_MENU) {
		current_menu_item_selected = false;
		if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches)) {
			previous_menu_item();
			screen_changed = 1;
		} else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches)) {
			next_menu_item();
			screen_changed = 1;
		} else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
			current_menu_item_selected = true;
			handle_menu_options();
			screen_changed = 1;
		} else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
			twenty_forty_eight_state = TWENTY_FORTY_EIGHT_EXIT;
			screen_changed = 1;
		}
		return;
	}
	if (twenty_forty_eight_state == TWENTY_FORTY_EIGHT_WIN) {
		if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
			keep_playing_after_win = true;
			twenty_forty_eight_state = TWENTY_FORTY_EIGHT_RUN;
			screen_changed = 1;
		} else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
			twenty_forty_eight_state = TWENTY_FORTY_EIGHT_MENU;
			screen_changed = 1;
		}
		return;
	}
	if (twenty_forty_eight_state == TWENTY_FORTY_EIGHT_GAME_OVER) {
		if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
			reset_game();
			twenty_forty_eight_state = TWENTY_FORTY_EIGHT_RUN;
			screen_changed = 1;
		} else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
			twenty_forty_eight_state = TWENTY_FORTY_EIGHT_MENU;
			screen_changed = 1;
		}
		return;
	}

	if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down_latches)) {
		move_tiles(move_left);
		screen_changed = 1;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches)) {
		move_tiles(move_right);
		screen_changed = 1;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches)) {
		move_tiles(move_up);
		screen_changed = 1;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches)) {
		move_tiles(move_down);
		screen_changed = 1;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
		twenty_forty_eight_state = TWENTY_FORTY_EIGHT_MENU;
		screen_changed = 1;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
		twenty_forty_eight_state = TWENTY_FORTY_EIGHT_MENU;
		screen_changed = 1;
	}
	prev_board = board;
}

static void draw_menu(void)
{
	for (int i = 0; i < NUM_MENU_ITEMS; i++) {
		int y_offset = (i - current_menu_item) * MENU_ITEM_SPACING;
		struct ui_button button = {
			.x = MENU_X,
			.y = MENU_Y + y_offset,
			.width = MENU_ITEM_WIDTH,
			.height = MENU_ITEM_HEIGHT,
			.text = menu_items[i],
			.outline_size = 1,
			.outline_color =
				palette_color_from_index(default_palette, 12),
			.fill_color =
				palette_color_from_index(default_palette, 2),
			.text_color =
				palette_color_from_index(default_palette, 7),
		};

		if (i == current_menu_item) {
			button.outline_color =
				palette_color_from_index(default_palette, 6);
			button.fill_color = palette_color_from_index(
					default_palette, 13);
			if (current_menu_item_selected)
				button.fill_color = palette_color_from_index(
					default_palette, 5);
		}

		if (button.y < 0 || button.y > LCD_YSIZE) continue;
		ui_button_dither_fill(button, button.fill_color,
			palette_color_from_index(default_palette, 0), 1);
		ui_button_draw_outline(button, button.outline_color);
		ui_button_draw_label(button, button.text_color);
	}
}

static void update_tile_animation(void)
{
	bool animation_triggered = false;

	for (int row = 0; row < GRID_SIZE; row++) {
		for (int col = 0; col < GRID_SIZE; col++) {
			/* ease the tile from its source cell to its final one */
			if (slide_off_x[row][col] != 0) {
				slide_off_x[row][col] /= 3;
				animation_triggered = true;
			}
			if (slide_off_y[row][col] != 0) {
				slide_off_y[row][col] /= 3;
				animation_triggered = true;
			}
			/* pop newly placed / merged tiles up to full size */
			if (moved_tiles[row][col] && tile_scale[row][col] < 100) {
				tile_scale[row][col] += 6;
				if (tile_scale[row][col] > 100)
					tile_scale[row][col] = 100;
				animation_triggered = true;
			}
			if (tile_scale[row][col] == 100)
				moved_tiles[row][col] = false;
		}
	}

	if (animation_triggered)
		screen_changed = 1;
}

static void twenty_forty_eight_update(void)
{
	update_tile_animation();
	twenty_forty_eight_state = TWENTY_FORTY_EIGHT_DRAW_SCREEN;
	if (won_game && !keep_playing_after_win) {
		twenty_forty_eight_state = TWENTY_FORTY_EIGHT_WIN;
		persist_best_score();
	} else if (is_game_over()) {
		twenty_forty_eight_state = TWENTY_FORTY_EIGHT_GAME_OVER;
		persist_best_score();
	}
}

static void draw_game_board(void);

static void draw_game_over_screen(void)
{
	char line[24];
	char *game_over = "Game Over";

	draw_game_board();
	FbPlaceFilledRectangle(18, 38, 124, 52, palette_color_from_index(default_palette, 0));
	FbColor(palette_color_from_index(default_palette, 8));
	FbMove(ui_center_text_x(game_over, 0, LCD_XSIZE),
		44);
	FbWriteString(game_over);
	snprintf(line, sizeof(line), "score %lu", score);
	FbColor(WHITE);
	FbMove(ui_center_text_x(line, 0, LCD_XSIZE), 58);
	FbWriteString(line);
	FbMove(ui_center_text_x("A new  B menu", 0, LCD_XSIZE), 74);
	FbWriteString("A new  B menu");
}

static void draw_win_screen(void)
{
	char line[24];
	char *title = "2048!";

	draw_game_board();
	FbPlaceFilledRectangle(18, 36, 124, 56, palette_color_from_index(default_palette, 0));
	FbColor(palette_color_from_index(default_palette, 10));
	FbMove(ui_center_text_x(title, 0, LCD_XSIZE), 42);
	FbWriteString(title);
	snprintf(line, sizeof(line), "score %lu", score);
	FbColor(WHITE);
	FbMove(ui_center_text_x(line, 0, LCD_XSIZE), 58);
	FbWriteString(line);
	FbMove(ui_center_text_x("A keep going", 0, LCD_XSIZE), 72);
	FbWriteString("A keep going");
	FbMove(ui_center_text_x("B menu", 0, LCD_XSIZE), 82);
	FbWriteString("B menu");
}

static void draw_help_screen(void)
{
	FbClear();

	const char *lines[] = {"use the dpad", "to slide tiles", "",
		"tiles of the", "same value will", "combine", "", "create a",
		"tile with", "value of 2048", "to win", "", "",
		"A btn for menu", "B btn for back"};

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
}

/* Classic 2048 palette: cream for small tiles warming to orange/gold as the
 * values climb, so a glance at the color tells you roughly how far along you
 * are.  Indexed by tile exponent (0 = empty). */
static unsigned short tile_fill_color(int tile_value)
{
	static const unsigned short tile_rgb[16] = {
		PACKRGB888(205, 193, 180),	/* empty   */
		PACKRGB888(238, 228, 218),	/* 2       */
		PACKRGB888(237, 224, 200),	/* 4       */
		PACKRGB888(242, 177, 121),	/* 8       */
		PACKRGB888(245, 149,  99),	/* 16      */
		PACKRGB888(246, 124,  95),	/* 32      */
		PACKRGB888(246,  94,  59),	/* 64      */
		PACKRGB888(237, 207, 114),	/* 128     */
		PACKRGB888(237, 204,  97),	/* 256     */
		PACKRGB888(237, 200,  80),	/* 512     */
		PACKRGB888(237, 197,  63),	/* 1024    */
		PACKRGB888(237, 194,  46),	/* 2048    */
		PACKRGB888(60,   58,  50),	/* 4096+   */
		PACKRGB888(60,   58,  50),
		PACKRGB888(60,   58,  50),
		PACKRGB888(60,   58,  50),
	};

	if (tile_value < 0)
		tile_value = 0;
	if (tile_value > 15)
		tile_value = 15;
	return tile_rgb[tile_value];
}

/* Dark text on the pale low tiles, white once they turn orange. */
static unsigned short tile_text_color(int tile_value)
{
	if (tile_value >= 1 && tile_value <= 2)
		return PACKRGB888(119, 110, 101);
	return WHITE;
}

static void draw_status_bar(void)
{
	char line[32];

	FbColor(WHITE);
	FbMove(2, 2);
	snprintf(line, sizeof(line), "2048  score:%lu", score);
	FbWriteString(line);
	FbColor(palette_color_from_index(default_palette, 6));
	FbMove(2, 12);
	snprintf(line, sizeof(line), "best:%lu", best_score);
	FbWriteString(line);
}

static void draw_game_board(void)
{
	draw_status_bar();
	FbPlaceFilledRectangle(grid_x - tile_spacing, grid_y - tile_spacing,
		tile_size * GRID_SIZE + tile_spacing * (GRID_SIZE + 1),
		tile_size * GRID_SIZE + tile_spacing * (GRID_SIZE + 1),
		PACKRGB888(187, 173, 160));

	for (int row = 0; row < GRID_SIZE; row++) {
		for (int col = 0; col < GRID_SIZE; col++) {
			int tile_value = get_tile(row, col);
			int x = grid_x + col * (tile_size + tile_spacing);
			int y = grid_y + row * (tile_size + tile_spacing);

			int scaled_size =
				(tile_size * tile_scale[row][col]) / 100;

			struct ui_button tile = {
				.x = x + (tile_size - scaled_size) / 2 +
					slide_off_x[row][col],
				.y = y + (tile_size - scaled_size) / 2 +
					slide_off_y[row][col],
				.width = scaled_size,
				.height = scaled_size,
				.outline_size = 1,
				.outline_color = palette_color_from_index(
					default_palette, 0),
				.fill_color = tile_fill_color(tile_value),
				.text_color = tile_text_color(tile_value),
			};

			ui_button_fill(tile, tile.fill_color);
			ui_button_draw_outline(tile, tile.outline_color);

			if (tile_value > 0) {
				char text[8];
				snprintf(text, sizeof(text), "%d",
					1 << tile_value);
				tile.text = text;
				ui_button_draw_label(tile, tile.text_color);
			}
		}
	}
}

static void draw_screen(void)
{
	if (!screen_changed) return;

	FbClear();
	if (twenty_forty_eight_state == TWENTY_FORTY_EIGHT_SHOW_HELP) {
		draw_help_screen();
	}
	if (twenty_forty_eight_state == TWENTY_FORTY_EIGHT_MENU) {
		draw_menu();
	}
	if (twenty_forty_eight_state == TWENTY_FORTY_EIGHT_DRAW_SCREEN) {
		twenty_forty_eight_state = TWENTY_FORTY_EIGHT_RUN;
		draw_game_board();
	}
	if (twenty_forty_eight_state == TWENTY_FORTY_EIGHT_WIN) {
		draw_win_screen();
	}
	if (twenty_forty_eight_state == TWENTY_FORTY_EIGHT_GAME_OVER) {
		draw_game_over_screen();
	}

	FbSwapBuffers();
	screen_changed = 0;
}

static void twenty_forty_eight_exit(void)
{
	persist_best_score();
	twenty_forty_eight_state = TWENTY_FORTY_EIGHT_INIT;
	current_menu_item = 0;
	pop_app();
}

void twenty_forty_eight_cb(struct badge_app *app)
{
	if (app->wake_up) {
		screen_changed = 1;
		app->wake_up = 0;
	}

	switch (twenty_forty_eight_state) {
	case TWENTY_FORTY_EIGHT_INIT:
		twenty_forty_eight_init();
		break;
	case TWENTY_FORTY_EIGHT_RUN:
		twenty_forty_eight_update();
		break;
	case TWENTY_FORTY_EIGHT_SHOW_HELP:
		draw_screen();
		break;
	case TWENTY_FORTY_EIGHT_DRAW_SCREEN:
		draw_screen();
		break;
	case TWENTY_FORTY_EIGHT_WIN:
		draw_screen();
		break;
	case TWENTY_FORTY_EIGHT_MENU:
		draw_screen();
		break;
	case TWENTY_FORTY_EIGHT_GAME_OVER:
		draw_screen();
		break;
	case TWENTY_FORTY_EIGHT_EXIT:
		twenty_forty_eight_exit();
		break;
	default:
		break;
	}
	check_buttons();
}

