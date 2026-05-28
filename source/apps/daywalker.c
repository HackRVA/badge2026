/*
 * Daywalker - an attempt at a vampire survivors style auto-battler.
 *
 * You move. Weapons fire automatically. Enemies swarm.
 * Kill enemies to drop XP gems. Collect gems to level up.
 * Pick upgrades. Survive as long as you can.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "audio.h"
#include "badge.h"
#include "button.h"
#include "colors.h"
#include "framebuffer.h"
#include "menu.h"
#include "palette.h"
#include "rtc.h"
#include "ui.h"

#include "daywalker.h"

#define WORLD_WIDTH 512
#define WORLD_HEIGHT 512

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
static void sfx_menu_select(void)  { sfx_debug_beep(900,  40); }

static inline int clamp(int v, int lo, int hi)
{
	if (v < lo)
		return lo;
	if (v > hi)
		return hi;
	return v;
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

static struct {
	int x, y;
	unsigned char damage_flash;
	unsigned char direction;
	unsigned char animation_frame;
	signed char animation_timer;
	unsigned char moving;
} player;

static unsigned char speed_level;

static void write_string(const char *string)
{
	int prev_transparent_index = FbGetTransparentIndex();
	FbTransparentIndex(0);
	FbWriteString(string);
	FbTransparentIndex(prev_transparent_index);
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

static void init_player(void)
{
	player.x = TO_FP(WORLD_WIDTH / 2);
	player.y = TO_FP(WORLD_HEIGHT / 2);
	player.damage_flash = 0;
	player.direction = DIR_DOWN;
	player.animation_frame = 0;
	player.animation_timer = 0;
	player.moving = 0;
}

static void init_camera(void)
{
	camera_x = TO_INT(player.x) - LCD_XSIZE / 2;
	camera_y = TO_INT(player.y) - LCD_YSIZE / 2;
}

static void reset_run_state(void)
{
	speed_level = 0;
	elapsed_frames = 0;
}

static void init_game(void)
{
	init_player();
	reset_run_state();
	init_camera();
	last_frame = rtc_get_ms_since_boot();
}

static void draw_play_frame(void)
{
	FbClear();
	draw_ground();
	draw_player_sprite();
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

	if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
		daywalker_state = DAYWALKER_EXIT;
		return;
	}

	elapsed_frames++;
	update_player();
	update_camera();

	if (player.damage_flash > 0)
		player.damage_flash--;

	draw_play_frame();
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
