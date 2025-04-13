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

#include "button.h"
#include "colors.h"
#include "framebuffer.h"
#include "menu.h"
#include "palette.h"
#include "random.h"
#include "rtc.h"
#include "ui.h"
#include "xorshift.h"

static void check_matches_and_collapse(void);
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
	0b0111110,
	0b0111110,
	0b0111110,
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
#define BLOCK_SPACING 4
#define CURSOR_OUTLINE_SIZE 1
#define CURSOR_COLOR WHITE
#define AREA_OUTLINE_SIZE 1
#define AREA_OUTLINE_COLOR_INDEX 6
#define AREA_FILL_COLOR_INDEX 1
#define SCORE_COLOR_INDEX 12
#define GAME_OVER_COLOR_INDEX 13
#define MENU_OUTLINE_SIZE 3
#define MENU_OUTLINE_COLOR_INDEX 12
#define MENU_FILL_COLOR_INDEX 2
#define MENU_TEXT_COLOR_INDEX 7
#define MENU_SELECTED_OUTLINE_COLOR_INDEX 6
#define MENU_SELECTED_FILL_COLOR_INDEX 5

struct block {
	enum BLOCK_TYPE type;
	bool remove_animation_active;
	int remove_animation_progress;
};
static struct block grid[GRID_ROWS][GRID_COLS];

static int cursor_x = 0;
static int cursor_y = 0;
static bool swap_requested = false;
static int score = 0;
static int tick = 0;
static uint64_t last_tick_time = 0;
static unsigned int xorshift_state = 0;
static int hang_time = 80;

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

static void insert_row(void);
static void init_grid(void)
{
	for (int y = 0; y < GRID_ROWS; y++) {
		for (int x = 0; x < GRID_COLS; x++) {
			grid[y][x].type = EMPTY_BLOCK;
			grid[y][x].remove_animation_active = false;
			grid[y][x].remove_animation_progress = 0;
		}
	}
	insert_row();
}

static void shift_grid_up(void)
{
	for (int y = 1; y < GRID_ROWS; y++) {
		for (int x = 0; x < GRID_COLS; x++) {
			grid[y - 1][x] = grid[y][x];
		}
	}
}

static void shift_cursor_up(void)
{
	if (cursor_y <= 0)
		return;

	cursor_y -= 1;
}

static void insert_row(void)
{
	for (int x = 0; x < GRID_COLS; x++) {
		grid[GRID_ROWS - 1][x].type = xorshift(&xorshift_state) % 5;
		grid[GRID_ROWS - 1][x].remove_animation_active = false;
		grid[GRID_ROWS - 1][x].remove_animation_progress = 0;
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
}

static void handle_menu_options(void)
{
	switch (current_menu_item) {
	case 0:
		puzzle_attack_state = PUZZLE_ATTACK_RUN;
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

static bool is_part_of_match(int x, int y);
static void register_block_for_removal(int x, int y)
{
	if (x >= 0 && x < GRID_COLS && y >= 0 && y < GRID_ROWS) {
		grid[y][x].remove_animation_active = true;
		grid[y][x].remove_animation_progress = 1;
	}
}

static void register_blocks_for_removal(void)
{
	for (int y = 0; y < GRID_ROWS; y++) {
		for (int x = 0; x < GRID_COLS; x++) {
			if (is_part_of_match(x, y)) {
				register_block_for_removal(x, y);
			}
		}
	}
}

static bool check_matches(void)
{
	bool matched = false;
	int matchCount = 0;

	for (int y = 0; y < GRID_ROWS; y++) {
		for (int x = 0; x < GRID_COLS; x++) {
			if (is_part_of_match(x, y)) {
				matched = true;
				matchCount++;
			}
		}
	}

	if (matched) {
		score += matchCount;
	}

	return matched;
}

static bool collapse_grid(void)
{
	bool collapsed = false;

	for (int x = 0; x < GRID_COLS; x++) {
		for (int y = GRID_ROWS - 2; y >= 0; y--) {
			if (grid[y][x].type != EMPTY_BLOCK) {
				int drop = 0;
				/*how many empty spaces are below until */
				/*we hit a non-empty block or the bottom. */
				while (y + drop + 1 < GRID_ROWS &&
					grid[y + drop + 1][x].type ==
						EMPTY_BLOCK) {
					drop++;
				}
				if (drop > 0) {
					grid[y + drop][x] = grid[y][x];
					grid[y][x].type = EMPTY_BLOCK;
					collapsed = true;
				}
			}
		}
	}

	return collapsed;
}

static void check_matches_and_collapse(void)
{
	collapse_grid();
	check_matches();
	register_blocks_for_removal();
}

#if 0
static bool is_top_row_populated(void)
{
	for (int x = 0; x < GRID_COLS; x++) {
		if (grid[0][x].type != EMPTY_BLOCK) {
			return true;
		}
	}
	return false;
}
#endif

static bool is_part_of_match(int x, int y)
{
	if (grid[y][x].type == EMPTY_BLOCK) {
		return false;
	}

	enum BLOCK_TYPE current_block = grid[y][x].type;

	/* check horizontal match */
	if (x > 0 && x < GRID_COLS - 1) {
		if (grid[y][x - 1].type == current_block &&
			grid[y][x + 1].type == current_block) {
			return true;
		}
	}
	if (x > 1) {
		if (grid[y][x - 1].type == current_block &&
			grid[y][x - 2].type == current_block) {
			return true;
		}
	}
	if (x < GRID_COLS - 2) {
		if (grid[y][x + 1].type == current_block &&
			grid[y][x + 2].type == current_block) {
			return true;
		}
	}

	/*check vertical match*/
	if (y > 0 && y < GRID_ROWS - 1) {
		if (grid[y - 1][x].type == current_block &&
			grid[y + 1][x].type == current_block) {
			return true;
		}
	}
	if (y > 1) {
		if (grid[y - 1][x].type == current_block &&
			grid[y - 2][x].type == current_block) {
			return true;
		}
	}
	if (y < GRID_ROWS - 2) {
		if (grid[y + 1][x].type == current_block &&
			grid[y + 2][x].type == current_block) {
			return true;
		}
	}

	return false;
}

#define TICK_INTERVAL_MS 1000

static void update_remove_animations(void);
static void puzzle_attack_update(void)
{
	int now = rtc_get_ms_since_boot();

	if (now - last_tick_time >= TICK_INTERVAL_MS) {
		tick++;
		last_tick_time += TICK_INTERVAL_MS;
		if (tick % 100 == 0) {
			shift_grid_up();
			insert_row();
			shift_cursor_up();
		}
	}

	if (swap_requested) {
		enum BLOCK_TYPE temp = grid[cursor_y][cursor_x].type;
		grid[cursor_y][cursor_x].type =
			grid[cursor_y][cursor_x + 1].type;
		grid[cursor_y][cursor_x + 1].type = temp;
		swap_requested = false;
	}

	if (tick % hang_time == 0) {
		check_matches_and_collapse();
	}

	update_remove_animations();

#if 0
    if (is_top_row_populated()) {
        puzzle_attack_state = PUZZLE_ATTACK_LOSE_SCREEN;
    }
#endif
}

static void puzzle_attack_init(void)
{
	if (xorshift_state == 0) {
		random_insecure_bytes(
			(uint8_t *)&xorshift_state, sizeof(xorshift_state));
	}
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

static void update_remove_animations(void)
{
	for (int y = 0; y < GRID_ROWS; y++) {
		for (int x = 0; x < GRID_COLS; x++) {
			if (grid[y][x].remove_animation_active) {
				grid[y][x].remove_animation_progress++;
				if (grid[y][x].remove_animation_progress > 5) {
					grid[y][x].type = EMPTY_BLOCK;
					grid[y][x].remove_animation_active =
						false;
					grid[y][x].remove_animation_progress =
						0;
				}
			}
		}
	}
}

static void block_update_remove_animation_state(
	struct ui_button *block, struct block blk)
{
	if (blk.remove_animation_active) {
		block->fill_color = palette_color_from_index(
			default_palette, 2 + blk.remove_animation_progress);
	}
}

static void draw_block(
	struct block blk, int x, int y, int size, bool is_floating)
{
	if (blk.type == EMPTY_BLOCK) {
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

	int offset_x =
		LCD_XSIZE / 2 - GRID_COLS * BLOCK_SIZE + (BLOCK_SIZE + 4);
	int grid_x = (x - offset_x) / (BLOCK_SIZE + 4);
	int grid_y = (y - 1) / (BLOCK_SIZE + 4);

	if (is_part_of_match(grid_x, grid_y)) {
		block.outline_color =
			palette_color_from_index(default_palette, 13);
	}
	if (is_floating) {
		block.outline_color =
			palette_color_from_index(default_palette, 9);
	}

	block_update_remove_animation_state(&block, blk);

	ui_button_dither_fill(block, block.fill_color, 0, 1);
	ui_button_draw_outline(block, block.outline_color);
	FbColor(palette_color_from_index(default_palette, blk.type + 8));

	int bitmap_offset_x = (block.width - 7) / 2;
	int bitmap_offset_y = (block.height - 7) / 2;

	switch (blk.type) {
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
	int start_y = 3;
	int start_x =
		LCD_XSIZE / 2 - GRID_COLS * (BLOCK_SIZE + BLOCK_SPACING) / 2;

	FbColor(CURSOR_COLOR);
	int cx = start_x + cursor_x * spacing;
	int cy = start_y + cursor_y * spacing;
	FbMove(cx, cy);
	FbRoundedRect(((BLOCK_SIZE + 3) * 2) + 1, BLOCK_SIZE + 3,
		CURSOR_OUTLINE_SIZE);
}

static void draw_play_area(void)
{
	int spacing = BLOCK_SIZE + BLOCK_SPACING;
	int start_y = 4;
	int start_x =
		LCD_XSIZE / 2 - GRID_COLS * (BLOCK_SIZE + BLOCK_SPACING) / 2;

	int area_width = GRID_COLS * spacing - (spacing - (BLOCK_SIZE + 3));
	int area_height = GRID_ROWS * spacing - (spacing - (BLOCK_SIZE + 3));

	struct ui_button area = {
		.x = start_x - 2,
		.y = start_y - 2,
		.width = area_width + 3,
		.height = area_height + 3,
		.outline_size = AREA_OUTLINE_SIZE,
		.outline_color = palette_color_from_index(
			default_palette, AREA_OUTLINE_COLOR_INDEX),
		.fill_color = palette_color_from_index(
			default_palette, AREA_FILL_COLOR_INDEX),
	};

	ui_button_fill(area, area.fill_color);
	ui_button_draw_outline(area, area.outline_color);
}

static void draw_score(void)
{
	char msg[10];
	snprintf(msg, sizeof(msg), "%3d\n", score);
	FbMove(10, 10);
	FbColor(palette_color_from_index(default_palette, SCORE_COLOR_INDEX));
	FbWriteString(msg);
}

static void draw_game_over_screen(char *msg)
{
	FbColor(palette_color_from_index(
		default_palette, GAME_OVER_COLOR_INDEX));
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
			.outline_size = MENU_OUTLINE_SIZE,
			.outline_color = palette_color_from_index(
				default_palette, MENU_OUTLINE_COLOR_INDEX),
			.fill_color = palette_color_from_index(
				default_palette, MENU_FILL_COLOR_INDEX),
			.text_color = palette_color_from_index(
				default_palette, MENU_TEXT_COLOR_INDEX),
		};

		if (i == current_menu_item) {
			button.outline_color =
				palette_color_from_index(default_palette,
					MENU_SELECTED_OUTLINE_COLOR_INDEX);
			if (current_menu_item_selected)
				button.fill_color = palette_color_from_index(
					default_palette,
					MENU_SELECTED_FILL_COLOR_INDEX);
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
	int padding_y = 1;
	int spacing = BLOCK_SIZE + BLOCK_SPACING;
	int start_y = padding_y + 2;
	int start_x =
		LCD_XSIZE / 2 - GRID_COLS * (BLOCK_SIZE + BLOCK_SPACING) / 2;

	draw_play_area();
	for (int y = 0; y < GRID_ROWS; y++) {
		for (int x = 0; x < GRID_COLS; x++) {
			if (grid[y][x].type != EMPTY_BLOCK) {
				bool is_floating = false;
				if (y < GRID_ROWS - 1 &&
					grid[y + 1][x].type == EMPTY_BLOCK) {
					is_floating = true;
				}
				draw_block(grid[y][x], start_x + x * spacing,
					start_y + y * spacing, BLOCK_SIZE,
					is_floating);
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
