/**
 * Sky Golf
 * Inspired by crisp-game-lib's skygolf
 *
 * Aim your shot angle, hold to set power, release to shoot.
 * Bounce the ball off platforms and into the flag hole.
 *
 * Ground types:
 *   fairway (green) - normal bounce
 *   sand (yellow) - reduced bounce
 *   water (blue) - sends ball back to previous position
 *   trees (green on red trunk) - obstacles
 *
 * Difficulty selects number of holes: Easy=3, Medium=6, Hard=9
 */

#include <stdbool.h>
#include <stdio.h>

#include "badge.h"
#include "button.h"
#include "colors.h"
#include "framebuffer.h"
#include "rtc.h"
#include "trig.h"
#include "ui.h"
#include "xorshift.h"
#include "particle.h"
#include "audio.h"

#define COLOR_BLACK      PACKRGB888(0, 0, 0)
#define COLOR_SKY_BLUE   PACKRGB888(135, 206, 235)
#define COLOR_SKY_DARK   PACKRGB888(110, 180, 210)
#define COLOR_GREEN      PACKRGB888(0, 135, 81)
#define COLOR_BROWN      PACKRGB888(120, 80, 40)
#define COLOR_DARK_GREY  PACKRGB888(96, 88, 79)
#define COLOR_LIGHT_GREY PACKRGB888(195, 195, 198)
#define COLOR_WHITE      PACKRGB888(255, 255, 255)
#define COLOR_RED        PACKRGB888(237, 27, 81)
#define COLOR_ORANGE     PACKRGB888(255, 150, 45)
#define COLOR_YELLOW     PACKRGB888(250, 200, 50)
#define COLOR_SAND       PACKRGB888(230, 210, 130)
#define COLOR_WATER      PACKRGB888(50, 120, 200)
#define COLOR_LAVENDER   PACKRGB888(131, 118, 156)
#define COLOR_CLOUD      PACKRGB888(240, 240, 240)
#define COLOR_CLOUD_SH   PACKRGB888(220, 225, 230)

#define FRAME_MS 33

#define FP 8
#define FP_ONE (1 << FP)
#define TO_FP(x) ((x) * FP_ONE)
#define TO_INT(x) ((x) >> FP)
#define FP_MUL(a, b) (((a) * (b)) >> FP)

#define MAX_PLATFORMS 4
#define MAX_GROUNDS 25
#define MAX_POWER 12

enum skygolf_state {
	SKYGOLF_INIT = 0,
	SKYGOLF_MENU,
	SKYGOLF_TITLE,
	SKYGOLF_INGAME,
	SKYGOLF_NEXT_HOLE,
	SKYGOLF_GIVE_UP,
	SKYGOLF_HOLE_OUT,
	SKYGOLF_EXIT,
};

enum ground_type {
	GROUND_FAIRWAY = 0,
	GROUND_SAND,
	GROUND_WATER,
	GROUND_TREE,
	GROUND_FLAG,
};

struct ground {
	enum ground_type type;
	int tree_height; /* only for GROUND_TREE */
};

struct platform {
	int x, y; /* pixel position */
	int num_grounds;
	struct ground grounds[MAX_GROUNDS];
};

enum ball_state {
	BALL_SHOT = 0, /* aiming angle */
	BALL_POWER,    /* holding to set power */
	BALL_FLY,      /* in flight */
};

struct ball {
	int x, y;           /* fixed-point position */
	int prev_x, prev_y; /* fixed-point previous safe position */
	int vx, vy;         /* fixed-point velocity */
	int angle;           /* badge angle: 0-127 maps to 0-360 degrees */
	int angle_dir;       /* +1 or -1 */
	int power;           /* fixed-point power accumulator */
	int base_power;      /* FP_ONE or FP_ONE/2 (sand) */
	int prev_base_power;
	enum ball_state state;
};

static enum skygolf_state skygolf_state = SKYGOLF_INIT;
static unsigned long long last_frame;

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

static void sfx_charge(void)      { sfx_debug_beep(700, 30); }
static void sfx_swing(void)       { sfx_debug_beep(1400, 40); }
static void sfx_bounce(void)      { sfx_debug_beep(1000, 18); }
static void sfx_sand(void)        { sfx_debug_beep(300, 40); }
static void sfx_water(void)       { sfx_debug_beep(220, 90); }
static void sfx_hole(void)        { sfx_debug_beep(2000, 220); }
static void sfx_fail(void)        { sfx_debug_beep(160, 300); }
static void sfx_next_hole(void)   { sfx_debug_beep(1500, 120); }
static void sfx_menu_move(void)   { sfx_debug_beep(1200, 15); }
static void sfx_menu_select(void) { sfx_debug_beep(1700, 40); }

#define NUM_MENU_ITEMS 2
#define MENU_ITEM_SPACING 30
#define MENU_ITEM_WIDTH 100
#define MENU_ITEM_HEIGHT 20
#define MENU_X (LCD_XSIZE / 2 - MENU_ITEM_WIDTH / 2)
#define MENU_Y (LCD_YSIZE / 2 - MENU_ITEM_HEIGHT / 2)
static int current_menu_item = 0;
static bool current_menu_item_selected = false;
static const char *menu_items[NUM_MENU_ITEMS] = {
	"play", "exit"
};

static struct platform platforms[MAX_PLATFORMS];
static int num_platforms;
static struct ball ball;
static int ball_count;
static int hole_count;
static int course_difficulty; /* 0=easy, 1=medium, 2=hard */
static int hole_starting_ticks;
static int transition_ticks;
static int pending_down_latches;
static int pending_up_latches;

static unsigned int rng_state;

static unsigned int rng(void)
{
	return xorshift(&rng_state);
}

static struct particle_pool *sparkpool = NULL;
#define SKYGOLF_POOL_SIG 0x5679014

static void spawn_particles(int px, int py, int count, int spread, unsigned short color)
{
	for (int i = 0; i < count; i++) {
		int vx = (int)(rng() % (unsigned int)(spread * 2 + 1)) - spread;
		int vy = -(int)(rng() % (unsigned int)(spread + 1)) - spread / 2;
		int life = (int)(rng() % 40) + 20;
		sparkpool->config.add_particle(sparkpool,
			px * 256, py * 256, vx, vy, life, color);
	}
}

static const int holes_per_difficulty[3] = {3, 6, 9};

/* --- level definitions ---
 *
 * Each level is an array of platform defs. First platform = tee.
 * Tile legend:
 *   f = fairway    s = sand    w = water
 *   t = short tree (h=10)    T = tall tree (h=16)
 *   H = flag/hole
 *
 * Each tile is 6px wide on screen.  LCD is 160x128.
 */

struct platform_def {
	int x, y;
	const char *tiles;
};

#define END_LEVEL {0, 0, NULL}

/* Easy: 3 holes - gentle, short shots */
static const struct platform_def hole_e1[] = {
	{  0, 110, "fffff"},
	{ 70,  80, "ffHfff"},
	END_LEVEL,
};

static const struct platform_def hole_e2[] = {
	{  0, 108, "ffffff"},
	{ 80,  70, "ffsHff"},
	END_LEVEL,
};

static const struct platform_def hole_e3[] = {
	{  0, 112, "ffff"},
	{ 55,  85, "ffffffff"},
	{100,  55, "ffHff"},
	END_LEVEL,
};

/* Medium: 6 holes - longer shots, hazards */
static const struct platform_def hole_m1[] = {
	{  0, 110, "ffffft"},
	{ 80,  65, "ffssHfff"},
	END_LEVEL,
};

static const struct platform_def hole_m2[] = {
	{  0, 105, "fffff"},
	{ 40,  80, "ffwwff"},
	{ 95,  50, "fffHff"},
	END_LEVEL,
};

static const struct platform_def hole_m3[] = {
	{  0, 112, "fffffft"},
	{100,  75, "ffsHsf"},
	END_LEVEL,
};

static const struct platform_def hole_m4[] = {
	{  0, 108, "ffff"},
	{ 50,  85, "fftff"},
	{ 95,  55, "fwHwf"},
	END_LEVEL,
};

static const struct platform_def hole_m5[] = {
	{  0, 115, "ffffffss"},
	{ 70,  70, "tffHfft"},
	END_LEVEL,
};

static const struct platform_def hole_m6[] = {
	{  0, 105, "fffff"},
	{ 30,  75, "ffwwwff"},
	{ 90,  45, "fffHff"},
	END_LEVEL,
};

/* Hard: 9 holes - tight gaps, multi-bounce, lots of hazards */
static const struct platform_def hole_h1[] = {
	{  0, 115, "ffff"},
	{ 55,  80, "fwwf"},
	{105,  50, "fsHsf"},
	END_LEVEL,
};

static const struct platform_def hole_h2[] = {
	{  0, 108, "ffffft"},
	{ 75,  60, "TffHffT"},
	END_LEVEL,
};

static const struct platform_def hole_h3[] = {
	{  0, 112, "fff"},
	{ 40,  90, "ffsff"},
	{ 85,  60, "fwwf"},
	{120,  35, "ffHf"},
	END_LEVEL,
};

static const struct platform_def hole_h4[] = {
	{  0, 110, "ffff"},
	{ 90,  55, "fssHssf"},
	END_LEVEL,
};

static const struct platform_def hole_h5[] = {
	{  0, 115, "fffft"},
	{ 50,  80, "fwf"},
	{ 85,  50, "TfHfT"},
	END_LEVEL,
};

static const struct platform_def hole_h6[] = {
	{  0, 105, "fffff"},
	{ 45,  70, "ffTff"},
	{100,  40, "fwHwf"},
	END_LEVEL,
};

static const struct platform_def hole_h7[] = {
	{  0, 112, "fff"},
	{ 35,  85, "fsf"},
	{ 70,  60, "fwf"},
	{110,  38, "ffHff"},
	END_LEVEL,
};

static const struct platform_def hole_h8[] = {
	{  0, 108, "ffffffs"},
	{100,  65, "TfHfT"},
	END_LEVEL,
};

static const struct platform_def hole_h9[] = {
	{  0, 115, "ffff"},
	{ 40,  90, "fwwwf"},
	{ 80,  60, "fssTf"},
	{120,  35, "fHf"},
	END_LEVEL,
};

static const struct platform_def *all_holes[3][9] = {
	/* easy */  {hole_e1, hole_e2, hole_e3, NULL, NULL, NULL, NULL, NULL, NULL},
	/* medium */ {hole_m1, hole_m2, hole_m3, hole_m4, hole_m5, hole_m6, NULL, NULL, NULL},
	/* hard */  {hole_h1, hole_h2, hole_h3, hole_h4, hole_h5, hole_h6, hole_h7, hole_h8, hole_h9},
};

static void load_hole(const struct platform_def *def)
{
	num_platforms = 0;

	for (int p = 0; def[p].tiles != NULL; p++) {
		if (num_platforms >= MAX_PLATFORMS)
			break;

		struct platform *plat = &platforms[num_platforms];
		plat->x = def[p].x;
		plat->y = def[p].y;
		plat->num_grounds = 0;

		for (const char *c = def[p].tiles; *c != '\0'; c++) {
			if (plat->num_grounds >= MAX_GROUNDS)
				break;
			struct ground *g = &plat->grounds[plat->num_grounds];
			g->tree_height = 0;
			switch (*c) {
			case 'f': case '.':
				g->type = GROUND_FAIRWAY;
				break;
			case 's':
				g->type = GROUND_SAND;
				break;
			case 'w':
				g->type = GROUND_WATER;
				break;
			case 't':
				g->type = GROUND_TREE;
				g->tree_height = 10;
				break;
			case 'T':
				g->type = GROUND_TREE;
				g->tree_height = 16;
				break;
			case 'H':
				g->type = GROUND_FLAG;
				break;
			default:
				g->type = GROUND_FAIRWAY;
				break;
			}
			plat->num_grounds++;
		}
		num_platforms++;
	}

	ball.x = TO_FP(platforms[0].x + 5);
	ball.y = TO_FP(platforms[0].y - 14);
}

static void init_ball(void)
{
	ball.vx = 0;
	ball.vy = 0;
	ball.angle = 40;
	ball.angle_dir = -1;
	ball.power = TO_FP(0);
	ball.base_power = FP_ONE;
	ball.prev_base_power = FP_ONE;
	ball.state = BALL_SHOT;
}

static void init_ball_shot_state(void)
{
	ball.state = BALL_SHOT;
	ball.power = TO_FP(0) + FP_ONE / 10;
}

static void back_to_prev_pos(void)
{
	ball.x = ball.prev_x;
	ball.y = ball.prev_y;
	ball.base_power = ball.prev_base_power;
}

/* Check collision of ball (bx, by in pixels) against all platforms.
 * Returns the ground type at collision, or -1 if no collision.
 */
static int check_platform_collision(int bx, int by)
{
	for (int p = 0; p < num_platforms; p++) {
		struct platform *plat = &platforms[p];
		int plat_top = plat->y - 5; /* top of ground surface */
		int plat_bot = plat->y;     /* bottom of platform (red base) */
		int plat_left = plat->x;
		int plat_right = plat->x + plat->num_grounds * 6;

		int gi_lo, gi_hi, gi, center_gi;

		/* ball is 4x4 pixels.  Reject if it is entirely beside or below
		 * the platform; either way it can't touch any column. */
		if (bx + 4 <= plat_left || bx >= plat_right || by > plat_bot)
			continue;

		/* every column the 4px-wide ball overlaps, not just its center */
		gi_lo = (bx - plat_left) / 6;
		gi_hi = (bx + 3 - plat_left) / 6;
		if (gi_lo < 0)
			gi_lo = 0;
		if (gi_hi >= plat->num_grounds)
			gi_hi = plat->num_grounds - 1;

		/* trees are vertical obstacles: any overlapped column whose trunk
		 * the ball reaches hits, even when the ball's center is elsewhere. */
		for (gi = gi_lo; gi <= gi_hi; gi++) {
			if (plat->grounds[gi].type != GROUND_TREE)
				continue;
			if (by + 4 >= plat_top - plat->grounds[gi].tree_height)
				return GROUND_TREE;
		}

		/* ground surface — report the type under the ball's center */
		if (by + 4 >= plat_top) {
			center_gi = (bx + 2 - plat_left) / 6;
			if (center_gi < 0)
				center_gi = 0;
			if (center_gi >= plat->num_grounds)
				center_gi = plat->num_grounds - 1;
			return (int)plat->grounds[center_gi].type;
		}
	}
	return -1;
}

#define NUM_CLOUDS 4

static struct cloud {
	int x;    /* .8 fixed point x position */
	int y;    /* pixel y */
	int w;    /* pixel width */
	int h;    /* pixel height */
	int speed; /* .8 fixed point speed */
	int phase;
	int variant;
} clouds[NUM_CLOUDS];

static int clouds_inited;

static void reset_cloud(struct cloud *c, int x)
{
	c->x = TO_FP(x);
	c->y = 5 + (int)(rng() % 35);
	c->w = 20 + (int)(rng() % 22);
	c->h = 7 + (int)(rng() % 6);
	c->speed = 6 + (int)(rng() % 24); /* slow drift */
	c->phase = (int)(rng() % 32);
	c->variant = (int)(rng() % 3);
}

static void init_clouds(void)
{
	clouds_inited = 1;
	rng_state = 0xDEADBEEF;
	for (int i = 0; i < NUM_CLOUDS; i++)
		reset_cloud(&clouds[i], (int)(rng() % LCD_XSIZE));
}

static void draw_sun(int x, int y)
{
	FbColor(COLOR_YELLOW);
	FbClippedLine(x, y - 12, x, y - 8);
	FbClippedLine(x, y + 8, x, y + 12);
	FbClippedLine(x - 13, y, x - 8, y);
	FbClippedLine(x + 8, y, x + 13, y);
	FbClippedLine(x - 9, y - 9, x - 6, y - 6);
	FbClippedLine(x + 9, y - 9, x + 6, y - 6);
	FbClippedLine(x - 9, y + 9, x - 6, y + 6);
	FbClippedLine(x + 9, y + 9, x + 6, y + 6);

	FbColor(COLOR_ORANGE);
	FbMove(x - 4, y - 8);
	FbFilledRectangle(9, 1);
	FbMove(x - 6, y - 7);
	FbFilledRectangle(13, 2);
	FbMove(x - 7, y - 5);
	FbFilledRectangle(15, 10);
	FbMove(x - 6, y + 5);
	FbFilledRectangle(13, 2);
	FbMove(x - 4, y + 7);
	FbFilledRectangle(9, 1);

	FbColor(COLOR_YELLOW);
	FbMove(x - 4, y - 6);
	FbFilledRectangle(9, 2);
	FbMove(x - 5, y - 4);
	FbFilledRectangle(11, 8);
	FbMove(x - 4, y + 4);
	FbFilledRectangle(9, 2);

	FbColor(COLOR_WHITE);
	FbMove(x - 2, y - 4);
	FbFilledRectangle(4, 3);
}

static void draw_cloud_rect(int x, int y, int w, int h)
{
	if (x < 0) {
		w += x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		y = 0;
	}
	if (x + w > LCD_XSIZE)
		w = LCD_XSIZE - x;
	if (y + h > LCD_YSIZE)
		h = LCD_YSIZE - y;
	if (w <= 0 || h <= 0)
		return;

	FbMove(x, y);
	FbFilledRectangle(w, h);
}

static void draw_cloud(int x, int y, int w, int h, int variant)
{
	if (x >= LCD_XSIZE || x + w < 0 || y >= LCD_YSIZE || y + h < 0)
		return;

	FbColor(COLOR_CLOUD);
	draw_cloud_rect(x + 2, y + h / 2, w - 4, h / 2);

	switch (variant) {
	case 1:
		draw_cloud_rect(x, y + h / 2 + 1, w, h / 3);
		draw_cloud_rect(x + 5, y + h / 3, w / 3, h / 2);
		draw_cloud_rect(x + w / 2, y, w / 3, h * 2 / 3);
		draw_cloud_rect(x + w - w / 4 - 3, y + h / 3, w / 4, h / 2);
		break;
	case 2:
		draw_cloud_rect(x + 1, y + h / 3, w - 2, h / 2);
		draw_cloud_rect(x + w / 5, y + 1, w / 3, h * 2 / 3);
		draw_cloud_rect(x + w / 2, y - 1, w / 3, h * 3 / 4);
		break;
	default:
		draw_cloud_rect(x + 4, y + h / 4, w / 3, h / 2);
		draw_cloud_rect(x + w / 3, y, w / 3, h * 3 / 4);
		draw_cloud_rect(x + w * 2 / 3 - 2, y + h / 3, w / 4, h / 2);
		break;
	}

	FbColor(COLOR_WHITE);
	draw_cloud_rect(x + w / 4, y + h / 4, w / 3, 1);

	FbColor(COLOR_CLOUD_SH);
	draw_cloud_rect(x + 3, y + h, w - 6, 1);
}

static void update_cloud(struct cloud *c)
{
	c->x += c->speed;
	if (TO_INT(c->x) > LCD_XSIZE + 10)
		reset_cloud(c, -c->w - 5);
}

static void update_clouds(void)
{
	for (int i = 0; i < NUM_CLOUDS; i++)
		update_cloud(&clouds[i]);
}

static void draw_clouds(void)
{
	for (int i = 0; i < NUM_CLOUDS; i++) {
		struct cloud *c = &clouds[i];
		int px = TO_INT(c->x);
		int bob = ((px + c->phase) / 8) & 3;
		if (bob > 1)
			bob = 3 - bob;

		draw_cloud(px, c->y + bob, c->w, c->h, c->variant);
	}
}

static void draw_sky(void)
{
	draw_sun(137, 15);
	draw_clouds();
}

static void draw_ground_rect(int x, int y, int w, enum ground_type type)
{
	switch (type) {
	case GROUND_FAIRWAY:
		FbColor(COLOR_GREEN);
		break;
	case GROUND_SAND:
		FbColor(COLOR_SAND);
		break;
	case GROUND_WATER:
		FbColor(COLOR_WATER);
		break;
	case GROUND_TREE:
		FbColor(COLOR_GREEN);
		break;
	case GROUND_FLAG:
		FbColor(COLOR_WHITE);
		break;
	}
	if (x < 0) {
		w += x;
		x = 0;
	}
	if (x + w > LCD_XSIZE)
		w = LCD_XSIZE - x;
	if (w > 0) {
		FbMove(x, y);
		FbFilledRectangle(w, 3);
		FbMove(x, y);
	}
}

static void draw_tree(int x, int y, int h)
{
	int h2 = h / 2;
	/* trunk */
	FbColor(COLOR_BROWN);
	if (x + 1 >= 0 && x + 4 < LCD_XSIZE) {
		FbMove(x + 1, y - h2);
		FbFilledRectangle(3, h2);
	}
	/* leaves */
	FbColor(COLOR_GREEN);
	if (x >= 0 && x + 5 < LCD_XSIZE) {
		FbMove(x, y - h);
		FbFilledRectangle(5, h2);
	}
}

static void draw_flag(int x, int y)
{
	/* pole */
	FbColor(COLOR_WHITE);
	if (x + 1 >= 0 && x + 3 < LCD_XSIZE) {
		FbMove(x + 1, y - 10);
		FbFilledRectangle(2, 10);
	}
	/* flag */
	FbColor(COLOR_RED);
	if (x + 3 >= 0 && x + 8 < LCD_XSIZE) {
		FbMove(x + 3, y - 10);
		FbFilledRectangle(5, 4);
	}
}

static void draw_red_base(struct platform *plat)
{
	FbColor(COLOR_RED);
	int rx = plat->x;
	int rw = plat->num_grounds * 6;
	if (rx < 0) {
		rw += rx;
		rx = 0;
	}
	if (rx + rw > LCD_XSIZE) rw = LCD_XSIZE - rx;
	if (rw > 0) {
		FbMove(rx, plat->y - 2);
		FbFilledRectangle(rw, 2);
	}
}

static void draw_ground_segments(struct platform *plat)
{
	int x = plat->x;
	enum ground_type prev_type = plat->grounds[0].type;
	int seg_start = plat->x;

	for (int g = 0; g < plat->num_grounds; g++) {
		struct ground *gr = &plat->grounds[g];

		if (gr->type != prev_type) {
			draw_ground_rect(seg_start, plat->y - 5,
					 x - seg_start, prev_type);
			seg_start = x;
			prev_type = gr->type;
		}

		if (gr->type == GROUND_TREE)
			draw_tree(x, plat->y - 5, gr->tree_height);
		else if (gr->type == GROUND_FLAG)
			draw_flag(x, plat->y - 5);

		x += 6;
	}
	draw_ground_rect(seg_start, plat->y - 5, x - seg_start, prev_type);
}

static void draw_hole(void)
{
	for (int p = 0; p < num_platforms; p++) {
		struct platform *plat = &platforms[p];

		draw_red_base(plat);
		draw_ground_segments(plat);
	}
}

static void draw_ball(void)
{
	int bx = TO_INT(ball.x);
	int by = TO_INT(ball.y);

	if (bx < 0 || bx >= LCD_XSIZE - 4 || by < 0 || by >= LCD_YSIZE - 4)
		return;

	if (ball.state == BALL_SHOT && ball.base_power < FP_ONE)
		FbColor(COLOR_SAND);
	else
		FbColor(COLOR_WHITE);

	FbMove(bx + 1, by);
	FbFilledRectangle(2, 1);
	FbMove(bx, by + 1);
	FbFilledRectangle(4, 2);
	FbMove(bx + 1, by + 3);
	FbFilledRectangle(2, 1);
}

static void draw_aim_line(int length)
{
	int bx = TO_INT(ball.x) + 2;
	int by = TO_INT(ball.y) + 2;
	/* badge trig: angle 0-127, cosine/sine return value*256 */
	int ex = bx + (length * cosine(ball.angle)) / 256;
	int ey = by + (length * -sine(ball.angle)) / 256;

	FbColor(COLOR_WHITE);
	FbClippedLine(bx, by, ex, ey);
}

static void draw_power_bar(void)
{
	int power_pixels = TO_INT(ball.power);
	if (power_pixels < 0)
		power_pixels = 0;
	if (power_pixels > MAX_POWER)
		power_pixels = MAX_POWER;

	/* fill color ramps: green → yellow → red */
	unsigned short fill_color;
	if (power_pixels < MAX_POWER / 3)
		fill_color = COLOR_GREEN;
	else if (power_pixels < (MAX_POWER * 2) / 3)
		fill_color = COLOR_YELLOW;
	else
		fill_color = COLOR_RED;

	struct ui_progress_bar bar = {
		.x = (LCD_XSIZE - 120) / 2,
		.y = LCD_YSIZE - 16,
		.width = 120,
		.height = 10,
		.outline_size = 1,
		.fill_color = fill_color,
		.empty_color = COLOR_DARK_GREY,
		.outline_color = COLOR_BLACK,
		.fill = ui_progress_bar_calculate_fill_percentage(
			(power_pixels * 100) / MAX_POWER),
	};
	ui_progress_bar_draw(bar);
}

static void draw_hud_ball_count(void)
{
	char buf[32];

	FbMove(3, 2);
	FbFilledRectangle(3, 3);
	snprintf(buf, sizeof(buf), "x%d", ball_count);
	FbMove(8, 1);
	FbWriteString(buf);
}

static void draw_hud(void)
{
	FbColor(COLOR_WHITE);
	draw_hud_ball_count();
}

static void go_to_next_hole(void);
static void init_give_up(void);
static void init_hole_out(void);

/* badge angles: 0 = right, 32 = up, 64 = left, 96 = down.
 * The aim sweeps the upper half, between about 64 (left/up) and 32 (up/right).
 */
static void oscillate_aim_angle(void)
{
	ball.angle += ball.angle_dir;
	if (ball.angle <= 16 && ball.angle_dir < 0) {
		ball.angle_dir = 1;
		ball.angle += 2;
	}
	if (ball.angle >= 64 && ball.angle_dir > 0) {
		ball.angle_dir = -1;
		ball.angle -= 2;
	}
}

static void update_shot_state(int down_latches)
{
	oscillate_aim_angle();

	if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
		ball.state = BALL_POWER;
		ball.power = FP_ONE / 10;
		sfx_charge();
	}
}

static void update_power_state(int up_latches)
{
	ball.power += FP_ONE / 5; /* accumulate power */

	if (TO_INT(ball.power) > MAX_POWER || BUTTON_PRESSED(BADGE_BUTTON_A, up_latches)) {
		/* set velocity based on angle and power */
		int spd = FP_MUL(ball.power / 2, ball.base_power);
		ball.vx = (spd * cosine(ball.angle)) / 256;
		ball.vy = (spd * -sine(ball.angle)) / 256;
		ball.state = BALL_FLY;
		ball_count--;
		sfx_swing();
	}
}

static void ball_step_horizontal(void)
{
	int next_x = ball.x + ball.vx;
	int bx = TO_INT(next_x);
	int by = TO_INT(ball.y);
	int coll_h = check_platform_collision(bx, by);

	if (coll_h >= 0 || (ball.vx < 0 && bx < 2) ||
	    (ball.vx > 0 && bx > LCD_XSIZE - 6)) {
		ball.vx = FP_MUL(ball.vx, -TO_FP(8) / 10);
		ball.vy = FP_MUL(ball.vy, TO_FP(8) / 10);
		sfx_bounce();
		return;
	}
	ball.x = next_x;
}

/* Ball landed in water: splash and reset to the previous safe position.
 * Returns true if the fly update should stop (give-up or replay this shot).
 */
static bool handle_water(void)
{
	sfx_water();
	spawn_particles(TO_INT(ball.x) + 2, TO_INT(ball.y) + 4,
		10, 100, COLOR_WATER); /* big splash */
	if (ball_count <= 0) {
		init_give_up();
		return true;
	}
	back_to_prev_pos();
	init_ball_shot_state();
	return true;
}

/* Ball hit sand: dampened bounce ratio plus a small spray. */
static void handle_sand(int *vr_num)
{
	*vr_num = 5;
	spawn_particles(TO_INT(ball.x) + 2, TO_INT(ball.y) + 4,
		4, 60, COLOR_SAND); /* yellow sand */
	sfx_sand();
}

/* Decide whether the ball has settled after a bounce; if so, transition into
 * the next shot (or end the hole / course). Returns true if the fly update
 * should stop.
 */
static bool try_settle_ball(int coll_v)
{
	int speed_sq = FP_MUL(ball.vx, ball.vx) + FP_MUL(ball.vy, ball.vy);
	if (ball.vy < 0 && speed_sq < FP_ONE / 4) {
		if (coll_v == GROUND_FLAG) {
			init_hole_out();
			return true;
		}
		if (ball_count <= 0) {
			init_give_up();
			return true;
		}

		init_ball_shot_state();
		ball.base_power = (coll_v == GROUND_SAND) ? FP_ONE / 2 : FP_ONE;

		ball.prev_x = ball.x;
		ball.prev_y = ball.y;
		ball.prev_base_power = ball.base_power;
		return true;
	}
	return false;
}

/* Returns true if the fly update should stop (ball settled, scored, or lost). */
static bool ball_step_vertical(void)
{
	int next_y = ball.y + ball.vy;
	int bx = TO_INT(ball.x);
	int by = TO_INT(next_y);
	int coll_v = check_platform_collision(bx, by);

	if (coll_v < 0) {
		ball.y = next_y;
		return false;
	}

	int vr_num = 8; /* bounce ratio numerator (out of 10) */

	if (ball.vy > 0 && coll_v == GROUND_WATER)
		return handle_water();
	else if (ball.vy > 0 && coll_v == GROUND_SAND)
		handle_sand(&vr_num);
	else
		sfx_bounce();

	ball.vy = FP_MUL(ball.vy, -TO_FP(vr_num) / 10);
	ball.vx = FP_MUL(ball.vx, TO_FP(vr_num) / 10);

	return try_settle_ball(coll_v);
}

static void apply_gravity(void)
{
	ball.vy += FP_ONE / 10;
}

static void apply_air_friction(void)
{
	ball.vx = FP_MUL(ball.vx, TO_FP(98) / 100);
	ball.vy = FP_MUL(ball.vy, TO_FP(98) / 100);
}

/* Ball fell off the bottom of the screen: replay the shot (or give up). */
static void handle_off_screen_bottom(void)
{
	if (TO_INT(ball.y) <= LCD_YSIZE + 16)
		return;
	if (ball_count <= 0) {
		init_give_up();
		return;
	}
	back_to_prev_pos();
	init_ball_shot_state();
}

static void update_fly_state(void)
{
	ball_step_horizontal();

	if (ball_step_vertical())
		return;

	apply_gravity();
	apply_air_friction();
	handle_off_screen_bottom();
}

static void go_to_next_hole(void)
{
	skygolf_state = SKYGOLF_INGAME;
	init_ball();
	load_hole(all_holes[course_difficulty][hole_count]);
	ball.prev_x = ball.x;
	ball.prev_y = ball.y;
	hole_starting_ticks = 90; /* show "HOLE N" for 3 seconds */
	hole_count++;
	ball_count += 5;
	sfx_next_hole();
	init_ball_shot_state();
}

static void init_in_game(int difficulty)
{
	course_difficulty = difficulty;
	ball_count = 0;
	hole_count = 0;
	pending_down_latches = 0;
	pending_up_latches = 0;
	rng_state = (unsigned int)rtc_get_ms_since_boot();
	if (rng_state == 0)
		rng_state = 1;
	if (sparkpool)
		sparkpool->nparticles = 0;
	go_to_next_hole();
}

static void init_give_up(void)
{
	sfx_fail();
	skygolf_state = SKYGOLF_GIVE_UP;
	transition_ticks = 0;
}

static void init_hole_out(void)
{
	sfx_hole();
	if (hole_count >= holes_per_difficulty[course_difficulty]) {
		skygolf_state = SKYGOLF_HOLE_OUT;
		transition_ticks = 0;
		return;
	}
	skygolf_state = SKYGOLF_NEXT_HOLE;
	transition_ticks = 0;
}

static void previous_menu_item(void)
{
	current_menu_item--;
	if (current_menu_item < 0)
		current_menu_item = NUM_MENU_ITEMS - 1;
	sfx_menu_move();
}

static void next_menu_item(void)
{
	current_menu_item++;
	if (current_menu_item >= NUM_MENU_ITEMS)
		current_menu_item = 0;
	sfx_menu_move();
}

static void handle_menu_options(void)
{
	sfx_menu_select();
	switch (current_menu_item) {
	case 0: /* play - go to title/difficulty select */
		skygolf_state = SKYGOLF_TITLE;
		break;
	case 1: /* exit */
		skygolf_state = SKYGOLF_EXIT;
		break;
	}
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
			.outline_color = COLOR_WATER,
			.fill_color = COLOR_SKY_DARK,
			.text_color = COLOR_WHITE,
		};

		if (i == current_menu_item) {
			button.outline_color = COLOR_LIGHT_GREY;
			button.fill_color = COLOR_LAVENDER;
			if (current_menu_item_selected)
				button.fill_color = COLOR_DARK_GREY;
		}

		if (button.y < 0 || button.y > LCD_YSIZE)
			continue;
		ui_button_dither_fill(button, button.fill_color, COLOR_BLACK, 1);
		ui_button_draw_outline(button, button.outline_color);
		ui_button_draw_label(button, button.text_color);
	}
}

static int title_cursor; /* 0=easy, 1=medium, 2=hard */

static void draw_title(void)
{
	FbColor(COLOR_WHITE);
	FbMove(ui_center_text_x("SKY GOLF", 0, LCD_XSIZE), 15);
	FbWriteString("SKY GOLF");

	const char *labels[] = {"Easy  (3)", "Medium(6)", "Hard  (9)"};
	for (int i = 0; i < 3; i++) {
		struct ui_button btn = {
			.x = 30,
			.y = 40 + i * 25,
			.width = 100,
			.height = 18,
			.text = labels[i],
			.outline_size = 1,
			.outline_color = COLOR_DARK_GREY,
			.fill_color = COLOR_SKY_DARK,
			.text_color = COLOR_WHITE,
		};
		if (i == title_cursor) {
			btn.outline_color = COLOR_SAND;
			btn.fill_color = COLOR_GREEN;
		}
		ui_button_fill(btn, btn.fill_color);
		ui_button_draw_outline(btn, btn.outline_color);
		ui_button_draw_label(btn, btn.text_color);
	}

	FbColor(COLOR_WHITE);
	FbMove(ui_center_text_x("A:select B:back", 0, LCD_XSIZE), 115);
	FbWriteString("A:select B:back");
}

static void update_ingame(int frame_down_latches, int frame_up_latches)
{
	if (ball.state == BALL_SHOT) {
		update_shot_state(frame_down_latches);
		if (ball.state == BALL_POWER &&
		    BUTTON_PRESSED(BADGE_BUTTON_A, frame_up_latches))
			update_power_state(frame_up_latches);
	} else if (ball.state == BALL_POWER)
		update_power_state(frame_up_latches);
	else if (ball.state == BALL_FLY)
		update_fly_state();

	update_clouds();
	sparkpool->config.move_particles(sparkpool);

	if (BUTTON_PRESSED(BADGE_BUTTON_B, frame_down_latches)) {
		pending_down_latches = 0;
		pending_up_latches = 0;
		skygolf_state = SKYGOLF_MENU;
	}
}

/* "HOLE N" banner shown for the first few seconds of a hole. */
static void draw_hole_intro(void)
{
	if (hole_starting_ticks <= 0)
		return;

	char buf[16];
	hole_starting_ticks--;
	FbColor(COLOR_WHITE);
	snprintf(buf, sizeof(buf), "HOLE %d", hole_count);
	FbMove(ui_center_text_x(buf, 0, LCD_XSIZE), 5);
	FbWriteString(buf);
}

static void draw_ingame(void)
{
	FbClear();
	draw_sky();
	draw_hole();
	draw_ball();
	sparkpool->config.draw_particles(sparkpool);

	if (ball.state == BALL_SHOT)
		draw_aim_line(12);
	else if (ball.state == BALL_POWER) {
		draw_aim_line(12);
		draw_power_bar();
	}

	draw_hole_intro();

	draw_hud();
	FbSwapBuffers();
}

void skygolf_cb(struct badge_app *app)
{
	if (app->wake_up) {	/* acknowledge another app ran; we redraw every frame */
		app->wake_up = 0;
		FbBackgroundColor(COLOR_SKY_BLUE);
	}
	int dl = button_down_latches();
	int ul = button_up_latches();

	if (sparkpool == NULL) {
		sparkpool = get_common_particle_pool();
		sparkpool->nparticles = 0;
	}
	if (claim_particle_pool(sparkpool, SKYGOLF_POOL_SIG))
		sparkpool->config.gravityy = 8;

	switch (skygolf_state) {
	case SKYGOLF_INIT:
		FbInit();
		FbBackgroundColor(COLOR_SKY_BLUE);
		FbClear();
		if (!clouds_inited)
			init_clouds();
		current_menu_item = 0;
		title_cursor = 0;
		skygolf_state = SKYGOLF_MENU;
		break;

	case SKYGOLF_MENU:
		current_menu_item_selected = false;
		if (BUTTON_PRESSED(BADGE_BUTTON_UP, dl))
			previous_menu_item();
		else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, dl))
			next_menu_item();
		else if (BUTTON_PRESSED(BADGE_BUTTON_A, dl)) {
			current_menu_item_selected = true;
			handle_menu_options();
		} else if (BUTTON_PRESSED(BADGE_BUTTON_B, dl))
			skygolf_state = SKYGOLF_EXIT;
		FbClear();
		update_clouds();
		draw_sky();
		draw_menu();
		FbSwapBuffers();
		break;

	case SKYGOLF_TITLE:
		if (BUTTON_PRESSED(BADGE_BUTTON_UP, dl)) {
			title_cursor--;
			if (title_cursor < 0)
				title_cursor = 2;
		} else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, dl)) {
			title_cursor++;
			if (title_cursor > 2)
				title_cursor = 0;
		} else if (BUTTON_PRESSED(BADGE_BUTTON_A, dl))
			init_in_game(title_cursor);
		else if (BUTTON_PRESSED(BADGE_BUTTON_B, dl))
			skygolf_state = SKYGOLF_MENU;
		FbClear();
		update_clouds();
		draw_sky();
		draw_title();
		FbSwapBuffers();
		break;

	case SKYGOLF_INGAME: {
		pending_down_latches |= dl;
		pending_up_latches |= ul;
		unsigned long long now = rtc_get_ms_since_boot();
		if (now - last_frame < FRAME_MS)
			return;
		last_frame = now;
		int frame_down_latches = pending_down_latches;
		int frame_up_latches = pending_up_latches;
		pending_down_latches = 0;
		pending_up_latches = 0;

		update_ingame(frame_down_latches, frame_up_latches);
		draw_ingame();
		break;
	}

	case SKYGOLF_NEXT_HOLE:
		transition_ticks++;
		if (BUTTON_PRESSED(BADGE_BUTTON_A, dl) || transition_ticks > 120)
			go_to_next_hole();
		FbClear();
		update_clouds();
		draw_sky();
		draw_hole();
		FbColor(COLOR_WHITE);
		FbMove(ui_center_text_x("GO TO NEXT HOLE", 0, LCD_XSIZE), 55);
		FbWriteString("GO TO NEXT HOLE");
		draw_hud();
		FbSwapBuffers();
		break;

	case SKYGOLF_GIVE_UP:
		transition_ticks++;
		if (BUTTON_PRESSED(BADGE_BUTTON_A, dl) || transition_ticks > 180)
			skygolf_state = SKYGOLF_TITLE;
		FbClear();
		update_clouds();
		draw_sky();
		draw_hole();
		FbColor(COLOR_RED);
		FbMove(ui_center_text_x("GIVE UP", 0, LCD_XSIZE), 55);
		FbWriteString("GIVE UP");
		FbColor(COLOR_WHITE);
		FbMove(ui_center_text_x("press A", 0, LCD_XSIZE), 70);
		FbWriteString("press A");
		FbSwapBuffers();
		break;

	case SKYGOLF_HOLE_OUT:
		transition_ticks++;
		if (BUTTON_PRESSED(BADGE_BUTTON_A, dl) || transition_ticks > 300)
			skygolf_state = SKYGOLF_TITLE;
		FbClear();
		update_clouds();
		draw_sky();
		draw_hole();
		FbColor(COLOR_WHITE);
		FbMove(ui_center_text_x("HOLE OUT!", 0, LCD_XSIZE), 55);
		FbWriteString("HOLE OUT!");
		draw_hud();
		FbSwapBuffers();
		break;

	case SKYGOLF_EXIT:
		skygolf_state = SKYGOLF_INIT;
		current_menu_item = 0;
		pop_app();
		break;

	default:
		break;
	}
}
