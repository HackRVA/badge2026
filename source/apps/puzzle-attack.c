/**
 * A basic match 3 style game similar to tetris attack.
 *
 * i've decided to only have endless play.
 * no win or lose screen or game timer
 * i think the board is too small and it would be
 * too difficultif you lose when the top row is populated.
 *
 * --
 *  Dustin Firebaugh
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
#include "audio.h"
#include "music.h"

#define IS_ENDLESS_PLAY_DISABLED 0
#define ENABLE_LIGHTNING 1
#define DEBUG_LIGHTNING 0

#define EVAL_CYCLE_MS 5000
#define GRID_SHIFT_MS 6000
#define COLLAPSE_GRID_MS 1000
static uint64_t collapse_cooldown;
static uint64_t cycle_cooldown;
static uint64_t grid_shift_cooldown;

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
	CIRCLE_BLOCK = 0,	/* 000 */
	SQUARE_BLOCK = 1,	/* 001 */
	TRIANGLE_BLOCK = 2,	/* 010 */
	HEART_BLOCK = 3,	/* 011 */
	STAR_BLOCK = 4,	/* 100 */
	EMPTY_BLOCK = 7	/* 111 */
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

static uint8_t block_type[CELL_COUNT];
static bool removal_state[CELL_COUNT];
static uint8_t removal_progress[CELL_COUNT];

static struct particle_pool *particle_pool = NULL;
#define PARTICLE_GRAVITY 16
#define PARTICLE_MAX_INITIAL_VELOCITY 800

#define ARRAYSIZE(x) (sizeof(x) / sizeof((x)[0]))

#define whole_note (2000)
#define half_note (whole_note / 2)
#define quarter_note (whole_note / 4)
#define dotted_quarter ((3 * whole_note) / 8)
#define eighth_note (whole_note / 8)
#define sixteenth_note (whole_note / 16)
#define thirtysecond_note (whole_note / 32)

#define kick_drum { NOTE_A2, 10, }
#define snare_drum { NOTE_B6, 10, }

static enum {
	AUDIO_THEME,
	AUDIO_SFX,
} audio_mode = AUDIO_THEME;

static size_t theme_index = 0;
static uint64_t theme_note_start = 0;

static struct note *current_sfx = NULL;
static size_t current_sfx_len = 0;
static size_t sfx_index = 0;
static uint64_t sfx_note_start = 0;

static struct note sfx_one[] = {
	{ NOTE_C5, thirtysecond_note },
	{ NOTE_Ef5, thirtysecond_note },
	{ NOTE_G5, thirtysecond_note },
};

static struct note sfx_two[] = {
	{NOTE_G5, thirtysecond_note},
	{NOTE_F5, thirtysecond_note},
	{NOTE_G5, thirtysecond_note},
	{NOTE_Bf5, thirtysecond_note},
	{NOTE_C6, thirtysecond_note},
	{NOTE_C6, thirtysecond_note},
	{NOTE_C6, thirtysecond_note},
	{NOTE_C6, thirtysecond_note},
};

static struct note puzzle_attack_theme_notes[] = {
	/* just bass */
  kick_drum,
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_F3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,

	{ NOTE_REST, quarter_note, },
  snare_drum,
	{ NOTE_REST, eighth_note, },
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_G3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_G3, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
  snare_drum,
	{ NOTE_REST, quarter_note, },
  kick_drum,
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_Bf3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_REST, quarter_note, },
  snare_drum,
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Bf2, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_Bf2, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
        { NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_REST, quarter_note, },
  kick_drum,
	{ NOTE_Bf2, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Bf2, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_REST, quarter_note, },
  snare_drum,
	{ NOTE_REST, quarter_note, },


	/* both */
  kick_drum,
	{ NOTE_C3, sixteenth_note, },
		{ NOTE_Ef5, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
		{ NOTE_Ef5, sixteenth_note, },
  snare_drum,
	{ NOTE_F3, sixteenth_note, },
		{ NOTE_C5, sixteenth_note, },
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
		{ NOTE_F4, thirtysecond_note, },
		{ NOTE_G4, thirtysecond_note, },
		{ NOTE_Bf4, thirtysecond_note, },
		{ NOTE_C5, thirtysecond_note, },
		{ NOTE_Ef5, thirtysecond_note, },
		{ NOTE_F5, eighth_note, },
  snare_drum,
		{ NOTE_G5, eighth_note, },
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_G3, sixteenth_note, },
		{ NOTE_C5, sixteenth_note, },
	{ NOTE_G3, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
  snare_drum,
		{ NOTE_C5, eighth_note, },
		{ NOTE_C6, sixteenth_note, },
		{ NOTE_Bf5, sixteenth_note, },
  kick_drum,
	{ NOTE_Ef3, sixteenth_note, },
		{ NOTE_G5, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_Bf2, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
		{ NOTE_G5, thirtysecond_note, },
		{ NOTE_Bf5, thirtysecond_note, },
		{ NOTE_REST, sixteenth_note, },
		{ NOTE_F5, eighth_note, },
  snare_drum,
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Bf2, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_Bf2, sixteenth_note, },
		{ NOTE_Ef5, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
		{ NOTE_G5, sixteenth_note, },
  snare_drum,
	{ NOTE_REST, quarter_note, },
  kick_drum,
	{ NOTE_Bf2, sixteenth_note, },
		{ NOTE_F5, sixteenth_note, },
	{ NOTE_C3, sixteenth_note, },
		{ NOTE_G5, sixteenth_note, },
  snare_drum,
	{ NOTE_Ef3, sixteenth_note, },
		{ NOTE_Bf5, sixteenth_note, },
	{ NOTE_Bf2, sixteenth_note, },
		{ NOTE_G5, sixteenth_note, },
  kick_drum,
		{ NOTE_G5, thirtysecond_note, },
		{ NOTE_Bf5, thirtysecond_note, },
		{ NOTE_F5, sixteenth_note, },
		{ NOTE_Ef5, sixteenth_note, },
		{ NOTE_C5, sixteenth_note, },
  snare_drum,
		{ NOTE_F5, sixteenth_note, },
		{ NOTE_Ef5, eighth_note, },
		{ NOTE_REST, sixteenth_note, },

	/* both */
  kick_drum,
	{ NOTE_C3, sixteenth_note, },
		{ NOTE_Ef5, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
		{ NOTE_Ef5, sixteenth_note, },
  snare_drum,
	{ NOTE_F3, sixteenth_note, },
		{ NOTE_C5, sixteenth_note, },
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
		{ NOTE_F4, thirtysecond_note, },
		{ NOTE_G4, thirtysecond_note, },
		{ NOTE_Bf4, thirtysecond_note, },
		{ NOTE_C5, thirtysecond_note, },
		{ NOTE_Ef5, thirtysecond_note, },
		{ NOTE_F5, eighth_note, },
  snare_drum,
		{ NOTE_G5, eighth_note, },
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_G3, sixteenth_note, },
		{ NOTE_C5, sixteenth_note, },
	{ NOTE_G3, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
  snare_drum,
		{ NOTE_C5, eighth_note, },
		{ NOTE_C6, sixteenth_note, },
		{ NOTE_Bf5, sixteenth_note, },
  kick_drum,
	{ NOTE_Ef3, sixteenth_note, },
		{ NOTE_G5, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_Bf2, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
		{ NOTE_G5, thirtysecond_note, },
		{ NOTE_Bf5, thirtysecond_note, },
		{ NOTE_REST, sixteenth_note, },
		{ NOTE_F5, eighth_note, },
  snare_drum,
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Bf2, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_Bf2, sixteenth_note, },
		{ NOTE_Ef5, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
		{ NOTE_G5, sixteenth_note, },
  snare_drum,
	{ NOTE_REST, quarter_note, },
  kick_drum,
	{ NOTE_Bf2, sixteenth_note, },
		{ NOTE_F5, sixteenth_note, },
	{ NOTE_C3, sixteenth_note, },
		{ NOTE_G5, sixteenth_note, },
  snare_drum,
	{ NOTE_Ef3, sixteenth_note, },
		{ NOTE_Bf5, sixteenth_note, },
	{ NOTE_Bf2, sixteenth_note, },
		{ NOTE_G5, sixteenth_note, },
  kick_drum,
		{ NOTE_G5, thirtysecond_note, },
		{ NOTE_Bf5, thirtysecond_note, },
		{ NOTE_F5, sixteenth_note, },
		{ NOTE_Ef5, sixteenth_note, },
		{ NOTE_C5, sixteenth_note, },
  snare_drum,
		{ NOTE_F5, sixteenth_note, },
		{ NOTE_Ef5, eighth_note, },
		{ NOTE_REST, sixteenth_note, },

	/* just bass */
  kick_drum,
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_F3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,

	{ NOTE_REST, quarter_note, },
  snare_drum,
	{ NOTE_REST, eighth_note, },
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_G3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_G3, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
  snare_drum,
	{ NOTE_REST, quarter_note, },
  kick_drum,
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_Bf3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_REST, quarter_note, },
  snare_drum,
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Bf2, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_Bf2, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
        { NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_REST, quarter_note, },
  kick_drum,
	{ NOTE_Bf2, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Bf2, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_REST, quarter_note, },
  snare_drum,
	{ NOTE_REST, quarter_note, },
};

static struct tune puzzle_attack_theme = {
	.num_notes = ARRAYSIZE(puzzle_attack_theme_notes),
	.note = &puzzle_attack_theme_notes[0],
};

static inline enum BLOCK_TYPE block_get_type(int x, int y) {
	int idx = y * GRID_COLS + x;
	return (enum BLOCK_TYPE)block_type[idx];
}

static inline bool is_marked_for_removal(int x, int y) {
	int idx = y * GRID_COLS + x;
	return removal_state[idx];
}

static inline uint8_t get_removal_progress(int x, int y) {
	int idx = y * GRID_COLS + x;
	return removal_progress[idx];
}

static inline void set_cell(int x, int y, enum BLOCK_TYPE t, bool r, uint8_t p) {
	int idx = y * GRID_COLS + x;
	block_type[idx] = t;
	removal_state[idx] = r;
	removal_progress[idx] = p;
	has_grid_changed = 1;
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
	for (int i = 0; i < CELL_COUNT; i++) {
		block_type[i] = EMPTY_BLOCK;
		removal_state[i] = false;
		removal_progress[i] = 0;
	}
	insert_row();
}

static void shift_grid_up(void)
{
	for (int y = 1; y < GRID_ROWS; y++)
		for (int x = 0; x < GRID_COLS; x++) {
			enum BLOCK_TYPE t = block_get_type(x, y);
			bool r = is_marked_for_removal(x, y);
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
	if (puzzle_attack_state == PUZZLE_ATTACK_SHOW_HELP) {
		if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down_latches))
			puzzle_attack_state = PUZZLE_ATTACK_MENU;
		else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches))
			puzzle_attack_state = PUZZLE_ATTACK_MENU;
		else if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches))
			puzzle_attack_state = PUZZLE_ATTACK_MENU;
		else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches))
			puzzle_attack_state = PUZZLE_ATTACK_MENU;
		else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches))
			puzzle_attack_state = PUZZLE_ATTACK_MENU;
		else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches))
			puzzle_attack_state = PUZZLE_ATTACK_MENU;
		return;
	}
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

#if ENABLE_LIGHTNING
/*
 * the lightning looks way cooler with more segments
 * but i guess we should be somewhat memory conscious
 */
#define MAX_LIGHTNING_SEGS 32
struct lightning_seg { int16_t x1, y1, x2, y2; };
static struct lightning_seg lightning[MAX_LIGHTNING_SEGS];
static int lightning_count = 0;
static bool lightning_active = false;
static int  lightning_frames_left = 0;

/*
 * subdivide_lightning
 * inspired by wordwarvi lightning
 *
 * This implementation is different in that it it stores segments
 * to draw later rather than drawing them directly
 */
static void subdivide_lightning(int x1, int y1, int x2, int y2)
{
	if (lightning_count >= MAX_LIGHTNING_SEGS)
		return;

	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);

	if (dx < 10 && dy < 10) {
		lightning[lightning_count++] = (struct lightning_seg){x1, y1, x2, y2};
		return;
	}

	int x3 = (x1 + x2) / 2;
	int y3 = (y1 + y2) / 2;

	if (dx < 9)
		dx = 9;
	if (dy < 9)
		dy = 9;

	/* perturb the midpoint using xorshift */
	int rx = (xorshift(&xorshift_state) % (2*dx)) - dx;
	int ry = (xorshift(&xorshift_state) % (2*dy)) - dy;

	x3 += (rx * 2) / 5;
	y3 += (ry * 2) / 5;

#if DEBUG_LIGHTNING
	printf("subdivide: %d, %d, %d, %d, %d, %d\n", x1, y1, x2, y2, x3, y3);
#endif
	subdivide_lightning(x1, y1, x3, y3);
	subdivide_lightning(x3, y3, x2, y2);
}

static void draw_lightning(void)
{
	if (!lightning_active) return;

	FbColor(WHITE);
	for (int i = 0; i < lightning_count; i++) {
#if DEBUG_LIGHTNING
		printf("segment-white: %d, %d, %d, %d\n",
			(unsigned char)lightning[i].x1,
			(unsigned char)lightning[i].y1,
			(unsigned char)lightning[i].x2,
			(unsigned char)lightning[i].y2
		);
#endif
		FbLine(
			(unsigned char)lightning[i].x1,
			(unsigned char)lightning[i].y1,
			(unsigned char)lightning[i].x2,
			(unsigned char)lightning[i].y2
		);
	}

	FbColor(BLUE);
	for (int i = 0; i < lightning_count; i++) {
#if DEBUG_LIGHTNING
		printf("segment-blue: %d, %d, %d, %d\n",
			(unsigned char)lightning[i].x1,
			(unsigned char)lightning[i].y1,
			(unsigned char)lightning[i].x2,
			(unsigned char)lightning[i].y2
		);
#endif
		FbLine(
			(unsigned char)(lightning[i].x1 - 1),
			(unsigned char) lightning[i].y1,
			(unsigned char)(lightning[i].x2 - 1),
			(unsigned char) lightning[i].y2
		);
		FbLine(
			(unsigned char)(lightning[i].x1 + 1),
			(unsigned char) lightning[i].y1,
			(unsigned char)(lightning[i].x2 + 1),
			(unsigned char) lightning[i].y2
		);
	}

	if (--lightning_frames_left <= 0)
	  lightning_active = false;
}

/*
 * pixel_coordinate
 * i was using point from framebuffer.h for this, but
 * i want them to be unsigned
 */
struct pixel_coordinate {
	uint16_t x;
	uint16_t y;
};

static struct pixel_coordinate get_random_pixel_coordinate_top(void)
{
	struct pixel_coordinate p = {
		.x = (xorshift(&xorshift_state) % (LCD_XSIZE - 1)),
		.y = 5,
	};
#if DEBUG_LIGHTNING
	printf("lighting source: %d\n", p.x);
#endif
	return p;
}

static struct pixel_coordinate get_center_of_block(int bx, int by, int origin_x, int origin_y, int spacing, int center_offset)
{
	struct pixel_coordinate p;
	p.x = origin_x + bx * spacing + center_offset;
	p.y = origin_y + by * spacing + center_offset;
	return p;
}

static void reset_lightning_state(void)
{
	lightning_count = 0;
	lightning_active = true;
	lightning_frames_left = 15;
}
#endif

enum {
	MATCH_LEVEL_NONE = 0,
	MATCH_LEVEL_PARTICLES = 1,
	MATCH_LEVEL_LIGHTNING = 6,
};

static int get_match_count(void)
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

#if ENABLE_LIGHTNING
	if (match_count > MATCH_LEVEL_LIGHTNING) {
		int spacing = BLOCK_SIZE + BLOCK_SPACING;
		int origin_x = (LCD_XSIZE / 2) - (GRID_COLS * spacing / 2);
		int origin_y = 1 + 2;
		int center_offset = (BLOCK_SIZE + 3) / 2;

		reset_lightning_state();

		/*
		 * we try to target the center of each block
		 * with not many segments, it will only target a few.
		 * it's also possible that the lightning bolts
		 * won't reach their targets (because of the segment limit).
		 */
		for (int m = 0; m < match_count; m++) {
			if (lightning_count >= MAX_LIGHTNING_SEGS)
				break;

			int bx = match_x[m];
			int by = match_y[m];
			struct pixel_coordinate center = get_center_of_block(bx, by, origin_x, origin_y, spacing, center_offset);
			struct pixel_coordinate p = get_random_pixel_coordinate_top();

			subdivide_lightning(
				p.x,
				p.y,
				center.x,
				center.y
			);
		}
	}
#endif

	return match_count;
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
					bool r = is_marked_for_removal(x, y);
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
			if (is_marked_for_removal(x, y)) {
				uint8_t p = get_removal_progress(x, y) + 1;
				if (p > REMOVE_BLOCK_ANIMATION_END)
					set_cell(x, y, EMPTY_BLOCK, false, 0);
				else
					set_cell(x, y, block_get_type(x, y),
						true, p);
			}
		}
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

static int theme_duration = 200;
static int sfx_duration = 0;

static int calculate_tune_duration(struct note *notes, size_t note_count)
{
	int total_duration = 0;
	for (size_t i = 0; i < note_count; ++i) {
		total_duration += notes[i].duration;
	}
	return total_duration;
}

static void update_audio(uint64_t now)
{
	if (audio_mode == AUDIO_SFX) {
		struct note *n = &current_sfx[sfx_index];

		if (now < sfx_note_start + n->duration)
		  return;

		sfx_index++;

		if (sfx_index < current_sfx_len) {
			audio_out_beep(n->freq, n->duration);
			sfx_note_start = now;
			return;
		}
		audio_mode = AUDIO_THEME;
		theme_note_start += sfx_duration;
		return;
	}

	struct note *n = &puzzle_attack_theme_notes[theme_index];
	if (now >= theme_note_start + n->duration) {
		theme_index = (theme_index + 1) % puzzle_attack_theme.num_notes;
		theme_note_start = now;
		struct note *next = &puzzle_attack_theme_notes[theme_index];
		audio_out_beep(next->freq, next->duration);
	}
}

static void puzzle_attack_update(void)
{
	uint64_t now = rtc_get_ms_since_boot();
	if (now > collapse_cooldown) {
		collapse_grid();
		collapse_cooldown = rtc_get_ms_since_boot()+COLLAPSE_GRID_MS;
		tick = (tick + 1) % 60;
		last_tick_time = now;
	}
	if (now > cycle_cooldown) {
		cycle_cooldown = now + EVAL_CYCLE_MS;

		int match_count = get_match_count();
		register_blocks_for_removal();

		if (match_count > 0 && audio_mode == AUDIO_THEME) {
			if (match_count > MATCH_LEVEL_LIGHTNING) {
				current_sfx = sfx_two;
				current_sfx_len = ARRAYSIZE(sfx_two);
			} else {
				current_sfx = sfx_one;
				current_sfx_len = ARRAYSIZE(sfx_one);
			}
			audio_mode = AUDIO_SFX;
			sfx_index = 0;
			sfx_note_start = now;
		}
		sfx_duration = calculate_tune_duration(current_sfx, current_sfx_len);
	}
	if (now > grid_shift_cooldown) {
		grid_shift_cooldown = rtc_get_ms_since_boot()+GRID_SHIFT_MS;
		shift_grid_up();
		insert_row();
		shift_cursor_up();
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
	score = 0;
	cycle_cooldown = rtc_get_ms_since_boot()+EVAL_CYCLE_MS;
	grid_shift_cooldown = rtc_get_ms_since_boot()+GRID_SHIFT_MS;
	collapse_cooldown = rtc_get_ms_since_boot()+COLLAPSE_GRID_MS;
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

	if (is_marked_for_removal(grid_x, grid_y))
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
	unsigned int now = rtc_get_ms_since_boot();
	unsigned int elapsed = now - (cycle_cooldown - EVAL_CYCLE_MS);
	if (elapsed > EVAL_CYCLE_MS) elapsed = EVAL_CYCLE_MS;

	int fill_percent = (int)((elapsed * 100) / EVAL_CYCLE_MS);

	struct ui_progress_bar pb = {
		.x = 1,
		.y = 18,
		.width = 40,
		.height = 12,
		.outline_size = 2,
		.fill_color = palette_color_from_index(default_palette,12),
		.empty_color = palette_color_from_index(default_palette,0),
		.outline_color = palette_color_from_index(default_palette,13),
		.fill = ui_progress_bar_calculate_fill_percentage(fill_percent),
	};
	ui_progress_bar_draw(pb);
#if 0
	char buf[8];
	snprintf(buf, sizeof(buf), "%3d", tick);
	FbMove(10, 20);
	FbColor(palette_color_from_index(
		default_palette, SCORE_COLOR_INDEX + 1));
	FbWriteString(buf);
	has_screen_changed = 1;
#endif
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

#if ENABLE_LIGHTNING
	draw_lightning();

	/* if lightning is active, flicker white for some frames */
	if (lightning_active && (lightning_frames_left % 4 < 2)) {
		FbColor(WHITE);
		FbMove(0, 0);
		FbFilledRectangle(LCD_XSIZE, LCD_YSIZE);
	}
#endif
}

void puzzle_attack_cb(struct badge_app *app)
{
	if (app->wake_up)
		has_screen_changed = 1;

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
		theme_duration = calculate_tune_duration(puzzle_attack_theme_notes, puzzle_attack_theme.num_notes);
		theme_duration += quarter_note;
		break;
	case PUZZLE_ATTACK_RUN:
		puzzle_attack_update();
		draw_screen();
		update_audio(rtc_get_ms_since_boot());
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
		stop_tune();
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
