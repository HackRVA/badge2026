/*
 * Daywalker - an attempt at a vampire survivors style auto-battler.
 *
 * You move. Weapons fire automatically. Enemies swarm.
 * Kill enemies to drop XP gems. Collect gems to level up.
 * Pick upgrades. Survive as long as you can.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "badge.h"
#include "button.h"
#include "colors.h"
#include "framebuffer.h"
#include "fxp_sqrt.h"
#include "menu.h"
#include "palette.h"
#include "rtc.h"
#include "trig.h"
#include "ui.h"
#include "xorshift.h"

#include "daywalker.h"

#define WORLD_WIDTH 512
#define WORLD_HEIGHT 512

#define MAX_ENEMIES 48
#define MAX_GEMS 48
#define MAX_DAMAGE_NUMBERS 12

#define GEM_COLLECT_RADIUS 16
#define GEM_MAGNET_RADIUS 32
#define GEM_MAGNET_SPEED 3

#define MAX_BOLTS 10

#define WEAPON_ORBIT_BASE_COUNT 1
#define WEAPON_ORBIT_PER_LEVEL_COUNT 1
#define WEAPON_ORBIT_BASE_RADIUS 18
#define WEAPON_ORBIT_PER_LEVEL_RADIUS 3

#define WEAPON_BOLT_BASE_DAMAGE 2
#define WEAPON_BOLT_PER_LEVEL_DAMAGE 1
#define WEAPON_BOLT_BASE_DELAY 30
#define WEAPON_BOLT_PER_LEVEL_RATE 4
#define WEAPON_BOLT_MIN_DELAY 8

#define WEAPON_AURA_BASE_RADIUS 14
#define WEAPON_AURA_PER_LEVEL_RADIUS 4

#define WEAPON_CHAIN_BASE_DELAY 64
#define WEAPON_CHAIN_PER_LEVEL_RATE 5
#define WEAPON_CHAIN_MIN_DELAY 15

#define WEAPON_LEVEL_CAP 5
#define MAX_CHAIN_POINTS (2 + WEAPON_LEVEL_CAP)

#define RANGED_ACTIVATE_DISTANCE 30
#define RANGED_ACTIVATE_DISTANCE_SQUARED (RANGED_ACTIVATE_DISTANCE * RANGED_ACTIVATE_DISTANCE)

#define DAMAGE_NUMBER_LIFETIME 30
#define DAMAGE_NUMBER_RISE_SPEED 1

#define FP 8
#define FP_ONE (1 << FP)
#define TO_FP(x) ((x) << FP)
#define TO_INT(x) ((x) >> FP)

static int camera_x, camera_y;
static unsigned int elapsed_frames;

static const struct palette daywalker_palette = {
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

#define PC(i) palette_color_from_index(daywalker_palette, (i))

#define DEBUG_BEEP_ENABLED 0
static void sfx_debug_beep(uint16_t freq, uint16_t duration)
{
#if DEBUG_BEEP_ENABLED
	audio_out_beep(freq, duration);
#endif
}
static void sfx_enemy_die(void)    { sfx_debug_beep(380,  60); }
static void sfx_player_hurt(void)  { sfx_debug_beep(180, 120); }
static void sfx_orbit_hit(void)    { sfx_debug_beep(1300, 12); }
static void sfx_bolt_fire(void)    { sfx_debug_beep(1800, 18); }
static void sfx_chain_cast(void)   { sfx_debug_beep(2200, 35); }
static void sfx_aura_pulse(void)   { sfx_debug_beep(600,  45); }
static void sfx_chain_hit(void)    { sfx_debug_beep(2800, 12); }
static void sfx_bolt_hit(void)     { sfx_debug_beep(2400, 15); }
static void sfx_aura_hit(void)     { sfx_debug_beep(800,  12); }
static void sfx_gem(void)          { sfx_debug_beep(1400, 25); }
static void sfx_level_up(void)     { sfx_debug_beep(1500, 180); }
static void sfx_upgrade_pick(void) { sfx_debug_beep(1200, 60); }
static void sfx_menu_select(void)  { sfx_debug_beep(900,  40); }

static unsigned int rng_state;

static unsigned int rng(void)
{
	return xorshift(&rng_state);
}

static int rng_range(int lo, int hi)
{
	if (lo >= hi)
		return lo;
	return lo + (int) (rng() % (unsigned int) (hi - lo));
}

static inline int clamp(int v, int lo, int hi)
{
	if (v < lo)
		return lo;
	if (v > hi)
		return hi;
	return v;
}

static void write_string(const char *string)
{
	int prev_transparent_index = FbGetTransparentIndex();
	FbTransparentIndex(0);
	FbWriteString(string);
	FbTransparentIndex(prev_transparent_index);
}

/* Convert squared FP distance to FP distance, saturated to >= 1 so callers
 * can always divide by the result without a separate zero-check. */
static int fp_distance_from_squared(int64_t distance_squared)
{
	int distance = fxp_sqrt((int) (distance_squared >> 8));
	return distance < 1 ? 1 : distance;
}

/* True if a sprite at screen pos (sx, sy) is entirely outside the LCD,
 * given a half-margin in pixels. */
static int offscreen(int sx, int sy, int margin)
{
	return sx < -margin || sx >= LCD_XSIZE + margin ||
	       sy < -margin || sy >= LCD_YSIZE + margin;
}

/* Circle overlap in fixed-point space; radius is in world pixels. */
static int overlap_fp(int ax, int ay, int bx, int by, int r_world)
{
	int dx = ax - bx, dy = ay - by;
	int64_t r = TO_FP(r_world);
	return (int64_t) dx * dx + (int64_t) dy * dy < r * r;
}

/* Circle overlap in integer pixel space. */
static int overlap_int(int ax, int ay, int bx, int by, int r)
{
	int dx = ax - bx, dy = ay - by;
	return dx * dx + dy * dy < r * r;
}

/*
 * draw_sprite iterates through *data (left to right top to bottom)
 * each nibble represents a pixel
 */
static void draw_sprite(int sx, int sy, const unsigned char *data)
{
	for (int row = 0; row < 8; row++) {
		int dy = sy + row;
		if (dy < 0 || dy >= LCD_YSIZE)
			continue;
		for (int b = 0; b < 4; b++) {
			unsigned char byte = data[row * 4 + b];
			unsigned char hi = (byte >> 4) & 0x0F;
			unsigned char lo = byte & 0x0F;
			int dx = sx + b * 2;
			if (hi && dx >= 0 && dx < LCD_XSIZE) {
				FbColor(PC(hi));
				FbPoint(dx, dy);
			}
			if (lo && dx + 1 >= 0 && dx + 1 < LCD_XSIZE) {
				FbColor(PC(lo));
				FbPoint(dx + 1, dy);
			}
		}
	}
}

static const unsigned char sprite_player_down0[] = {
	0x00, 0xAA, 0xA0, 0x00, 0x00, 0xAA, 0xA0, 0x00, 0x00, 0x07, 0x70,
	0x00, 0x00, 0xCC, 0xC0, 0x00, 0x0C, 0x0C, 0x0C, 0x00, 0x00, 0x0C,
	0x00, 0x00, 0x00, 0x02, 0x20, 0x00, 0x00, 0x20, 0x02, 0x00,
};
static const unsigned char sprite_player_down1[] = {
	0x00, 0xAA, 0xA0, 0x00, 0x00, 0xAA, 0xA0, 0x00, 0x00, 0x07, 0x70,
	0x00, 0x00, 0xCC, 0xC0, 0x00, 0x00, 0xCC, 0x0C, 0x00, 0x00, 0x0C,
	0x00, 0x00, 0x00, 0x02, 0x20, 0x00, 0x00, 0x02, 0x20, 0x00,
};
static const unsigned char sprite_player_up0[] = {
	0x00, 0xAA, 0xA0, 0x00, 0x00, 0xAA, 0xA0, 0x00, 0x00, 0x0A, 0xA0,
	0x00, 0x00, 0xCC, 0xC0, 0x00, 0x0C, 0x0C, 0x0C, 0x00, 0x00, 0x0C,
	0x00, 0x00, 0x00, 0x02, 0x20, 0x00, 0x00, 0x20, 0x02, 0x00,
};
static const unsigned char sprite_player_up1[] = {
	0x00, 0xAA, 0xA0, 0x00, 0x00, 0xAA, 0xA0, 0x00, 0x00, 0x0A, 0xA0,
	0x00, 0x00, 0xCC, 0xC0, 0x00, 0x00, 0xCC, 0x0C, 0x00, 0x00, 0x0C,
	0x00, 0x00, 0x00, 0x02, 0x20, 0x00, 0x00, 0x02, 0x20, 0x00,
};
static const unsigned char sprite_player_right0[] = {
	0x00, 0xAA, 0xA0, 0x00, 0x00, 0xAA, 0xA0, 0x00, 0x00, 0x07, 0x70,
	0x00, 0x00, 0xCC, 0xC0, 0x00, 0x00, 0xCC, 0xC0, 0x00, 0x00, 0x0C,
	0xC0, 0x00, 0x00, 0x02, 0x20, 0x00, 0x00, 0x20, 0x02, 0x00,
};
static const unsigned char sprite_player_right1[] = {
	0x00, 0xAA, 0xA0, 0x00, 0x00, 0xAA, 0xA0, 0x00, 0x00, 0x07, 0x70,
	0x00, 0x00, 0xCC, 0xC0, 0x00, 0x00, 0xCC, 0xC0, 0x00, 0x00, 0x0C,
	0xC0, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00, 0x02, 0x20, 0x00,
};
static const unsigned char sprite_player_left0[] = {
	0x00, 0x0A, 0xAA, 0x00, 0x00, 0x0A, 0xAA, 0x00, 0x00, 0x07, 0x70,
	0x00, 0x00, 0x0C, 0xCC, 0x00, 0x00, 0x0C, 0xCC, 0x00, 0x00, 0x0C,
	0xC0, 0x00, 0x00, 0x02, 0x20, 0x00, 0x00, 0x20, 0x02, 0x00,
};
static const unsigned char sprite_player_left1[] = {
	0x00, 0x0A, 0xAA, 0x00, 0x00, 0x0A, 0xAA, 0x00, 0x00, 0x07, 0x70,
	0x00, 0x00, 0x0C, 0xCC, 0x00, 0x00, 0x0C, 0xCC, 0x00, 0x00, 0x0C,
	0xC0, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00, 0x02, 0x20, 0x00,
};

#define DIR_DOWN 0
#define DIR_UP 1
#define DIR_RIGHT 2
#define DIR_LEFT 3

static const unsigned char *const player_frames[4][2] = {
	{ sprite_player_down0, sprite_player_down1 },
	{ sprite_player_up0, sprite_player_up1 },
	{ sprite_player_right0, sprite_player_right1 },
	{ sprite_player_left0, sprite_player_left1 },
};

#define DIGIT_WIDTH 4
#define DIGIT_HEIGHT 5

static const unsigned char digit_bitmap[10][10] = {
	{ 0x08, 0x80, 0x80, 0x08, 0x80, 0x08, 0x80, 0x08, 0x08, 0x80 },
	{ 0x08, 0x00, 0x88, 0x00, 0x08, 0x00, 0x08, 0x00, 0x88, 0x80 },
	{ 0x08, 0x80, 0x80, 0x08, 0x00, 0x80, 0x08, 0x00, 0x88, 0x88 },
	{ 0x88, 0x80, 0x00, 0x08, 0x08, 0x80, 0x00, 0x08, 0x88, 0x80 },
	{ 0x80, 0x80, 0x80, 0x80, 0x88, 0x80, 0x00, 0x80, 0x00, 0x80 },
	{ 0x88, 0x80, 0x80, 0x00, 0x88, 0x80, 0x00, 0x08, 0x88, 0x80 },
	{ 0x08, 0x80, 0x80, 0x00, 0x88, 0x80, 0x80, 0x08, 0x08, 0x80 },
	{ 0x88, 0x88, 0x00, 0x08, 0x00, 0x80, 0x08, 0x00, 0x08, 0x00 },
	{ 0x08, 0x80, 0x80, 0x08, 0x08, 0x80, 0x80, 0x08, 0x08, 0x80 },
	{ 0x08, 0x80, 0x80, 0x08, 0x08, 0x88, 0x00, 0x08, 0x08, 0x80 },
};

static void draw_digit_bitmap(int sx, int sy, int digit)
{
	if (digit < 0 || digit > 9)
		return;
	const unsigned char *bitmap = digit_bitmap[digit];
	for (int row = 0; row < DIGIT_HEIGHT; row++) {
		for (int col = 0; col < DIGIT_WIDTH; col++) {
			int bi = row * (DIGIT_WIDTH / 2) + col / 2;
			int nibble = (col & 1) ? (bitmap[bi] & 0x0F) : (bitmap[bi] >> 4);
			if (nibble == 0)
				continue;
			int px = sx + col, py = sy + row;
			if (px >= 0 && px < LCD_XSIZE && py >= 0 &&
			    py < LCD_YSIZE)
				FbPoint(px, py);
		}
	}
}

static void draw_number_bitmap(int sx, int sy, int value)
{
	if (value < 0)
		value = 0;
	if (value > 99)
		value = 99;
	if (value >= 10) {
		draw_digit_bitmap(sx, sy, value / 10);
		draw_digit_bitmap(sx + DIGIT_WIDTH, sy, value % 10);
	} else {
		draw_digit_bitmap(sx, sy, value);
	}
}

struct damage_number {
	short world_x, world_y;
	unsigned char value;
	signed char timer;
};

static struct damage_number damage_numbers[MAX_DAMAGE_NUMBERS];

static void spawn_damage_number(int wx, int wy, int value)
{
	for (int i = 0; i < MAX_DAMAGE_NUMBERS; i++) {
		if (damage_numbers[i].timer > 0)
			continue;
		damage_numbers[i].world_x = (short) wx;
		damage_numbers[i].world_y = (short) wy;
		damage_numbers[i].value = (unsigned char) (value > 99 ? 99 : value);
		damage_numbers[i].timer = DAMAGE_NUMBER_LIFETIME;
		return;
	}
}

static void update_damage_numbers(void)
{
	for (int i = 0; i < MAX_DAMAGE_NUMBERS; i++) {
		if (damage_numbers[i].timer <= 0)
			continue;
		damage_numbers[i].world_y -= DAMAGE_NUMBER_RISE_SPEED;
		damage_numbers[i].timer--;
	}
}

static void draw_damage_numbers(void)
{
	for (int i = 0; i < MAX_DAMAGE_NUMBERS; i++) {
		if (damage_numbers[i].timer <= 0)
			continue;
		int sx = damage_numbers[i].world_x - camera_x - 2;
		int sy = damage_numbers[i].world_y - camera_y;
		if (offscreen(sx, sy, 8))
			continue;
		int t = damage_numbers[i].timer;
		if (t > DAMAGE_NUMBER_LIFETIME * 2 / 3)
			FbColor(PC(8));
		else if (t > DAMAGE_NUMBER_LIFETIME / 3)
			FbColor(PC(9));
		else
			FbColor(PC(5));
		draw_number_bitmap(sx, sy, damage_numbers[i].value);
	}
}

static const unsigned char sprite_bat0[] = {
	0x00, 0x00, 0x00, 0x00, 0xD0, 0x0D, 0xD0, 0x0D, 0xDD, 0xDD, 0xDD,
	0xDD, 0x0D, 0x7D, 0xD7, 0xD0, 0x0D, 0xDD, 0xDD, 0xD0, 0x00, 0xD0,
	0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
static const unsigned char sprite_bat1[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x0D, 0xD0,
	0xD0, 0x0D, 0x7D, 0xD7, 0xD0, 0x0D, 0xDD, 0xDD, 0xD0, 0xD0, 0xDD,
	0xDD, 0x0D, 0x00, 0xD0, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const unsigned char sprite_slime0[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0xB0,
	0x00, 0x0B, 0x7B, 0xB7, 0xB0, 0x0B, 0xBB, 0xBB, 0xB0, 0xBB, 0xBB,
	0xBB, 0xBB, 0x03, 0x33, 0x33, 0x30, 0x00, 0x00, 0x00, 0x00,
};
static const unsigned char sprite_slime1[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x0B, 0x7B, 0xB7, 0xB0, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB,
	0xBB, 0xBB, 0x03, 0x33, 0x33, 0x30, 0x00, 0x00, 0x00, 0x00,
};

static const unsigned char sprite_skeleton0[] = {
	0x00, 0x77, 0x77, 0x00, 0x00, 0x77, 0x77, 0x00, 0x00, 0x07, 0x70,
	0x00, 0x00, 0x87, 0x78, 0x00, 0x07, 0x07, 0x70, 0x70, 0x00, 0x07,
	0x70, 0x00, 0x00, 0x07, 0x70, 0x00, 0x00, 0x70, 0x07, 0x00,
};
static const unsigned char sprite_skeleton1[] = {
	0x00, 0x77, 0x77, 0x00, 0x00, 0x77, 0x77, 0x00, 0x00, 0x07, 0x70,
	0x00, 0x00, 0x87, 0x78, 0x00, 0x00, 0x77, 0x70, 0x70, 0x00, 0x07,
	0x70, 0x00, 0x00, 0x07, 0x70, 0x00, 0x07, 0x00, 0x00, 0x70,
};

static struct {
	int x, y;
	short health, max_health;
	uint32_t experience;
	uint32_t experience_next;
	unsigned short kills;
	uint16_t level;
	unsigned char damage_flash;
	unsigned char direction;
	unsigned char animation_frame;
	signed char animation_timer;
	unsigned char moving;
} player;

static unsigned char speed_level;

struct gem {
	short x, y;
	unsigned char value;
};

static struct gem gems[MAX_GEMS];

static void update_gems(void)
{
	int px_fp = player.x;
	int py_fp = player.y;

	int64_t magnet_radius_squared =
	    (int64_t) TO_FP(GEM_MAGNET_RADIUS) * TO_FP(GEM_MAGNET_RADIUS);
	int64_t collect_radius_squared =
	    (int64_t) TO_FP(GEM_COLLECT_RADIUS) * TO_FP(GEM_COLLECT_RADIUS);

	for (int i = 0; i < MAX_GEMS; i++) {
		if (gems[i].value == 0)
			continue;

		int dx = TO_FP((int) gems[i].x) - px_fp;
		int dy = TO_FP((int) gems[i].y) - py_fp;
		int64_t distance_squared = (int64_t) dx * dx + (int64_t) dy * dy;

		if (distance_squared < magnet_radius_squared && distance_squared > 0) {
			int distance = fp_distance_from_squared(distance_squared);
			gems[i].x -= (short) (dx * GEM_MAGNET_SPEED / distance);
			gems[i].y -= (short) (dy * GEM_MAGNET_SPEED / distance);
			dx = TO_FP((int) gems[i].x) - px_fp;
			dy = TO_FP((int) gems[i].y) - py_fp;
			distance_squared = (int64_t) dx * dx + (int64_t) dy * dy;
		}

		if (distance_squared < collect_radius_squared) {
			player.experience += gems[i].value;
			gems[i].value = 0;
			sfx_gem();
		}
	}
}

static void draw_gems(void)
{
	int shimmer = (elapsed_frames / 5) & 1;
	for (int i = 0; i < MAX_GEMS; i++) {
		if (gems[i].value == 0)
			continue;
		int sx = gems[i].x - camera_x;
		int sy = gems[i].y - camera_y;
		if (offscreen(sx, sy, 2))
			continue;
		FbColor(shimmer ? PC(12) : PC(7));
		FbPoint(sx, sy);
		if (sx + 1 < LCD_XSIZE)
			FbPoint(sx + 1, sy);
		if (sy + 1 < LCD_YSIZE)
			FbPoint(sx, sy + 1);
	}
}

static void spawn_gem(int wx, int wy, int value)
{
	for (int i = 0; i < MAX_GEMS; i++) {
		if (gems[i].value != 0)
			continue;
		gems[i].x = (short) wx;
		gems[i].y = (short) wy;
		gems[i].value = (unsigned char) (value > 255 ? 255 : value);
		return;
	}
}

#define ENEMY_NONE 0
#define ENEMY_BAT 1
#define ENEMY_SLIME 2
#define ENEMY_SKELETON 3
#define NUM_ENEMY_TYPES 4

static const struct enemy_def {
	const unsigned char *sprite[2];
	unsigned char animation_rate;
	short speed;
	short health;
	unsigned char attack;
	unsigned char gem_value;
	unsigned char show_health_bar;
	unsigned char health_bar_color;
} enemy_definitions[NUM_ENEMY_TYPES] = {
	[ENEMY_NONE] = { { NULL, NULL }, 0, 0, 0, 0, 0, 0, 0 },
	[ENEMY_BAT] = { { sprite_bat0, sprite_bat1 }, 4, FP_ONE + FP_ONE / 2, 2, 2, 1, 1, 8 },
	[ENEMY_SLIME] = { { sprite_slime0, sprite_slime1 }, 12, FP_ONE / 2, 4, 1, 4, 1, 8 },
	[ENEMY_SKELETON] = { { sprite_skeleton0, sprite_skeleton1 }, 8, FP_ONE, 6, 2, 6, 1, 8 },
};

struct enemy {
	int x, y;
	short vx, vy;
	short health;
	unsigned char type;
	unsigned char damage_flash;
	unsigned char attack_cooldown;
	unsigned char orbit_cooldown;
};

static struct enemy enemies[MAX_ENEMIES];

static void hurt_enemy(struct enemy *e, int damage)
{
	e->health -= (short) damage;
	e->damage_flash = 4;
	spawn_damage_number(TO_INT(e->x), TO_INT(e->y), damage);
	if (e->health <= 0) {
		int wx = TO_INT(e->x);
		int wy = TO_INT(e->y);
		spawn_gem(wx, wy, enemy_definitions[e->type].gem_value);
		e->type = ENEMY_NONE;
		player.kills++;
		sfx_enemy_die();
	}
}

static short spawn_timer;

/* Pick a spawn location offscreen-ish around the player, clamped to world bounds. */
static void pick_offscreen_spawn(int *out_sx, int *out_sy)
{
	int px = TO_INT(player.x), py = TO_INT(player.y);
	int sx, sy;
	switch (rng_range(0, 4)) {
	case 0:
		sx = px + rng_range(-100, 100);
		sy = py - 90;
		break;
	case 1:
		sx = px + rng_range(-100, 100);
		sy = py + 90;
		break;
	case 2:
		sx = px - 100;
		sy = py + rng_range(-80, 80);
		break;
	default:
		sx = px + 100;
		sy = py + rng_range(-80, 80);
		break;
	}
	*out_sx = clamp(sx, 4, WORLD_WIDTH - 12);
	*out_sy = clamp(sy, 4, WORLD_HEIGHT - 12);
}

static int spawn_enemy(int type)
{
	int slot = -1;
	for (int i = 0; i < MAX_ENEMIES; i++)
		if (enemies[i].type == ENEMY_NONE) {
			slot = i;
			break;
		}
	if (slot < 0)
		return 0;

	struct enemy *e = &enemies[slot];
	e->type = (unsigned char) type;
	e->health = enemy_definitions[type].health;
	e->damage_flash = 0;
	e->attack_cooldown = 0;
	e->orbit_cooldown = 0;
	e->vx = e->vy = 0;

	int sx, sy;
	pick_offscreen_spawn(&sx, &sy);
	e->x = TO_FP(sx);
	e->y = TO_FP(sy);
	return 1;
}

static void update_mob_wave(int minute)
{
	int rate = 30 - minute * 4;
	if (rate < 6)
		rate = 6;
	spawn_timer = (short) rate;

	int burst = 1 + minute / 2;
	if (burst > 4)
		burst = 4;

	for (int b = 0; b < burst; b++) {
		int r = rng_range(0, 10 + minute * 3);
		if (r < 5)
			spawn_enemy(ENEMY_BAT);
		else if (r < 8)
			spawn_enemy(ENEMY_SLIME);
		else
			spawn_enemy(ENEMY_SKELETON);
	}
}

static void update_spawns(void)
{
	spawn_timer--;
	if (spawn_timer > 0)
		return;
	update_mob_wave(elapsed_frames / (30 * 60));
}

static void enemy_apply_knockback(struct enemy *e)
{
	e->x += e->vx;
	e->y += e->vy;
	e->vx /= 2;
	e->vy /= 2;
}

static void enemy_chase_player(struct enemy *e, int px, int py)
{
	int dx = px - e->x;
	int dy = py - e->y;
	int distance = fp_distance_from_squared((int64_t) dx * dx + (int64_t) dy * dy);
	int speed = enemy_definitions[e->type].speed;
	if (speed > distance)
		speed = distance;
	e->x += (dx * speed) / distance;
	e->y += (dy * speed) / distance;
}

static void enemy_contact_damage(struct enemy *e, int px, int py)
{
	if (overlap_int(TO_INT(e->x), TO_INT(e->y), TO_INT(px), TO_INT(py), 6) &&
	    e->attack_cooldown == 0) {
		player.health -= enemy_definitions[e->type].attack;
		player.damage_flash = 8;
		e->attack_cooldown = 30;
		hurt_enemy(e, 1);
		sfx_player_hurt();
	}
}

static void update_enemies(void)
{
	int px = player.x, py = player.y;

	for (int i = 0; i < MAX_ENEMIES; i++) {
		struct enemy *e = &enemies[i];
		if (e->type == ENEMY_NONE)
			continue;

		if (e->damage_flash > 0)
			e->damage_flash--;
		if (e->attack_cooldown > 0)
			e->attack_cooldown--;
		if (e->orbit_cooldown > 0)
			e->orbit_cooldown--;

		enemy_apply_knockback(e);
		enemy_chase_player(e, px, py);
		enemy_contact_damage(e, px, py);
	}
}

static void draw_enemy_damage_flash(struct enemy *e, int sx, int sy)
{
	if (e->damage_flash <= 0)
		return;

	FbColor(PC(7));
	for (int dy = 1; dy < 7; dy++)
		for (int dx = 1; dx < 7; dx++)
			if (sx + dx >= 0 && sx + dx < LCD_XSIZE &&
			    sy + dy >= 0 && sy + dy < LCD_YSIZE)
				FbPoint(sx + dx, sy + dy);
}

static void draw_enemy_health_bar(struct enemy *e, int sx, int sy)
{
	if (!enemy_definitions[e->type].show_health_bar)
		return;
	if (e->health <= 0)
		return;

	int max_health = enemy_definitions[e->type].health;
	int percent = clamp(e->health * 100 / max_health, 0, 100);
	struct ui_progress_bar ehp = {
		.x = sx - 2,
		.y = sy - 2,
		.width = 12,
		.height = 1,
		.outline_size = 0,
		.fill_color = PC(enemy_definitions[e->type].health_bar_color),
		.empty_color = PC(5),
		.outline_color = PC(5),
		.fill = ui_progress_bar_calculate_fill_percentage(percent),
	};
	ui_progress_bar_draw(ehp);
}

static void draw_enemies(void)
{
	for (int i = 0; i < MAX_ENEMIES; i++) {
		struct enemy *e = &enemies[i];
		if (e->type == ENEMY_NONE)
			continue;

		int sx = TO_INT(e->x) - camera_x;
		int sy = TO_INT(e->y) - camera_y;
		if (offscreen(sx, sy, 8))
			continue;

		draw_enemy_damage_flash(e, sx, sy);
		int rate = enemy_definitions[e->type].animation_rate;
		int frame = (elapsed_frames / rate) & 1;
		draw_sprite(sx, sy, enemy_definitions[e->type].sprite[frame]);
		draw_enemy_health_bar(e, sx, sy);
	}
}

struct bolt {
	int x, y;
	short vx, vy;
	signed char damage;
	signed char life;
};

static struct bolt bolts[MAX_BOLTS];

#define WEAPON_ORBIT 0
#define WEAPON_BOLT 1
#define WEAPON_AURA 2
#define WEAPON_CHAIN 3
#define NUM_WEAPONS 4

static unsigned char weapons[NUM_WEAPONS];

static void weapon_orbit_init(void);
static void weapon_orbit_update(int px_fp, int py_fp);
static void weapon_orbit_draw(int px_scr, int py_scr);

static void weapon_bolt_init(void);
static void weapon_bolt_update(int px_fp, int py_fp);
static void weapon_bolt_draw(int px_scr, int py_scr);

static void weapon_aura_init(void);
static void weapon_aura_update(int px_fp, int py_fp);
static void weapon_aura_draw(int px_scr, int py_scr);

static void weapon_chain_init(void);
static void weapon_chain_update(int px_fp, int py_fp);
static void weapon_chain_draw(int px_scr, int py_scr);

typedef void (*weapon_function_void)(void);
typedef void (*weapon_function_2i)(int, int);

static const struct weapon_def {
	weapon_function_void init;
	weapon_function_2i update;
	weapon_function_2i draw;
} weapon_definitions[NUM_WEAPONS] = {
	[WEAPON_ORBIT] = { weapon_orbit_init, weapon_orbit_update, weapon_orbit_draw },
	[WEAPON_BOLT]  = { weapon_bolt_init,  weapon_bolt_update,  weapon_bolt_draw  },
	[WEAPON_AURA]  = { weapon_aura_init,  weapon_aura_update,  weapon_aura_draw  },
	[WEAPON_CHAIN] = { weapon_chain_init, weapon_chain_update, weapon_chain_draw },
};

/*
 * DDA circle outline -- efficient integer alg picked up from Casey Muratori.
 */
static void FbDDACircle(int cx, int cy, int radius)
{
	int r2 = radius + radius;
	int x = radius, y = 0;
	int delta_y = -2, delta_x = r2 + r2 - 4, delta = r2 - 1;
	while (y <= x) {
		FbPoint(cx - x, cy - y);
		FbPoint(cx + x, cy - y);
		FbPoint(cx - x, cy + y);
		FbPoint(cx + x, cy + y);
		FbPoint(cx - y, cy - x);
		FbPoint(cx + y, cy - x);
		FbPoint(cx - y, cy + x);
		FbPoint(cx + y, cy + x);
		delta += delta_y;
		delta_y -= 4;
		++y;
		int mask = (delta >> 31);
		delta += delta_x & mask;
		delta_x -= 4 & mask;
		x += mask;
	}
}

static unsigned char orbit_angle;

static int orbit_damage(void)
{
	return weapons[WEAPON_ORBIT];
}
static int orbit_count(void)
{
	return WEAPON_ORBIT_BASE_COUNT + weapons[WEAPON_ORBIT] * WEAPON_ORBIT_PER_LEVEL_COUNT;
}
static int orbit_radius(void)
{
	return WEAPON_ORBIT_BASE_RADIUS + weapons[WEAPON_ORBIT] * WEAPON_ORBIT_PER_LEVEL_RADIUS;
}

static void weapon_orbit_init(void)
{
	orbit_angle = 0;
}

static void weapon_orbit_update(int px_fp, int py_fp)
{
	if (weapons[WEAPON_ORBIT] == 0)
		return;

	orbit_angle = (orbit_angle + 2) & 127;
	int n = orbit_count();
	int r = orbit_radius();
	int damage = orbit_damage();

	for (int o = 0; o < n; o++) {
		int a = (orbit_angle + o * 128 / n) & 127;

		int ox_fp = px_fp + r * cosine(a);
		int oy_fp = py_fp - r * sine(a);

		for (int i = 0; i < MAX_ENEMIES; i++) {
			if (enemies[i].type == ENEMY_NONE)
				continue;
			if (enemies[i].orbit_cooldown > 0)
				continue;

			int dx = enemies[i].x - ox_fp;
			int dy = enemies[i].y - oy_fp;
			int64_t distance_squared = (int64_t) dx * dx + (int64_t) dy * dy;
			if (distance_squared < (int64_t) TO_FP(7) * TO_FP(7)) {
				sfx_orbit_hit();
				hurt_enemy(&enemies[i], damage);
				enemies[i].orbit_cooldown = 10;
				int distance = fp_distance_from_squared(distance_squared);
				int knockback = TO_FP(3);
				enemies[i].vx = (short) (dx * knockback / distance);
				enemies[i].vy = (short) (dy * knockback / distance);
			}
		}
	}
}

static void weapon_orbit_draw(int px_scr, int py_scr)
{
	if (weapons[WEAPON_ORBIT] == 0)
		return;

	int n = orbit_count();
	int r = orbit_radius();
	FbColor(PC(9));
	for (int o = 0; o < n; o++) {
		int a = (orbit_angle + o * 128 / n) & 127;
		int ox = px_scr + r * cosine(a) / 256;
		int oy = py_scr + r * (-sine(a)) / 256;
		for (int dy = -1; dy <= 1; dy++)
			for (int dx = -1; dx <= 1; dx++)
				if (ox + dx >= 0 && ox + dx < LCD_XSIZE &&
				    oy + dy >= 0 && oy + dy < LCD_YSIZE)
					FbPoint(ox + dx, oy + dy);
	}
}

static signed char bolt_cooldown;

static int bolt_damage(void)
{
	return WEAPON_BOLT_BASE_DAMAGE + weapons[WEAPON_BOLT] * WEAPON_BOLT_PER_LEVEL_DAMAGE;
}
static int bolt_rate(void)
{
	int r = WEAPON_BOLT_BASE_DELAY - weapons[WEAPON_BOLT] * WEAPON_BOLT_PER_LEVEL_RATE;
	return r < WEAPON_BOLT_MIN_DELAY ? WEAPON_BOLT_MIN_DELAY : r;
}

static int find_nearest_enemy_skip(int ox, int oy, int max_distance_squared,
                                   const unsigned char *skip)
{
	int best_distance = 0x7FFFFFFF, best_i = -1;
	for (int i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i].type == ENEMY_NONE)
			continue;
		if (skip && skip[i])
			continue;
		int dx = TO_INT(enemies[i].x - ox);
		int dy = TO_INT(enemies[i].y - oy);
		int d = dx * dx + dy * dy;
		if (d < best_distance && d <= max_distance_squared) {
			best_distance = d;
			best_i = i;
		}
	}
	return best_i;
}

static int find_nearest_enemy(int ox, int oy, int max_distance_squared)
{
	return find_nearest_enemy_skip(ox, oy, max_distance_squared, NULL);
}

static int fire_bolt(void)
{
	int best_i = find_nearest_enemy(player.x, player.y, RANGED_ACTIVATE_DISTANCE_SQUARED);
	if (best_i < 0)
		return 0;

	int bi = -1;
	for (int i = 0; i < MAX_BOLTS; i++)
		if (bolts[i].life <= 0) {
			bi = i;
			break;
		}
	if (bi < 0)
		return 0;

	int dx = enemies[best_i].x - player.x;
	int dy = enemies[best_i].y - player.y;
	int distance = fp_distance_from_squared((int64_t) dx * dx + (int64_t) dy * dy);
	int speed = TO_FP(4);
	bolts[bi].x = player.x;
	bolts[bi].y = player.y;
	bolts[bi].vx = (short) ((int64_t) dx * speed / distance);
	bolts[bi].vy = (short) ((int64_t) dy * speed / distance);
	bolts[bi].damage = (signed char) bolt_damage();
	bolts[bi].life = 40;
	sfx_bolt_fire();
	return 1;
}

static void weapon_bolt_init(void)
{
	bolt_cooldown = 30;
	memset(bolts, 0, sizeof(bolts));
}

static void bolt_fire_if_ready(void)
{
	bolt_cooldown--;
	if (bolt_cooldown > 0)
		return;
	bolt_cooldown = (signed char) (fire_bolt() ? bolt_rate() : 3);
}

static void bolt_advance_and_collide(void)
{
	for (int i = 0; i < MAX_BOLTS; i++) {
		struct bolt *b = &bolts[i];
		if (b->life <= 0)
			continue;
		b->x += b->vx;
		b->y += b->vy;
		b->life--;
		if (b->life <= 0)
			continue;
		for (int j = 0; j < MAX_ENEMIES; j++) {
			if (enemies[j].type == ENEMY_NONE)
				continue;
			if (overlap_int(TO_INT(b->x), TO_INT(b->y),
			                TO_INT(enemies[j].x),
			                TO_INT(enemies[j].y), 6)) {
				sfx_bolt_hit();
				hurt_enemy(&enemies[j], b->damage);
				b->life = 0;
				break;
			}
		}
	}
}

static void weapon_bolt_update(int px_fp, int py_fp)
{
	(void) px_fp;
	(void) py_fp;
	if (weapons[WEAPON_BOLT] == 0)
		return;

	bolt_fire_if_ready();
	bolt_advance_and_collide();
}

static void weapon_bolt_draw(int px_scr, int py_scr)
{
	(void) px_scr;
	(void) py_scr;
	FbColor(PC(10));
	for (int i = 0; i < MAX_BOLTS; i++) {
		if (bolts[i].life <= 0)
			continue;
		int bx = TO_INT(bolts[i].x) - camera_x;
		int by = TO_INT(bolts[i].y) - camera_y;
		if (bx >= 0 && bx < LCD_XSIZE && by >= 0 && by < LCD_YSIZE) {
			FbPoint(bx, by);
			if (bx + 1 < LCD_XSIZE)
				FbPoint(bx + 1, by);
			if (by + 1 < LCD_YSIZE)
				FbPoint(bx, by + 1);
		}
	}
}

#define AURA_GROW_FRAMES 20
#define AURA_PAUSE_FRAMES 30

static unsigned char aura_frame;
static unsigned char aura_damaged;
static signed char current_aura_radius;

static int aura_damage(void)
{
	return weapons[WEAPON_AURA];
}
static int aura_radius(void)
{
	return WEAPON_AURA_BASE_RADIUS + weapons[WEAPON_AURA] * WEAPON_AURA_PER_LEVEL_RADIUS;
}

static void weapon_aura_init(void)
{
	aura_frame = 0;
	aura_damaged = 0;
	current_aura_radius = 0;
}

static void weapon_aura_update(int px_fp, int py_fp)
{
	if (weapons[WEAPON_AURA] == 0)
		return;

	int cycle = AURA_PAUSE_FRAMES + AURA_GROW_FRAMES;

	if (aura_frame < AURA_PAUSE_FRAMES) {
		current_aura_radius = 0;
	} else {
		int gf = aura_frame - AURA_PAUSE_FRAMES;
		current_aura_radius =
		    (signed char) (aura_radius() * gf / AURA_GROW_FRAMES);
	}

	if (aura_frame == cycle - 1 && !aura_damaged) {
		int r = aura_radius();
		int damage = aura_damage();
		sfx_aura_pulse();
		for (int i = 0; i < MAX_ENEMIES; i++) {
			if (enemies[i].type == ENEMY_NONE)
				continue;
			if (overlap_fp(enemies[i].x, enemies[i].y, px_fp, py_fp, r)) {
				sfx_aura_hit();
				hurt_enemy(&enemies[i], damage);
			}
		}
		aura_damaged = 1;
	}

	aura_frame++;
	if (aura_frame >= cycle) {
		aura_frame = 0;
		aura_damaged = 0;
	}
}

static void weapon_aura_draw(int px_scr, int py_scr)
{
	if (weapons[WEAPON_AURA] == 0 || current_aura_radius <= 0)
		return;
	unsigned short col = ((elapsed_frames / 5) & 1) ? PC(3) : PC(2);
	FbColor(col);
	FbDDACircle(px_scr, py_scr, current_aura_radius);
}

static struct {
	short x[MAX_CHAIN_POINTS], y[MAX_CHAIN_POINTS];
	unsigned char count;
	unsigned char flash;
} chain_visual;

static signed char chain_cooldown;

static int chain_damage(void)
{
	return 1 + weapons[WEAPON_CHAIN];
}
static int chain_count(void)
{
	return 1 + weapons[WEAPON_CHAIN];
}
static int chain_rate(void)
{
	int d = WEAPON_CHAIN_BASE_DELAY - weapons[WEAPON_CHAIN] * WEAPON_CHAIN_PER_LEVEL_RATE;
	return d < WEAPON_CHAIN_MIN_DELAY ? WEAPON_CHAIN_MIN_DELAY : d;
}
static int chain_range_squared(void)
{
	int r = 60 + weapons[WEAPON_CHAIN] * 8;
	return r * r;
}
static int chain_hop_range_squared(void)
{
	int r = 40 + weapons[WEAPON_CHAIN] * 6;
	return r * r;
}

static void weapon_chain_init(void)
{
	chain_cooldown = 40;
	chain_visual.count = 0;
	chain_visual.flash = 0;
}

static void weapon_chain_update(int px_fp, int py_fp)
{
	if (chain_visual.flash > 0)
		chain_visual.flash--;

	if (weapons[WEAPON_CHAIN] == 0)
		return;

	if (--chain_cooldown > 0)
		return;

	int first = find_nearest_enemy(px_fp, py_fp, chain_range_squared());
	if (first < 0) {
		chain_cooldown = 3;
		return;
	}
	chain_cooldown = (signed char) chain_rate();
	sfx_chain_cast();

	const int damage = chain_damage();
	const int jumps = chain_count();
	const int hop_range_squared = chain_hop_range_squared();
	const int ppx = TO_INT(px_fp), ppy = TO_INT(py_fp);

	unsigned char hit[MAX_ENEMIES];
	memset(hit, 0, sizeof(hit));

	chain_visual.count = 0;
	chain_visual.x[chain_visual.count] = (short) ppx;
	chain_visual.y[chain_visual.count] = (short) ppy;
	chain_visual.count++;

	int current = first;
	for (int j = 0; j < jumps && chain_visual.count < MAX_CHAIN_POINTS; j++) {
		sfx_chain_hit();
		hurt_enemy(&enemies[current], damage);
		hit[current] = 1;
		chain_visual.x[chain_visual.count] = (short) TO_INT(enemies[current].x);
		chain_visual.y[chain_visual.count] = (short) TO_INT(enemies[current].y);
		chain_visual.count++;

		int next = find_nearest_enemy_skip(enemies[current].x, enemies[current].y,
		                                   hop_range_squared, hit);
		if (next < 0)
			break;

		int nx = TO_INT(enemies[next].x);
		int ny = TO_INT(enemies[next].y);
		int sx = nx - camera_x, sy = ny - camera_y;
		if (sx < 0 || sx >= LCD_XSIZE || sy < 0 || sy >= LCD_YSIZE)
			break;
		int dxp = nx - ppx, dyp = ny - ppy;
		if (dxp * dxp + dyp * dyp > 120 * 120)
			break;

		current = next;
	}

	chain_visual.flash = 6;
}

/* DDA line walker with per-step jitter for a jagged electric look. */
static void draw_jagged_line(int x0, int y0, int x1, int y1)
{
	int ddx = x1 - x0, ddy = y1 - y0;
	int adx = ddx < 0 ? -ddx : ddx;
	int ady = ddy < 0 ? -ddy : ddy;
	int steps = adx > ady ? adx : ady;
	if (steps == 0)
		steps = 1;

	for (int s = 0; s <= steps; s++) {
		int lx = x0 + ddx * s / steps;
		int ly = y0 + ddy * s / steps;
		if (steps > 2) {
			int jitter = ((s * 7 + elapsed_frames) % 5) - 2;
			if (ddy != 0) lx += jitter;
			if (ddx != 0) ly += jitter;
		}
		if (lx >= 0 && lx < LCD_XSIZE && ly >= 0 && ly < LCD_YSIZE)
			FbPoint(lx, ly);
	}
}

static void weapon_chain_draw(int px_scr, int py_scr)
{
	(void) px_scr;
	(void) py_scr;

	if (chain_visual.flash == 0 || chain_visual.count < 2)
		return;

	FbColor((chain_visual.flash & 1) ? PC(10) : PC(7));
	for (int i = 0; i < chain_visual.count - 1; i++) {
		draw_jagged_line(chain_visual.x[i] - camera_x,
		                 chain_visual.y[i] - camera_y,
		                 chain_visual.x[i + 1] - camera_x,
		                 chain_visual.y[i + 1] - camera_y);
	}
}

static void weapons_init_all(void)
{
	memset(weapons, 0, sizeof(weapons));
	for (int i = 0; i < NUM_WEAPONS; i++)
		weapon_definitions[i].init();
}

static void weapons_update_all(int px_fp, int py_fp)
{
	for (int i = 0; i < NUM_WEAPONS; i++)
		weapon_definitions[i].update(px_fp, py_fp);
}

static void weapons_draw_all(int px_scr, int py_scr)
{
	for (int i = 0; i < NUM_WEAPONS; i++)
		weapon_definitions[i].draw(px_scr, py_scr);
}

#define UPGRADE_SPEED NUM_WEAPONS
#define NUM_UPGRADES (NUM_WEAPONS + 1)

static const struct {
	const char *name;
	const char *desc;
	const char *short_name;
} upgrade_definitions[NUM_UPGRADES] = {
	[WEAPON_ORBIT] = { "ORBIT", "Spinning orbs", "or" },
	[WEAPON_BOLT] = { "BOLT", "Auto-fire bolts", "bl" },
	[WEAPON_AURA] = { "AURA", "Damage aura", "au" },
	[WEAPON_CHAIN] = { "CHAIN", "Chain lightning", "ch" },
	[UPGRADE_SPEED] = { "SPEED", "Move faster", "sp" },
};

static int upgrade_level(int id)
{
	return (id < NUM_WEAPONS) ? weapons[id] : speed_level;
}

static void apply_upgrade(int id)
{
	if (id < NUM_WEAPONS) {
		if (weapons[id] < WEAPON_LEVEL_CAP)
			weapons[id]++;
	} else if (id == UPGRADE_SPEED) {
		if (speed_level < WEAPON_LEVEL_CAP)
			speed_level++;
	}
}

static struct {
	unsigned char upgrade_a, upgrade_b;
	unsigned char active;
} level_up;

static unsigned char choose_level_up_weapon(int exclude[NUM_UPGRADES])
{
	int count = 0;
	for (int i = 0; i < NUM_UPGRADES; ++i)
		if (!exclude[i]) ++count;
	if (count == 0) return 0;
	int pick = rng_range(0, count);
	for (int i = 0; i < NUM_UPGRADES; ++i) {
		if (!exclude[i]) {
			if (pick == 0) return (unsigned char) i;
			--pick;
		}
	}
	return 0;
}

static void get_max_level_weapons(int exclude[NUM_UPGRADES], int *available)
{
	*available = 0;
	for (int i = 0; i < NUM_UPGRADES; ++i) {
		if ((i < NUM_WEAPONS && weapons[i] >= WEAPON_LEVEL_CAP) ||
		    (i == UPGRADE_SPEED && speed_level >= WEAPON_LEVEL_CAP))
			exclude[i] = 1;
		else
			*available = 1;
	}
}

static int count_available_upgrades(int exclude[NUM_UPGRADES])
{
	int count = 0;
	for (int i = 0; i < NUM_UPGRADES; ++i)
		if (!exclude[i])
			count++;
	return count;
}

static void start_level_up(void)
{
	int exclude[NUM_UPGRADES] = {0};
	int available = 0;
	get_max_level_weapons(exclude, &available);
	if (!available) {
		level_up.active = 0;
		return;
	}
	level_up.active = 1;
	level_up.upgrade_a = choose_level_up_weapon(exclude);
	exclude[level_up.upgrade_a] = 1;
	if (count_available_upgrades(exclude) > 0)
		level_up.upgrade_b = choose_level_up_weapon(exclude);
	else
		level_up.upgrade_b = level_up.upgrade_a;
}

static void apply_level_up(int choice)
{
	int id = (choice == 0) ? level_up.upgrade_a : level_up.upgrade_b;
	apply_upgrade(id);
	player.max_health += 2;
	player.health += 2;
	if (player.health > player.max_health)
		player.health = player.max_health;
	level_up.active = 0;
	sfx_upgrade_pick();
}

static void draw_upgrade_choice(char letter, int upgrade_id, int y,
                                uint16_t accent, uint16_t fill)
{
	char buf[24];
	snprintf(buf, sizeof(buf), "[%c] %s Lv%d",
	         letter, upgrade_definitions[upgrade_id].name, upgrade_level(upgrade_id) + 1);
	struct ui_button btn = {
		.x = 14,
		.y = y,
		.width = LCD_XSIZE - 28,
		.height = 22,
		.text = buf,
		.outline_size = 1,
		.outline_color = accent,
		.fill_color = fill,
		.text_color = accent,
	};
	ui_button_draw(btn);
	FbColor(PC(6));
	FbMove(ui_center_text_x(upgrade_definitions[upgrade_id].desc, btn.x, btn.width),
	       y + 13);
	write_string(upgrade_definitions[upgrade_id].desc);
}

static void draw_level_up(void)
{
	struct ui_text_box dialog = {
		.x = 10,
		.y = 24,
		.width = LCD_XSIZE - 20,
		.height = 80,
		.outline_size = 1,
		.outline_color = PC(10),
		.fill_color = PC(0),
		.text_color = PC(10),
		.text = "LEVEL UP!",
	};
	ui_text_box_draw(dialog);

	draw_upgrade_choice('A', level_up.upgrade_a, 42, PC(12), PC(2));
	if (level_up.upgrade_b != level_up.upgrade_a)
		draw_upgrade_choice('B', level_up.upgrade_b, 72, PC(14), PC(1));
}

static uint32_t next_experience_threshold(uint32_t current)
{
	uint32_t next = current * 3 / 2 + 5;
	return next < current ? UINT32_MAX : next;
}

static void check_level_up(void)
{
	if (player.experience < player.experience_next)
		return;
	player.experience -= player.experience_next;
	player.level++;
	player.experience_next = next_experience_threshold(player.experience_next);
	start_level_up();
	sfx_level_up();
}

static int player_speed(void)
{
	return TO_FP(2) + speed_level * (FP_ONE / 4);
}

static void update_player_movement(int *vx, int *vy)
{
	int speed = player_speed();
	*vx = 0;
	*vy = 0;
	if (button_poll(BADGE_BUTTON_LEFT))
		*vx -= speed;
	if (button_poll(BADGE_BUTTON_RIGHT))
		*vx += speed;
	if (button_poll(BADGE_BUTTON_UP))
		*vy -= speed;
	if (button_poll(BADGE_BUTTON_DOWN))
		*vy += speed;
}

static void update_player_direction(int vx, int vy)
{
	player.moving = (vx || vy) ? 1 : 0;
	if (vx < 0)
		player.direction = DIR_LEFT;
	else if (vx > 0)
		player.direction = DIR_RIGHT;
	else if (vy < 0)
		player.direction = DIR_UP;
	else if (vy > 0)
		player.direction = DIR_DOWN;
}

static void update_player_animation(void)
{
	if (player.moving) {
		player.animation_timer--;
		if (player.animation_timer <= 0) {
			player.animation_frame ^= 1;
			player.animation_timer = 6;
		}
	} else {
		player.animation_frame = 0;
		player.animation_timer = 0;
	}
}

static void update_player_position(int vx, int vy)
{
	if (vx && vy) {
		vx = vx * 181 / 256;
		vy = vy * 181 / 256;
	}

	player.x += vx;
	player.y += vy;
	player.x = clamp(player.x, TO_FP(4), TO_FP(WORLD_WIDTH - 12));
	player.y = clamp(player.y, TO_FP(4), TO_FP(WORLD_HEIGHT - 12));
}

static void update_player(void)
{
	int vx, vy;
	update_player_movement(&vx, &vy);
	update_player_direction(vx, vy);
	update_player_animation();
	update_player_position(vx, vy);
}

static void update_camera(void)
{
	int tx = TO_INT(player.x) + 4 - LCD_XSIZE / 2;
	int ty = TO_INT(player.y) + 4 - LCD_YSIZE / 2;
	camera_x += (tx - camera_x) / 3;
	camera_y += (ty - camera_y) / 3;
	camera_x = clamp(camera_x, 0, WORLD_WIDTH - LCD_XSIZE);
	camera_y = clamp(camera_y, 0, WORLD_HEIGHT - LCD_YSIZE);
}

static void draw_ground(void)
{
	FbColor(PC(2));
	for (int y = 0; y < LCD_YSIZE; y++) {
		int val = ((camera_y + y) * 13 + camera_x * 7) & 0x7F;
		for (int x = 0; x < LCD_XSIZE; x++) {
			if (val == 0)
				FbPoint(x, y);
			val = (val + 7) & 0x7F;
		}
	}
}

static void draw_player_damage_flash(int sx, int sy)
{
	if (player.damage_flash <= 0)
		return;
	if (!(player.damage_flash & 1))
		return;

	FbColor(PC(8));
	for (int dy = 1; dy < 7; dy++)
		for (int dx = 1; dx < 7; dx++)
			FbPoint(sx + dx, sy + dy);
}

static void draw_player_sprite(void)
{
	int sx = TO_INT(player.x) - camera_x;
	int sy = TO_INT(player.y) - camera_y;
	draw_player_damage_flash(sx, sy);
	draw_sprite(sx, sy, player_frames[player.direction][player.animation_frame]);
}

static void draw_title(void)
{
	FbClear();

	struct ui_button title = {
		.x = 8,
		.y = 10,
		.width = LCD_XSIZE - 16,
		.height = 20,
		.outline_size = 1,
		.outline_color = PC(8),
		.fill_color = PC(0),
		.text_color = PC(8),
		.text = "DAYWALKER",
	};
	ui_button_draw(title);

	FbColor(PC(6));
	FbMove(ui_center_text_x("Move to survive.", 0, LCD_XSIZE), 38);
	write_string("Move to survive.");
	FbMove(ui_center_text_x("Weapons auto-fire.", 0, LCD_XSIZE), 50);
	write_string("Weapons auto-fire.");
	FbMove(ui_center_text_x("Collect XP to grow.", 0, LCD_XSIZE), 62);
	write_string("Collect XP to grow.");

	int num_sprites = 3;
	int sprite_width = 8;
	int spacing = 4;
	int total_w = num_sprites * sprite_width + (num_sprites - 1) * spacing;
	int start_x = (LCD_XSIZE - total_w) / 2;

	draw_sprite(start_x + 0 * (sprite_width + spacing), 80, sprite_bat0);
	draw_sprite(start_x + 1 * (sprite_width + spacing), 80, sprite_slime0);
	draw_sprite(start_x + 2 * (sprite_width + spacing), 80, sprite_skeleton0);

	struct ui_button start_btn = {
		.x = 30,
		.y = 100,
		.width = LCD_XSIZE - 60,
		.height = 18,
		.text = "[A] START",
		.outline_size = 1,
		.outline_color = PC(10),
		.fill_color = PC(2),
		.text_color = PC(10),
	};
	ui_button_draw(start_btn);
}

static unsigned long long last_frame;
#define FRAME_MS 33

static void init_rng(void)
{
	rng_state = (unsigned int) rtc_get_ms_since_boot();
	if (rng_state == 0)
		rng_state = 1;
}

static void init_player(void)
{
	player.x = TO_FP(WORLD_WIDTH / 2);
	player.y = TO_FP(WORLD_HEIGHT / 2);
	player.health = 10;
	player.max_health = 10;
	player.experience = 0;
	player.experience_next = 10;
	player.level = 1;
	player.kills = 0;
	player.damage_flash = 0;
	player.direction = DIR_DOWN;
	player.animation_frame = 0;
	player.animation_timer = 0;
	player.moving = 0;
}

static void clear_world(void)
{
	memset(enemies, 0, sizeof(enemies));
	memset(gems, 0, sizeof(gems));
	memset(damage_numbers, 0, sizeof(damage_numbers));
}

static void init_camera(void)
{
	camera_x = TO_INT(player.x) - LCD_XSIZE / 2;
	camera_y = TO_INT(player.y) - LCD_YSIZE / 2;
}

static void reset_run_state(void)
{
	weapons_init_all();
	weapons[WEAPON_ORBIT] = 1;
	speed_level = 0;
	level_up.active = 0;
	spawn_timer = 30;
	elapsed_frames = 0;
}

static void init_game(void)
{
	init_rng();
	init_player();
	clear_world();
	reset_run_state();
	init_camera();
	last_frame = rtc_get_ms_since_boot();
}

static void draw_play_frame(bool show_level_up)
{
	FbClear();
	draw_ground();
	draw_gems();
	draw_enemies();
	draw_player_sprite();

	int px_scr = TO_INT(player.x) - camera_x + 4;
	int py_scr = TO_INT(player.y) - camera_y + 4;
	weapons_draw_all(px_scr, py_scr);

	draw_damage_numbers();
	if (show_level_up)
		draw_level_up();
	FbSwapBuffers();
}

static enum {
	DAYWALKER_INIT = 0,
	DAYWALKER_TITLE,
	DAYWALKER_PLAY,
	DAYWALKER_EXIT,
} daywalker_state;

static void tick_play(int down_latches)
{
	unsigned long long now = rtc_get_ms_since_boot();
	if (now - last_frame < FRAME_MS)
		return;
	last_frame = now;

	if (level_up.active) {
		if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches))
			apply_level_up(0);
		else if (level_up.upgrade_b != level_up.upgrade_a &&
		         BUTTON_PRESSED(BADGE_BUTTON_B, down_latches))
			apply_level_up(1);
		draw_play_frame(true);
		return;
	}

	if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
		daywalker_state = DAYWALKER_EXIT;
		return;
	}

	elapsed_frames++;
	update_player();
	update_enemies();
	weapons_update_all(player.x, player.y);
	update_gems();
	update_damage_numbers();
	update_spawns();
	update_camera();

	if (player.damage_flash > 0)
		player.damage_flash--;

	check_level_up();

	draw_play_frame(false);
}

void daywalker_cb(struct badge_app *app)
{
	if (app->wake_up)
		app->wake_up = 0;

	int down_latches = button_down_latches();

	switch (daywalker_state) {
	case DAYWALKER_INIT:
		FbInit();
		daywalker_state = DAYWALKER_TITLE;
		break;

	case DAYWALKER_TITLE:
		if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
			sfx_menu_select();
			init_game();
			daywalker_state = DAYWALKER_PLAY;
			break;
		}
		if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
			daywalker_state = DAYWALKER_EXIT;
			break;
		}
		draw_title();
		FbSwapBuffers();
		break;

	case DAYWALKER_PLAY:
		tick_play(down_latches);
		break;

	case DAYWALKER_EXIT:
		daywalker_state = DAYWALKER_INIT;
		pop_app();
		break;
	}
}
