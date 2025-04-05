/**
 * A basic match 3 style game similar to tetris attack.
 *
 * needs some polish, but the fundamentals are there.
 *
 * TODO:
 * - display timer to show that there is a time limit
 * - have a win condition
 * - provide way to increase difficulty (like making the rows generate faster)
 * - balance the gameplay (e.g. is the time limit reasonable, 
 *    should the row generation be slowed down)
 * - multiple rounds?
 *
 * --
 *  Dustin Firebaugh
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "button.h"
#include "colors.h"
#include "framebuffer.h"
#include "menu.h"
#include "palette.h"
#include "rtc.h"
#include "ui.h"
#include "xorshift.h"

static enum puzzle_attack_state_t {
	PUZZLE_ATTACK_INIT = 0,
	PUZZLE_ATTACK_RUN,
	PUZZLE_ATTACK_SHOW_HELP,
	PUZZLE_ATTACK_MENU,
	PUZZLE_ATTACK_WIN_SCREEN,
	PUZZLE_ATTACK_LOSE_SCREEN,
	PUZZLE_ATTACK_EXIT,
} puzzle_attack_state = PUZZLE_ATTACK_INIT;

static unsigned short selected_outline_color;
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

enum BLOCK_TYPE {
	EMPTY_BLOCK = -1,
	CIRCLE_BLOCK,
	SQUARE_BLOCK,
	TRIANGLE_BLOCK,
	HEART_BLOCK,
	STAR_BLOCK,
};

static const uint8_t circle_bitmap[] = {
	0b0000000,
	0b0000000,
	0b0011100,
	0b0111110,
	0b0111110,
	0b0011100,
	0b0000000,
};

static const uint8_t square_bitmap[] = {
	0b0000000,
	0b0000000,
	0b0011110,
	0b0011110,
	0b0011110,
	0b0000000,
	0b0000000,
};

static const uint8_t triangle_bitmap[] = {
	0b0000000,
	0b0001000,
	0b0011100,
	0b0111110,
	0b0000000,
	0b0000000,
	0b0000000,
};

static const uint8_t heart_bitmap[] = {
	0b0110110,
	0b1111111,
	0b1111111,
	0b0111110,
	0b0011100,
	0b0001000,
};

static const uint8_t star_bitmap[] = {
	0b0001000,
	0b0101010,
	0b0011100,
	0b1111111,
	0b0011100,
	0b0101010,
	0b0001000,
};

#define GRID_COLS 6
#define GRID_ROWS 10
#define BLOCK_SIZE 8

static enum BLOCK_TYPE grid[GRID_ROWS][GRID_COLS];

static int cursor_x = 0;
static int cursor_y = 0;
static bool swap_requested = false;
static int score = 0;
static uint tick = 0;
static uint64_t last_tick_time = 0;

#define NUM_MENU_ITEMS 4
#define MENU_ITEM_SPACING 30
#define MENU_ITEM_WIDTH 100
#define MENU_ITEM_HEIGHT 20
#define MENU_X (LCD_XSIZE / 2 - MENU_ITEM_WIDTH / 2)
#define MENU_Y (LCD_YSIZE / 2 - MENU_ITEM_HEIGHT / 2)
static int current_menu_item = 0;
static bool current_menu_item_selected = false;
static const char *menu_items[NUM_MENU_ITEMS] = {
	"play",
	"reset",
	"how to play",
	"exit",
};

static void init_grid(void)
{
	for (int y = 0; y < GRID_ROWS; y++) {
		for (int x = 0; x < GRID_COLS; x++) {
			grid[y][x] = EMPTY_BLOCK;
		}
	}
}

static void shift_grid_up(void)
{
	for (int y = 1; y < GRID_ROWS; y++) {
		for (int x = 0; x < GRID_COLS; x++) {
			grid[y - 1][x] = grid[y][x];
		}
	}
}
static void insert_row(void)
{
	for (int x = 0; x < GRID_COLS; x++) {
		grid[GRID_ROWS - 1][x] = rand() % 5;
	}
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

static void reset_game(void)
{
	puzzle_attack_state = PUZZLE_ATTACK_INIT;
	score = 0;
	/*first_launch = false;*/
}

static void handle_menu_options(void)
{
	switch (current_menu_item) {
	case 0:
		puzzle_attack_state = PUZZLE_ATTACK_RUN;
		/*if (first_launch)*/
		/*  reset_game();*/
		break;
	case 1:
		reset_game();
		break;
	case 2:
		puzzle_attack_state = PUZZLE_ATTACK_SHOW_HELP;
		break;
	case 3:
		puzzle_attack_state = PUZZLE_ATTACK_EXIT;
		break;
	default:
		break;
	}
}

static void check_buttons(void)
{
	int down_latches = button_down_latches();

	if (puzzle_attack_state == PUZZLE_ATTACK_MENU) {
		current_menu_item_selected = false;
		if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches)) {
			previous_menu_item();
		} else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches)) {
			next_menu_item();
		} else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
			current_menu_item_selected = true;
			handle_menu_options();
		} else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
			puzzle_attack_state = PUZZLE_ATTACK_EXIT;
		}
		return;
	}

	if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down_latches) && cursor_x > 0) {
		cursor_x--;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches) &&
		cursor_x < GRID_COLS - 2) {
		cursor_x++;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches) &&
		cursor_y > 0) {
		cursor_y--;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches) &&
		cursor_y < GRID_ROWS - 1) {
		cursor_y++;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
		swap_requested = true;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
		puzzle_attack_state = PUZZLE_ATTACK_MENU;
	}
}

struct Point {
	int16_t x, y;
};

static bool check_matches(void)
{
	bool matched = false;
	int runStart, runLength, currentBlock;
	int matchCount = 0;

	/* horizontal - run–detection loop */
	for (int y = 0; y < GRID_ROWS; y++) {
		runStart = 0;
		currentBlock = grid[y][0];
		for (int x = 1; x <= GRID_COLS; x++) {
			if (x < GRID_COLS && grid[y][x] == currentBlock &&
				currentBlock != EMPTY_BLOCK) {
				continue;
			}
			runLength = x - runStart;
			if (currentBlock != EMPTY_BLOCK && runLength >= 3) {
				matched = true;
				matchCount += runLength;
				for (int k = runStart; k < x; k++) {
					grid[y][k] = EMPTY_BLOCK;
				}
			}
			if (x < GRID_COLS) {
				runStart = x;
				currentBlock = grid[y][x];
			}
		}
	}

	/* vertical - run–detection loop */
	for (int x = 0; x < GRID_COLS; x++) {
		runStart = 0;
		currentBlock = grid[0][x];
		for (int y = 1; y <= GRID_ROWS; y++) {
			if (y < GRID_ROWS && grid[y][x] == currentBlock &&
				currentBlock != EMPTY_BLOCK) {
				continue;
			}
			runLength = y - runStart;
			if (currentBlock != EMPTY_BLOCK && runLength >= 3) {
				matched = true;
				matchCount = runLength;
				for (int k = runStart; k < y; k++) {
					grid[k][x] = EMPTY_BLOCK;
				}
			}
			if (y < GRID_ROWS) {
				runStart = y;
				currentBlock = grid[y][x];
			}
		}
	}

	if (matched) {
		score += matchCount;
	}

	return matched;
}

static void collapse_grid(void)
{
	for (int x = 0; x < GRID_COLS; x++) {
		int writeRow = GRID_ROWS - 1;

		/* copy non-empty blocks down */
		for (int y = GRID_ROWS - 1; y >= 0; y--) {
			if (grid[y][x] != EMPTY_BLOCK) {
				grid[writeRow][x] = grid[y][x];
				if (writeRow != y) {
					grid[y][x] = EMPTY_BLOCK;
				}
				writeRow--;
			}
		}

		/* fill remaining cells in column with EMPTY_BLOCK */
		for (int y = writeRow; y >= 0; y--) {
			grid[y][x] = EMPTY_BLOCK;
		}
	}
}

static bool is_top_row_populated(void)
{
	for (int x = 0; x < GRID_COLS; x++) {
		if (grid[0][x] != EMPTY_BLOCK) {
			return true;
		}
	}
	return false;
}

#define TICK_INTERVAL_MS 1000

static void puzzle_attack_update(void)
{
	int now = rtc_get_ms_since_boot();

	if (now - last_tick_time >= TICK_INTERVAL_MS) {
		tick++;
		last_tick_time += TICK_INTERVAL_MS;
		if (tick % 100 == 0) {
			shift_grid_up();
			insert_row();
		}
	}

	if (swap_requested) {
		enum BLOCK_TYPE temp = grid[cursor_y][cursor_x];
		grid[cursor_y][cursor_x] = grid[cursor_y][cursor_x + 1];
		grid[cursor_y][cursor_x + 1] = temp;
		swap_requested = false;
		collapse_grid();
	}

	bool hasMatches;
	do {
		hasMatches = check_matches();
		if (hasMatches) {
			collapse_grid();
			score++;
		}
	} while (hasMatches);
	if (is_top_row_populated()) {
		puzzle_attack_state = PUZZLE_ATTACK_LOSE_SCREEN;
	}
}
static void puzzle_attack_init(void)
{
	puzzle_attack_state = PUZZLE_ATTACK_MENU;
	FbInit();
	FbClear();
	selected_outline_color = palette_color_from_index(default_palette, 7);
	init_grid();
	score = 0;
}
static void draw_bitmap(
	int x, int y, const uint8_t *bitmap, int width, int height)
{
	for (int row = 0; row < height; row++) {
		uint8_t bits = bitmap[row];
		for (int col = 0; col < width; col++) {
			if (bits & (1 << (width - 1 - col))) {
				FbPoint(x + col, y + row);
			}
		}
	}
}

static void draw_block(enum BLOCK_TYPE t, int x, int y, int size)
{
	if (t == EMPTY_BLOCK) {
		return;
	}

	struct ui_button block = {
		.x = x,
		.y = y,
		.width = size + 3,
		.height = size + 3,
		.outline_size = 1,
		.outline_color = palette_color_from_index(default_palette, 2),
		.fill_color = palette_color_from_index(default_palette, 2),
	};

	ui_button_dither_fill(block, block.fill_color, 0, 1);
	ui_button_draw_outline(block, block.outline_color);

	FbColor(palette_color_from_index(default_palette, t + 8));

	int bitmap_offset_x = (block.width - 7) / 2;
	int bitmap_offset_y = (block.height - 7) / 2;

	switch (t) {
	case CIRCLE_BLOCK:
		draw_bitmap(x + bitmap_offset_x, y + bitmap_offset_y,
			circle_bitmap, 7, 7);
		break;
	case SQUARE_BLOCK:
		draw_bitmap(x + bitmap_offset_x, y + bitmap_offset_y,
			square_bitmap, 7, 7);
		break;
	case TRIANGLE_BLOCK:
		draw_bitmap(x + bitmap_offset_x, y + 1 + bitmap_offset_y,
			triangle_bitmap, 7, 7);
		break;
	case HEART_BLOCK:
		draw_bitmap(x + bitmap_offset_x,
			y + 1 + ((block.height - 6) / 2), heart_bitmap, 7, 6);
		break;
	case STAR_BLOCK:
		draw_bitmap(x + bitmap_offset_x, y + bitmap_offset_y,
			star_bitmap, 7, 7);
		break;
	case EMPTY_BLOCK:
		break;
	}
}

static void draw_cursor(int spacing)
{
	int start_y = 1;
	int start_x = LCD_XSIZE / 2 - GRID_COLS * BLOCK_SIZE + spacing;

	FbColor(PACKRGB888(255, 255, 255));
	int cx = start_x + cursor_x * spacing;
	int cy = start_y + cursor_y * spacing;
	FbMove(cx, cy);
	FbRoundedRect(((BLOCK_SIZE + 3) * 2) + 1, BLOCK_SIZE + 3, 1);
}

static void draw_play_area(void)
{
	int spacing = BLOCK_SIZE + 4;
	int start_y = 1;
	int start_x = LCD_XSIZE / 2 - GRID_COLS * BLOCK_SIZE + spacing;

	int area_width = GRID_COLS * spacing - (spacing - (BLOCK_SIZE + 3));
	int area_height = GRID_ROWS * spacing - (spacing - (BLOCK_SIZE + 3));

	struct ui_button area = {
		.x = start_x - 2,
		.y = start_y - 2,
		.width = area_width + 4,
		.height = area_height + 4,
		.outline_size = 1,
		.outline_color = palette_color_from_index(default_palette, 6),
		.fill_color = palette_color_from_index(default_palette, 1),
	};

	/*ui_button_dither_fill(area, area.fill_color, 0, 1);*/
	ui_button_fill(area, area.fill_color);
	ui_button_draw_outline(area, area.outline_color);
}

static void draw_score(void)
{
	char msg[10];
	snprintf(msg, sizeof(msg), "%3d\n", score);
	FbMove(10, 10);
	FbColor(palette_color_from_index(default_palette, 12));
	FbWriteString(msg);
}

static void draw_game_over_screen(char *msg)
{
	FbColor(palette_color_from_index(default_palette, 13));
	char *press_b = "press b";
	char *to_go_back = "to go back";
	FbMove(ui_center_text_x(msg, 0, LCD_XSIZE),
		ui_center_text_y(0, LCD_YSIZE));
	FbWriteString(msg);
	FbMove(ui_center_text_x(press_b, 0, LCD_XSIZE),
		ui_center_text_y(16, LCD_YSIZE));
	FbWriteString(press_b);
	FbMove(ui_center_text_x(to_go_back, 0, LCD_XSIZE),
		ui_center_text_y(24, LCD_YSIZE));
	FbWriteString(to_go_back);
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
			.outline_size = 3,
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
			if (current_menu_item_selected)
				button.fill_color = palette_color_from_index(
					default_palette, 5);
		}

		ui_button_dither_fill(button, button.fill_color,
			palette_color_from_index(default_palette, 0), 1);
		ui_button_draw_outline(button, button.outline_color);
		ui_button_draw_label(button, button.text_color);
	}
}

static void draw_help_screen(void)
{
	FbClear();

	const char *lines[] = {"use the dpad", "to move cursor", "", "press a",
		"to swap blocks", "", "create matches", "to get points", "", "",
		"A btn for menu", "B btn for back"};

	int y = 8;
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

static void draw_grid(void)
{
	int spacing = BLOCK_SIZE + 4;
	int start_y = 1;
	int start_x = LCD_XSIZE / 2 - GRID_COLS * BLOCK_SIZE + spacing;

	draw_play_area();
	for (int y = 0; y < GRID_ROWS; y++) {
		for (int x = 0; x < GRID_COLS; x++) {
			if (grid[y][x] != EMPTY_BLOCK) {
				draw_block(grid[y][x], start_x + x * spacing,
					start_y + y * spacing, BLOCK_SIZE);
			}
		}
	}
	draw_cursor(spacing);
	draw_score();
}

void puzzle_attack_cb(__attribute__((unused)) struct menu_t *m)
{
	switch (puzzle_attack_state) {
	case PUZZLE_ATTACK_INIT:
		puzzle_attack_init();
		break;
	case PUZZLE_ATTACK_RUN:
		puzzle_attack_update();
		FbClear();
		draw_grid();
		FbSwapBuffers();
		break;
	case PUZZLE_ATTACK_SHOW_HELP:
		FbClear();
		draw_help_screen();
		FbSwapBuffers();
		break;
	case PUZZLE_ATTACK_MENU:
		FbClear();
		draw_menu();
		FbSwapBuffers();
		break;
	case PUZZLE_ATTACK_WIN_SCREEN:
		FbClear();
		draw_game_over_screen("YOU WON!");
		FbSwapBuffers();
		break;
	case PUZZLE_ATTACK_LOSE_SCREEN:
		FbClear();
		draw_game_over_screen("YOU LOST!");
		FbSwapBuffers();
		break;
	case PUZZLE_ATTACK_EXIT:
		puzzle_attack_state = PUZZLE_ATTACK_INIT;
		pop_app();
		break;
	default:
		break;
	}
	check_buttons();
}
