/**
 * A basic match 3 style game similar to tetris attack.
 *
 * i've decided to only have endless play.
 * no win or lose screen or game timer
 * i think the board is too small and it would be
 * too difficultif you lose when the top row is populated.
 *
 *
 *
 * data optimization notes:
 *
 * original implementation was based around blocks being
 *struct block {
 *	enum BLOCK_TYPE type;
 *	bool remove_animation_active;
 *	int remove_animation_progress;
 *};
 * Size: 12 bytes, alignment 4 bytes
 *
 * #define GRID_COLS 6
 * #define GRID_ROWS 10
 *
 * 720 bytes for the entire board?
 *
 * each cell can be in one of 6 states
 *	EMPTY_BLOCK = -1,
 *	CIRCLE_BLOCK,
 *	SQUARE_BLOCK,
 *	TRIANGLE_BLOCK,
 *	HEART_BLOCK,
 *	STAR_BLOCK,
 *
 * we can represent that as 3 bits
 * e.g.
 * enum BLOCK_TYPE {
 *	CIRCLE_BLOCK = 0,   // 000
 *	SQUARE_BLOCK = 1,   // 001
 *	TRIANGLE_BLOCK = 2, // 010
 *	HEART_BLOCK = 3,    // 011
 *	STAR_BLOCK = 4,	    // 100
 *  // some unused
 *	EMPTY_BLOCK = 7	    // 111
 *};
 *
 * i guess if we got rid of 2 block types it would fit nicely into 2 bits.
 * ... but that might make the gameplay more boring.
 * maybe we could add 2 block types
 *
 *
 * since we have need 3 bits to represent the type and we know the grid size
 * is 6*10
 * 3*6*10 = 180
 * we can represent the grid as 180 bits.
 * 3*64=192 -- so we have some extra space
 *
 * we can track any removal/is_hovering/animation state separately.
 *
 * - type bits (pack into uint64_t types[3];  3×64=192 bits)
 * - remove mask (uint64_t)
 * - animation_progress bits (uint64_t progress[3];)
 *
 *   ~56 bytes for the entire board
 *
 * --
 *  Dustin Firebaugh
 */

#include <stdbool.h>
#include <stdint.h>
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
#include "particle.h"

#define IS_ENDLESS_PLAY_DISABLED 0

static int has_screen_changed = 0;
static int has_grid_changed = 1;

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
	CIRCLE_BLOCK = 0,   /* 000 */
	SQUARE_BLOCK = 1,   /* 001 */
	TRIANGLE_BLOCK = 2, /* 010 */
	HEART_BLOCK = 3,    /* 011 */
	STAR_BLOCK = 4,	    /* 100 */
	EMPTY_BLOCK = 7	    /* 111 */
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
#define CELL_COUNT (GRID_COLS * GRID_ROWS)

static uint64_t grid[3];
static uint64_t blocks_to_be_removed;
static uint64_t removal_animation_state[3];

static struct particle_pool *particle_pool = NULL;
#define PARTICLE_GRAVITY 16
#define PARTICLE_MAX_INITIAL_VELOCITY 800

/*
 * since grid and removal_animation_state are both 3*64
 * we can use the same functions to manipulate them
 */
static inline void bit_set(uint64_t *arr, int bit, uint64_t v)
{
	int idx = bit >> 6;
	int ofs = bit & 63;
	uint64_t mask = ((uint64_t)1) << ofs;

	arr[idx] = (arr[idx] & ~mask) | ((v & 1) << ofs);
}
static inline uint64_t bit_get(const uint64_t *arr, int bit)
{
	int idx = bit >> 6;
	int ofs = bit & 63;
	uint64_t mask = ((uint64_t)1) << ofs;

	return (arr[idx] & mask) ? 1 : 0;
}
static inline void field_set(uint64_t *arr, int base, uint8_t v)
{
	for (int i = 0; i < 3; i++)
		bit_set(arr, base + i, (v >> i) & 1);
}
static inline uint8_t field_get(const uint64_t *arr, int base)
{
	uint8_t v = 0;
	for (int i = 0; i < 3; i++)
		v |= bit_get(arr, base + i) << i;
	return v;
}
static inline void set_cell(
	int x, int y, enum BLOCK_TYPE block_type, bool r, uint8_t p)
{
	int idx = y * GRID_COLS + x;
	field_set(grid, idx * 3, (uint8_t)block_type);
	bit_set(&blocks_to_be_removed, idx, r);
	field_set(removal_animation_state, idx * 3, p);
	has_grid_changed = 1;
}
static inline enum BLOCK_TYPE block_get_type(int x, int y)
{
	return (enum BLOCK_TYPE)field_get(grid, (y * GRID_COLS + x) * 3);
}
static inline bool get_remove_state(int x, int y)
{
	return bit_get(&blocks_to_be_removed, y * GRID_COLS + x);
}
static inline uint8_t get_removal_progress(int x, int y)
{
	return field_get(removal_animation_state, (y * GRID_COLS + x) * 3);
}

#define BLOCK_SIZE 8
#define BLOCK_SPACING 4
#define REMOVE_BLOCK_ANIMATION_END 7
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

static int cursor_x = 0;
static int cursor_y = 0;
static bool swap_requested = false;
static int score = 0;
static int tick = 0;
static uint64_t last_tick_time = 0;
static unsigned int xorshift_state = 0;
static int hang_time = 4;

#define NUM_MENU_ITEMS 4
#define MENU_ITEM_SPACING 30
#define MENU_ITEM_WIDTH 100
#define MENU_ITEM_HEIGHT 20
#define MENU_X (LCD_XSIZE / 2 - MENU_ITEM_WIDTH / 2)
#define MENU_Y (LCD_YSIZE / 2 - MENU_ITEM_HEIGHT / 2)
static int current_menu_item = 0;
static bool current_menu_item_selected = false;
static const char *menu_items[NUM_MENU_ITEMS] = {
	"play", "reset", "how to play", "exit"};

static void insert_row(void)
{
	for (int x = 0; x < GRID_COLS; x++)
		set_cell(x, GRID_ROWS - 1, xorshift(&xorshift_state) % 5, false,
			0);
}
static void init_grid(void)
{
	for (int i = 0; i < 3; i++) {
		grid[i] = 0;
		removal_animation_state[i] = 0;
	}
	blocks_to_be_removed = 0;
	for (int y = 0; y < GRID_ROWS; y++)
		for (int x = 0; x < GRID_COLS; x++)
			set_cell(x, y, EMPTY_BLOCK, false, 0);
	insert_row();
}

static void shift_grid_up(void)
{
	for (int y = 1; y < GRID_ROWS; y++)
		for (int x = 0; x < GRID_COLS; x++) {
			enum BLOCK_TYPE t = block_get_type(x, y);
			bool r = get_remove_state(x, y);
			uint8_t p = get_removal_progress(x, y);
			set_cell(x, y - 1, t, r, p);
		}
	for (int x = 0; x < GRID_COLS; x++)
		set_cell(x, GRID_ROWS - 1, EMPTY_BLOCK, false, 0);
	has_grid_changed = 1;
}
static void shift_cursor_up(void)
{
	if (cursor_y > 0)
		cursor_y--;
}
static void previous_menu_item(void)
{
	if (--current_menu_item < 0)
		current_menu_item = NUM_MENU_ITEMS - 1;
}
static void next_menu_item(void)
{
	if (++current_menu_item >= NUM_MENU_ITEMS)
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
	}
}

static void check_buttons(void)
{
	int down_latches = button_down_latches();
	if (puzzle_attack_state == PUZZLE_ATTACK_MENU) {
		current_menu_item_selected = false;
		if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches))
			previous_menu_item();
		else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches))
			next_menu_item();
		else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
			current_menu_item_selected = true;
			handle_menu_options();
		} else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches))
			puzzle_attack_state = PUZZLE_ATTACK_EXIT;
		return;
	}
	if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down_latches) && cursor_x > 0)
		cursor_x--;
	else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches) &&
		cursor_x < GRID_COLS - 2)
		cursor_x++;
	else if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches) && cursor_y > 0)
		cursor_y--;
	else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches) &&
		cursor_y < GRID_ROWS - 1)
		cursor_y++;
	else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches))
		swap_requested = true;
	else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches))
		puzzle_attack_state = PUZZLE_ATTACK_MENU;
}

static bool is_part_of_match(int x, int y)
{
	enum BLOCK_TYPE block_type = block_get_type(x, y);

	if (block_type == EMPTY_BLOCK)
		return false;

	if (x > 0 && x < GRID_COLS - 1 &&
		block_get_type(x - 1, y) == block_type &&
		block_get_type(x + 1, y) == block_type)
		return true;

	if (x > 1 && block_get_type(x - 1, y) == block_type &&
		block_get_type(x - 2, y) == block_type)
		return true;

	if (x < GRID_COLS - 2 && block_get_type(x + 1, y) == block_type &&
		block_get_type(x + 2, y) == block_type)
		return true;

	if (y > 0 && y < GRID_ROWS - 1 &&
		block_get_type(x, y - 1) == block_type &&
		block_get_type(x, y + 1) == block_type)
		return true;

	if (y > 1 && block_get_type(x, y - 1) == block_type &&
		block_get_type(x, y - 2) == block_type)
		return true;

	if (y < GRID_ROWS - 2 && block_get_type(x, y + 1) == block_type &&
		block_get_type(x, y + 2) == block_type)
		return true;

	return false;
}

static void register_blocks_for_removal(void)
{
	for (int y = 0; y < GRID_ROWS; y++)
		for (int x = 0; x < GRID_COLS; x++)
			if (is_part_of_match(x, y))
				set_cell(x, y, block_get_type(x, y), true, 1);
}

enum {
	MATCH_LEVEL_NONE = 0,
	MATCH_LEVEL_PARTICLES = 1,
};

static bool check_matches(void)
{
	int match_count = 0;
	int match_x[GRID_ROWS * GRID_COLS];
	int match_y[GRID_ROWS * GRID_COLS];

	/* record matches */
	for (int y = 0; y < GRID_ROWS; y++) {
		for (int x = 0; x < GRID_COLS; x++) {
			if (is_part_of_match(x, y)) {
				match_x[match_count] = x;
				match_y[match_count] = y;
				match_count++;
			}
		}
	}

	if (match_count > MATCH_LEVEL_NONE)
		score += match_count;

	if (match_count > MATCH_LEVEL_PARTICLES) {
		int spacing = BLOCK_SIZE + BLOCK_SPACING;
		int origin_x = (LCD_XSIZE / 2) - (GRID_COLS * spacing / 2);
		int origin_y = 1 * 1 + 2;
		int center_offset = (BLOCK_SIZE + 3) / 2;

		for (int m = 0; m < match_count; m++) {
			int cx = origin_x + match_x[m] * spacing + center_offset;
			int cy = origin_y + match_y[m] * spacing + center_offset;

			for (int i = 0; i < 20; i++) {
				int vx = (xorshift(&xorshift_state) % (2*PARTICLE_MAX_INITIAL_VELOCITY + 1))
					- PARTICLE_MAX_INITIAL_VELOCITY;
				int vy = - (xorshift(&xorshift_state) % PARTICLE_MAX_INITIAL_VELOCITY);
				int idx = xorshift(&xorshift_state) % PALETTE_SIZE;
				int color = palette_color_from_index(default_palette, idx);

				particle_pool->config.add_particle(
					particle_pool,
					cx << 8,
					cy << 8,
					vx,
					vy,
					64 + ((xorshift(&xorshift_state) >> 16) & 0x7),
					color);
			}
		}
	}

	return (match_count > 0);
}

static bool collapse_grid(void)
{
	bool col = false;
	for (int x = 0; x < GRID_COLS; x++)
		for (int y = GRID_ROWS - 2; y >= 0; y--) {
			enum BLOCK_TYPE t = block_get_type(x, y);
			/*how many empty spaces are below until */
			/*we hit a non-empty block or the bottom. */
			if (t != EMPTY_BLOCK) {
				int d = 0;
				while (y + d + 1 < GRID_ROWS &&
					block_get_type(x, y + d + 1) ==
						EMPTY_BLOCK)
					d++;
				if (d > 0) {
					bool r = get_remove_state(x, y);
					uint8_t p = get_removal_progress(x, y);
					set_cell(x, y + d, t, r, p);
					set_cell(x, y, EMPTY_BLOCK, false, 0);
					col = true;
				}
			}
		}
	return col;
}

/*
 * when a block is marked for removal,
 * we store a value (removal_progress) that increments everytime we
 * call update_remove_animations. when that value reaches
 * REMOVE_BLOCK_ANIMATION_END, we remove the block (i.e. we set it to EMPTY).
 *
 * the basic animation should just iterate through an index
 * on the palette
 */
static void update_remove_animations(void)
{
	for (int y = 0; y < GRID_ROWS; y++)
		for (int x = 0; x < GRID_COLS; x++) {
			if (block_get_type(x, y) == EMPTY_BLOCK)
				continue;
			if (get_remove_state(x, y)) {
				uint8_t p = get_removal_progress(x, y) + 1;
				if (p > REMOVE_BLOCK_ANIMATION_END)
					set_cell(x, y, EMPTY_BLOCK, false, 0);
				else
					set_cell(x, y, block_get_type(x, y),
						true, p);
			}
		}
}

static void check_matches_and_collapse(void)
{
	collapse_grid();
	check_matches();
	register_blocks_for_removal();
}

/*
 * swaps should be blocked if an removal animation is
 * playing.
 */
static bool is_available_for_swap(int x, int y)
{
	return get_removal_progress(x, y) == 0 &&
		get_removal_progress(x + 1, y) == 0;
}

static void swap_blocks_at_cursor(void)
{
	int y = cursor_y, x = cursor_x;
	swap_requested = false;

	if (!is_available_for_swap(x, y))
		return;

	enum BLOCK_TYPE a = block_get_type(x, y);
	enum BLOCK_TYPE b = block_get_type(x + 1, y);

	set_cell(x, y, b, false, 0);
	set_cell(x + 1, y, a, false, 0);

	has_grid_changed = 1;
}

static void puzzle_attack_update(void)
{
	uint64_t now = rtc_get_ms_since_boot();
	int count = (now / 1000) % 60;
	static int last_count = -1;
	if (count != last_count) {
		last_count = count;
		if (--tick <= 0) {
			check_matches_and_collapse();
			tick = hang_time;
		}
		if (count % 5 == 0) {
			shift_grid_up();
			insert_row();
			shift_cursor_up();
		}
	}

	particle_pool->config.move_particles(particle_pool);


	if (swap_requested)
		swap_blocks_at_cursor();

	/*
	 * trying to update the animation state in
	 * a more controled way.
	 * in the sim, it seems to run too fast if we
	 * call it every frame
	 */
	int removal_count = (now / 60) % 60;
	static int removal_last_count = -1;
	if (removal_count != removal_last_count) {
		removal_last_count = removal_count;
		if (removal_count % 2 == 0) {
			update_remove_animations();
		}
	}
#if IS_ENDLESS_PLAY_DISABLED
	if (is_top_row_populated())
		puzzle_attack_state = PUZZLE_ATTACK_LOSE_SCREEN;
#endif
}

static void puzzle_attack_init(void)
{
	if (xorshift_state == 0)
		random_insecure_bytes(
			(uint8_t *)&xorshift_state, sizeof(xorshift_state));
	puzzle_attack_state = PUZZLE_ATTACK_MENU;
	FbInit();
	FbClear();
	selected_outline_color = palette_color_from_index(default_palette, 7);
	init_grid();
	last_tick_time = rtc_get_ms_since_boot();
	tick = hang_time;
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
	has_screen_changed = 1;
}

static void draw_block(int grid_y, int grid_x, int start_x, int start_y, int sz,
	bool is_floating)
{
	enum BLOCK_TYPE block_type = block_get_type(grid_x, grid_y);

	if (block_type == EMPTY_BLOCK)
		return;

	struct ui_button block = {
		.x = start_x,
		.y = start_y,
		.width = sz + 3,
		.height = sz + 3,
		.outline_size = 1,
		.outline_color = palette_color_from_index(default_palette, 2),
		.fill_color = palette_color_from_index(default_palette, 2),
	};

	if (is_part_of_match(grid_x, grid_y))
		block.outline_color =
			palette_color_from_index(default_palette, 13);

	if (is_floating)
		block.outline_color =
			palette_color_from_index(default_palette, 9);

	if (get_remove_state(grid_x, grid_y))
		block.fill_color = palette_color_from_index(default_palette,
			2 + get_removal_progress(grid_x, grid_y));

	ui_button_dither_fill(block, block.fill_color, 0, 1);
	ui_button_draw_outline(block, block.outline_color);
	FbColor(palette_color_from_index(default_palette, block_type + 8));

	int block_offset = (block.width - 7) / 2, bt = (block.height - 7) / 2;

	switch (block_type) {
	case CIRCLE_BLOCK:
		draw_bitmap(start_x + block_offset, start_y + bt, circle_bitmap,
			7, 7);
		break;
	case SQUARE_BLOCK:
		draw_bitmap(start_x + block_offset, start_y + bt, square_bitmap,
			7, 7);
		break;
	case TRIANGLE_BLOCK:
		draw_bitmap(start_x + block_offset, start_y + 1 + bt,
			triangle_bitmap, 7, 7);
		break;
	case HEART_BLOCK:
		draw_bitmap(start_x + block_offset,
			start_y + 1 + ((block.height - 6) / 2), heart_bitmap, 7,
			6);
		break;
	case STAR_BLOCK:
		draw_bitmap(start_x + block_offset, start_y + bt, star_bitmap,
			7, 7);
		break;
	default:
		break;
	}
	has_screen_changed = 1;
}

static void draw_cursor(int sp)
{
	int start_x =
		(LCD_XSIZE / 2 - GRID_COLS * (BLOCK_SIZE + BLOCK_SPACING) / 2) +
		cursor_x * sp;
	int start_y = 3 + cursor_y * sp;
	FbColor(CURSOR_COLOR);
	FbMove(start_x, start_y);
	FbRoundedRect(((BLOCK_SIZE + 3) * 2) + 1, BLOCK_SIZE + 3,
		CURSOR_OUTLINE_SIZE);
	has_screen_changed = 1;
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
	char buf[8];
	snprintf(buf, sizeof(buf), "%3d", score);
	FbMove(10, 10);
	FbColor(palette_color_from_index(default_palette, SCORE_COLOR_INDEX));
	FbWriteString(buf);
	has_screen_changed = 1;
}

static void draw_tick(void)
{
	char buf[8];
	snprintf(buf, sizeof(buf), "%3d", tick);
	FbMove(10, 20);
	FbColor(palette_color_from_index(
		default_palette, SCORE_COLOR_INDEX + 1));
	FbWriteString(buf);
	has_screen_changed = 1;
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
	has_screen_changed = 1;
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
	has_screen_changed = 1;
}

static void draw_grid(void)
{
	int sp = BLOCK_SIZE + BLOCK_SPACING;
	int sx = (LCD_XSIZE / 2 - GRID_COLS * sp / 2), sy = 1 * 1 + 2;
	draw_play_area();
	for (int y = 0; y < GRID_ROWS; y++)
		for (int x = 0; x < GRID_COLS; x++) {
			enum BLOCK_TYPE t = block_get_type(x, y);
			if (t != EMPTY_BLOCK) {
				bool f = (y < GRID_ROWS - 1 &&
					block_get_type(x, y + 1) ==
						EMPTY_BLOCK);
				draw_block(y, x, sx + x * sp, sy + y * sp,
					BLOCK_SIZE, f);
			}
		}
	has_screen_changed = 1;
}

static void draw_screen(void)
{
	FbClear();
	if (has_grid_changed) {
		draw_grid();
		has_grid_changed = 0;
	}
	draw_cursor(BLOCK_SIZE + BLOCK_SPACING);
	draw_score();
	draw_tick();
	particle_pool->config.draw_particles(particle_pool);
}

void puzzle_attack_cb(__attribute__((unused)) struct menu_t *m)
{
#define PUZZLE_ATTACK_POOL_SIG 0xC111456
	if (particle_pool == NULL){
		particle_pool = get_common_particle_pool();
		particle_pool->nparticles = 0;
	}
	if (claim_particle_pool(particle_pool, PUZZLE_ATTACK_POOL_SIG )) {
		particle_pool->config.gravityy = (int)PARTICLE_GRAVITY;
	}
	switch (puzzle_attack_state) {
	case PUZZLE_ATTACK_INIT:
		puzzle_attack_init();
		break;
	case PUZZLE_ATTACK_RUN:
		puzzle_attack_update();
		draw_screen();
		break;
	case PUZZLE_ATTACK_SHOW_HELP:
		FbClear();
		draw_help_screen();
		break;
	case PUZZLE_ATTACK_MENU:
		FbClear();
		draw_menu();
		break;
	case PUZZLE_ATTACK_WIN_SCREEN:
		FbClear();
		draw_game_over_screen("YOU WON!");
		break;
	case PUZZLE_ATTACK_LOSE_SCREEN:
		FbClear();
		draw_game_over_screen("YOU LOST!");
		break;
	case PUZZLE_ATTACK_EXIT:
		puzzle_attack_state = PUZZLE_ATTACK_INIT;
		pop_app();
		break;
	default:
		break;
	}
	check_buttons();
	if (has_screen_changed) {
		has_grid_changed = 1;
		FbSwapBuffers();
		has_screen_changed = 0;
	}
}
