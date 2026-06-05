/*********************************************

BATPING - badge port

You are a bat in a dark cave. Tap to flap and emit a sonar pulse; the cave is
only visible where recent echoes have revealed it.

Controls:
   A / UP : flap + sonar pulse
   B      : return to badge menu

**********************************************/

#include <stdbool.h>
#include <stdio.h>

#include "badge.h"
#include "button.h"
#include "colors.h"
#include "framebuffer.h"
#include "rtc.h"
#include "xorshift.h"
#include "audio.h"
#include "ui.h"
#include "particle.h"
#include "key_value_storage.h"

#define BATPING_BEST_KEY "BATPING_BEST"
#define BATPING_POOL_SIG 0x6A791234

#define BAT_X 36
#define REVEAL_W (LCD_XSIZE / 4)
#define REVEAL_H (LCD_YSIZE / 4)
#define MAX_WALLS 96
#define MAX_MOTHS 28
#define MAX_RINGS 6

#define GRAVITY 15
#define FLAP -280
#define MAX_FALL 400

struct wall {
	int x, top, bot;
};

struct moth {
	int x, y;
	unsigned char phase;
	bool collected;
};

struct ring {
	int x, y, r, max_r;
};

enum batping_state {
	BATPING_INIT,
	BATPING_PLAY,
	BATPING_EXIT,
};

static enum batping_state batping_state = BATPING_INIT;
static struct wall walls[MAX_WALLS];
static struct moth moths[MAX_MOTHS];
static struct ring rings[MAX_RINGS];
static struct particle_pool *particlepool = NULL;
static unsigned char reveal_map[REVEAL_W * REVEAL_H];
static int wall_count, moth_count, ring_count;
static int bat_y, bat_vel, score, best, tick, scroll_accum;
static int saved_best;		/* last best value written to flash */
static int world_scroll;	/* total pixels the world has scrolled, for parallax */
static int shake, shake_timer, flash_timer;
static bool dead, waiting_to_start;
static unsigned int rng_state;
static int screen_changed;

static int rnd(int n)
{
	if (n <= 0)
		return 0;
	return (int)(xorshift(&rng_state) % (unsigned int)n);
}

/* Sound effect stubs.  Flip DEBUG_BEEP_ENABLED to 1 to hear placeholder beeps;
 * real sounds can be dropped into these later (same pattern as daywalker). */
#define DEBUG_BEEP_ENABLED 0
static void sfx_debug_beep(uint16_t freq, uint16_t duration)
{
#if DEBUG_BEEP_ENABLED
	audio_out_beep(freq, duration);
#else
	(void)freq;
	(void)duration;
#endif
}

static void sfx_pulse(void) { sfx_debug_beep(1400, 18); }
static void sfx_point(void) { sfx_debug_beep(1700, 30); }
static void sfx_moth(void)  { sfx_debug_beep(2100, 45); }
static void sfx_die(void)   { sfx_debug_beep(150, 320); }

static void rect(int x, int y, int w, int h, unsigned short c)
{
	if (w <= 0 || h <= 0)
		return;
	FbPlaceFilledRectangle(x, y, w, h, c);
}

static void point(int x, int y, unsigned short c)
{
	if (x < 0 || x >= LCD_XSIZE || y < 0 || y >= LCD_YSIZE)
		return;

	FbColor(c);
	FbPoint(x, y);
}

/* draw a line of text horizontally centered within [box_x, box_x+box_w) */
static void centered_line(int box_x, int box_w, int y, const char *s, unsigned short color)
{
	FbColor(color);
	FbMove(ui_center_text_x(s, box_x, box_w), y);
	FbWriteString(s);
}

static void text_at(int x, int y, const char *s, unsigned short c)
{
	FbColor(c);
	FbMove(x, y);
	FbWriteString(s);
}

static void burst(int x, int y, int n, unsigned short c)
{
	int i;

	/* shared-pool coords are 24.8 fixed point; spread is ~2.5 px/frame each way */
	for (i = 0; i < n; i++)
		particlepool->config.add_particle(particlepool, x << 8, y << 8,
			rnd(1280) - 640, rnd(1280) - 640, 10 + rnd(14), c);
}

static void reveal_around(int cx, int cy, int radius)
{
	int gx, gy;
	int r2 = radius * radius;

	for (gy = 0; gy < REVEAL_H; gy++) {
		for (gx = 0; gx < REVEAL_W; gx++) {
			int x = gx * 4 + 2;
			int y = gy * 4 + 2;
			int dx = x - cx;
			int dy = y - cy;

			if (dx * dx + dy * dy < r2)
				reveal_map[gy * REVEAL_W + gx] = 30;
		}
	}
}

static void add_ring(void)
{
	struct ring *r;

	if (ring_count >= MAX_RINGS)
		return;
	r = &rings[ring_count++];
	r->x = BAT_X;
	r->y = bat_y / 100;
	r->r = 0;
	r->max_r = 64;
}

static void emit_pulse(void)
{
	add_ring();
	reveal_around(BAT_X, bat_y / 100, 18);
	sfx_pulse();
}

static void maybe_add_moth(int x, int top, int bot)
{
	struct moth *m;
	int gap = bot - top;

	if (moth_count >= MAX_MOTHS || gap < 18 || rnd(100) >= 8)
		return;
	m = &moths[moth_count++];
	m->x = x + rnd(8);
	m->y = top + 6 + rnd(gap - 12);
	m->phase = (unsigned char)rnd(32);
	m->collected = false;
}

static void append_wall(int x, int top, int bot)
{
	if (wall_count >= MAX_WALLS)
		return;
	walls[wall_count].x = x;
	walls[wall_count].top = top;
	walls[wall_count].bot = bot;
	wall_count++;
	maybe_add_moth(x, top, bot);
}

static void enforce_min_gap(int *top, int *bot, int gap)
{
	if (*bot - *top < gap) {
		int mid = (*top + *bot) / 2;
		*top = mid - gap / 2;
		*bot = mid + gap / 2;
	}
}

static void seed_walls(void)
{
	int i;
	int top = 10, bot = LCD_YSIZE - 10;

	wall_count = 0;
	moth_count = 0;
	for (i = 0; i < MAX_WALLS; i++) {
		int gap = 54;

		top += rnd(9) - 4;
		bot += rnd(9) - 4;
		if (top < 6) top = 6;
		if (top > 52) top = 52;
		if (bot < 78) bot = 78;
		if (bot > LCD_YSIZE - 5) bot = LCD_YSIZE - 5;
		enforce_min_gap(&top, &bot, gap);
		append_wall(LCD_XSIZE + i * 8, top, bot);
	}
}

static void clear_reveal_map(void)
{
	int i;

	for (i = 0; i < REVEAL_W * REVEAL_H; i++)
		reveal_map[i] = 0;
}

static void game_init(void)
{
	bat_y = 64 * 100;
	bat_vel = 0;
	score = 0;
	dead = false;
	waiting_to_start = true;
	scroll_accum = 0;
	world_scroll = 0;
	ring_count = 0;
	particlepool->nparticles = 0;
	shake = 0;
	shake_timer = 0;
	flash_timer = 0;
	tick = 0;
	clear_reveal_map();
	seed_walls();
	reveal_around(BAT_X, bat_y / 100, 44);
	screen_changed = 1;
}

/* Persist the best score to flash, but only when it actually changed -- flash
 * wears out if written every frame, so this is called at game-end boundaries. */
static void persist_best_score(void)
{
	if (best != saved_best) {
		flash_kv_store_int(BATPING_BEST_KEY, best);
		saved_best = best;
	}
}

static void die(void)
{
	if (dead)
		return;
	dead = true;
	sfx_die();
	if (score > best)
		best = score;
	persist_best_score();
	shake_timer = 14;
	flash_timer = 6;
	reveal_around(BAT_X, bat_y / 100, 100);
	burst(BAT_X, bat_y / 100, 12, CYAN);
}

static void update_particles(void)
{
	particlepool->config.move_particles(particlepool);
}

static void update_rings(void)
{
	int i, w = 0;

	for (i = 0; i < ring_count; i++) {
		rings[i].r += 3;
		reveal_around(rings[i].x, rings[i].y, rings[i].r);
		if (rings[i].r <= rings[i].max_r)
			rings[w++] = rings[i];
	}
	ring_count = w;
}

static void decay_reveal(void)
{
	int i;

	for (i = 0; i < REVEAL_W * REVEAL_H; i++)
		if (reveal_map[i] > 0)
			reveal_map[i]--;
}

static void refill_walls(void)
{
	while (wall_count < MAX_WALLS) {
		int top = wall_count ? walls[wall_count - 1].top : 14;
		int bot = wall_count ? walls[wall_count - 1].bot : LCD_YSIZE - 14;
		int x = wall_count ? walls[wall_count - 1].x + 8 : LCD_XSIZE + 80;
		int gap = 50 - score / 8;

		if (gap < 34)
			gap = 34;
		top += rnd(9) - 4;
		bot += rnd(9) - 4;
		if (top < 6) top = 6;
		if (top > 54) top = 54;
		if (bot < 76) bot = 76;
		if (bot > LCD_YSIZE - 5) bot = LCD_YSIZE - 5;
		enforce_min_gap(&top, &bot, gap);
		append_wall(x, top, bot);
	}
}

static void scroll_walls(int spd)
{
	int i, w = 0;

	for (i = 0; i < wall_count; i++)
		walls[i].x -= spd;
	for (i = 0; i < wall_count; i++) {
		if (walls[i].x >= -20) {
			walls[w++] = walls[i];
		} else {
			score++;
			sfx_point();
		}
	}
	wall_count = w;
}

static void scroll_moths(int spd)
{
	int i, w = 0;

	for (i = 0; i < moth_count; i++) {
		moths[i].x -= spd;
		if (moths[i].x >= -20)
			moths[w++] = moths[i];
	}
	moth_count = w;
}

static void scroll_world(int spd)
{
	world_scroll += spd;
	scroll_walls(spd);
	scroll_moths(spd);
	refill_walls();
}

static void update_moths(void)
{
	int i;
	int by = bat_y / 100;

	for (i = 0; i < moth_count; i++) {
		struct moth *m = &moths[i];
		int dx, dy;

		if (m->collected)
			continue;
		m->y += ((tick + m->phase) & 8) ? 1 : -1;
		dx = BAT_X - m->x;
		dy = by - m->y;
		if (dx * dx + dy * dy < 80) {
			m->collected = true;
			score += 5;
			sfx_moth();
			burst(m->x, m->y, 6, x11_gold);
		}
	}
}

static void check_collisions(void)
{
	int i;
	int by = bat_y / 100;

	for (i = 0; i < wall_count; i++) {
		struct wall *w = &walls[i];

		if (BAT_X + 4 > w->x && BAT_X - 4 < w->x + 8) {
			if (by - 3 < w->top || by + 3 > w->bot) {
				die();
				return;
			}
		}
	}
	if (by < 3 || by > LCD_YSIZE - 3)
		die();
}

static void update_screen_effects(void)
{
	update_particles();
	update_rings();
	decay_reveal();
	if (shake_timer > 0) {
		shake_timer--;
		shake = shake_timer ? rnd(3) - 1 : 0;
	}
	if (flash_timer > 0)
		flash_timer--;
}

static void update_waiting(bool pressed)
{
	bat_y = (64 + (((tick / 4) & 1) ? 3 : -3)) * 100;
	reveal_around(BAT_X, bat_y / 100, 42);
	if (pressed) {
		waiting_to_start = false;
		emit_pulse();
	}
}

static void apply_gravity(bool pressed)
{
	if (pressed) {
		bat_vel = FLAP;
		emit_pulse();
	}
	bat_vel += GRAVITY;
	if (bat_vel > MAX_FALL)
		bat_vel = MAX_FALL;
	bat_y += bat_vel;
}

static void advance_world(void)
{
	scroll_accum += 120 + score / 3;
	while (scroll_accum >= 100) {
		scroll_world(1);
		scroll_accum -= 100;
	}
}

static bool exit_requested(int down)
{
	return BUTTON_PRESSED(BADGE_BUTTON_B, down) ||
	       BUTTON_PRESSED(BADGE_BUTTON_REWIND, down);
}

static void update_play(void)
{
	int down = button_down_latches();
	bool pressed = BUTTON_PRESSED(BADGE_BUTTON_A, down) ||
		       BUTTON_PRESSED(BADGE_BUTTON_UP, down);

	if (exit_requested(down)) {
		batping_state = BATPING_EXIT;
		return;
	}
	tick++;
	update_screen_effects();
	if (dead) {
		if (pressed)
			game_init();
		screen_changed = 1;
		return;
	}
	if (waiting_to_start) {
		update_waiting(pressed);
		screen_changed = 1;
		return;
	}
	apply_gravity(pressed);
	advance_world();
	update_moths();
	check_collisions();
	reveal_around(BAT_X, bat_y / 100, 12);
	screen_changed = 1;
}

static int point_revealed(int x, int y)
{
	int gx = x / 4;
	int gy = y / 4;

	if (gx < 0 || gx >= REVEAL_W || gy < 0 || gy >= REVEAL_H)
		return 0;
	return reveal_map[gy * REVEAL_W + gx];
}

static void draw_wall_cell(int wx, int y, int vis, bool edge)
{
	rect(wx, y, 8, 4, vis > 20 ? x11_gray20 : x11_steel_blue);
	if (edge)
		rect(wx, y, 8, 4, x11_gray40);
}

static void draw_wall_column(struct wall *w, int wx, int y)
{
	int vis = point_revealed(w->x + 2, y + 2);

	if (vis <= 0)
		return;
	if (y < w->top)
		draw_wall_cell(wx, y, vis, y >= w->top - 4);
	else if (y >= w->bot)
		draw_wall_cell(wx, y, vis, y < w->bot + 4);
}

static void draw_walls(int ox)
{
	int i, y;

	for (i = 0; i < wall_count; i++) {
		struct wall *w = &walls[i];
		int wx = w->x + ox;

		if (wx > LCD_XSIZE + 4 || wx + 8 < -4)
			continue;
		for (y = 0; y < LCD_YSIZE; y += 4)
			draw_wall_column(w, wx, y);
	}
}

static void draw_ambient(int ox)
{
	int gx, gy;
	/* drift the 16px dot grid at 1/3 the world speed for a parallax sense
	 * of motion; it repeats every 16px so the offset just wraps */
	int par = (world_scroll / 3) % 16;

	for (gy = 0; gy < REVEAL_H; gy++) {
		for (gx = 0; gx < REVEAL_W; gx++) {
			int vis = reveal_map[gy * REVEAL_W + gx];

			if (vis > 20 && (gx & 3) == 0 && (gy & 3) == 0)
				point(gx * 4 + ox - par, gy * 4, x11_steel_blue);
		}
	}
}

static void draw_rings(int ox)
{
	int i;

	for (i = 0; i < ring_count; i++) {
		unsigned short c = rings[i].r < rings[i].max_r / 2 ? CYAN : x11_deep_sky_blue;

		FbColor(c);
		FbDDACircle(rings[i].x + ox, rings[i].y, rings[i].r);
	}
}

static void draw_moths(int ox)
{
	int i;

	for (i = 0; i < moth_count; i++) {
		struct moth *m = &moths[i];
		int wing;

		if (m->collected || point_revealed(m->x, m->y) <= 5)
			continue;
		wing = ((tick + m->phase) & 8) ? -1 : 0;
		point(m->x + ox, m->y, x11_gold);
		point(m->x - 1 + ox, m->y + wing, x11_gold);
		point(m->x + 1 + ox, m->y + wing, x11_gold);
	}
}

static void draw_particles(int ox)
{
	int i;

	/* custom draw so we can apply the parallax offset (see BADGE-APP-HOWTO) */
	for (i = 0; i < particlepool->nparticles; i++)
		point((particlepool->p[i].x >> 8) + ox, particlepool->p[i].y >> 8,
			(unsigned short)particlepool->p[i].color);
}

/* The bat sprite from daywalker: an 8x8 sprite, one palette-index nibble per
 * pixel (index 13 = lavender body, 7 = white eyes), two wing-flap frames. */
static const unsigned short bat_palette[16] = {
	PACKRGB888(0, 0, 0),     PACKRGB888(127, 36, 84),
	PACKRGB888(28, 43, 83),  PACKRGB888(0, 135, 81),
	PACKRGB888(171, 82, 54), PACKRGB888(96, 88, 79),
	PACKRGB888(195, 195, 198), PACKRGB888(255, 241, 233),
	PACKRGB888(237, 27, 81), PACKRGB888(250, 162, 27),
	PACKRGB888(247, 236, 47), PACKRGB888(93, 187, 77),
	PACKRGB888(81, 166, 220), PACKRGB888(131, 118, 156),
	PACKRGB888(241, 118, 166), PACKRGB888(252, 204, 171),
};

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

/* draw an 8x8 nibble sprite with its top-left at (sx, sy) */
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

			if (hi && dx >= 0 && dx < LCD_XSIZE)
				point(dx, dy, bat_palette[hi]);
			if (lo && dx + 1 >= 0 && dx + 1 < LCD_XSIZE)
				point(dx + 1, dy, bat_palette[lo]);
		}
	}
}

static void draw_bat(int ox)
{
	int bx = BAT_X + ox;
	int by = bat_y / 100;

	if (dead)
		return;
	/* center the 8x8 sprite on the bat, flapping wings with tick */
	draw_sprite(bx - 4, by - 4, (tick & 8) ? sprite_bat1 : sprite_bat0);
}

static void draw_hud(void)
{
	char buf[24];

	rect(0, 0, LCD_XSIZE, 10, BLACK);
	snprintf(buf, sizeof(buf), "SCORE:%d", score);
	text_at(2, 1, buf, CYAN);
	snprintf(buf, sizeof(buf), "BEST:%d", best);
	text_at(88, 1, buf, x11_gold);
}

static void draw_start_prompt(void)
{
	struct ui_text_box box = {
		.x = 6, .y = 44, .width = 148, .height = 40,
		.outline_size = 1,
		.outline_color = CYAN,
		.fill_color = BLACK,
		.text_color = WHITE,
		.text = "",
	};
	ui_text_box_fill(box);
	ui_text_box_draw_outline(box);
	centered_line(box.x, box.width, 52, "BATPING!", CYAN);
	centered_line(box.x, box.width, 66, "A/UP TAP TO PULSE", WHITE);
}

static void draw_game_over(void)
{
	char buf[24];
	struct ui_text_box box = {
		.x = 22, .y = 34, .width = 116, .height = 66,
		.outline_size = 1,
		.outline_color = CYAN,
		.fill_color = BLACK,
		.text_color = WHITE,
		.text = "",
	};
	ui_text_box_fill(box);
	ui_text_box_draw_outline(box);
	centered_line(box.x, box.width, 42, "SILENCED!", x11_gold);
	snprintf(buf, sizeof(buf), "SCORE: %d", score);
	centered_line(box.x, box.width, 58, buf, WHITE);
	snprintf(buf, sizeof(buf), "BEST: %d", best);
	centered_line(box.x, box.width, 72, buf, x11_gold);
	centered_line(box.x, box.width, 88, "A/UP RETRY", x11_gray40);
}

static void draw_overlay(void)
{
	draw_hud();
	if (waiting_to_start)
		draw_start_prompt();
	if (dead)
		draw_game_over();
}

static void draw_play(void)
{
	int ox = shake;

	if (!screen_changed)
		return;
	FbClear();
	rect(0, 0, LCD_XSIZE, LCD_YSIZE, BLACK);
	draw_ambient(ox);
	draw_walls(ox);
	draw_rings(ox);
	draw_moths(ox);
	draw_particles(ox);
	draw_bat(ox);
	if (flash_timer > 0 && (flash_timer & 1))
		rect(0, 0, LCD_XSIZE, LCD_YSIZE, CYAN);
	draw_overlay();
	FbSwapBuffers();
	screen_changed = 0;
}

static void batping_init(void)
{
	int stored;

	FbInit();
	FbClear();
	rng_state = (unsigned int)rtc_get_ms_since_boot();
	if (rng_state == 0)
		rng_state = 0xec001234;
	best = 0;
	if (flash_kv_get_int(BATPING_BEST_KEY, &stored) && stored > 0) {
		best = stored;
		saved_best = best;
	}
	game_init();
	batping_state = BATPING_PLAY;
}

static void batping_exit(void)
{
	batping_state = BATPING_INIT;
	pop_app();
}

void batping_cb(struct badge_app *app)
{
	if (app->wake_up) {	/* another app disturbed the screen; force a redraw */
		screen_changed = 1;
		app->wake_up = 0;
	}

	if (particlepool == NULL) {
		particlepool = get_common_particle_pool();
		particlepool->nparticles = 0;
	}
	claim_particle_pool(particlepool, BATPING_POOL_SIG);

	switch (batping_state) {
	case BATPING_INIT:
		batping_init();
		break;
	case BATPING_PLAY:
		update_play();
		draw_play();
		break;
	case BATPING_EXIT:
		batping_exit();
		break;
	default:
		break;
	}
}
