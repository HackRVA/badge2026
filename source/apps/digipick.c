/*********************************************

A lockpicking minigame in the style of Starfield's "Digipick" security.

Each lock is a set of concentric rings.  A ring is a circle of slots; some
slots start out solid and the rest are gaps that must be filled in.  You have
a collection of picks ("digipicks"), each of which is a pattern of prongs.
Rotate a pick so its prongs line up with the empty gaps of the outermost
unsolved ring, then insert it to fill those gaps.  Fill every ring to open the
lock.

Controls:
   LEFT / RIGHT : rotate the selected pick
   UP / DOWN    : cycle through your picks
   A            : insert the pick (only if it fits)
   FASTFWD      : auto-insert a guaranteed-safe pick (hint)
   REWIND       : pull the last pick back out (undo)
   B            : quit

**********************************************/

#include <stdio.h>

#include "colors.h"
#include "button.h"
#include "framebuffer.h"
#include "badge.h"
#include "trig.h"
#include "xorshift.h"
#include "rtc.h"

#define N_SLOTS 16
#define FULL_MASK ((1u << N_SLOTS) - 1)
#define ANGLE_PER_SLOT (128 / N_SLOTS)	/* trig uses 0..127 for a full circle */

#define MAX_RINGS 4
#define MAX_KEYS 28
#define MAX_DECOYS 4

#define LOCK_CX 80
#define LOCK_CY 50
#define HUB_RADIUS 8
#define RING_INNER0 10			/* inner radius of the first ring */
#define RING_BAND 6			/* radial thickness of a ring */
#define RING_GAP 2			/* dark gap between rings */
#define RING_STEP (RING_BAND + RING_GAP)
#define TOOTH_HALF 3			/* tooth angular half-width, in trig units */

/* the pick tray along the bottom of the screen */
#define TRAY_Y 100
#define TRAY_ICON_R 7
#define TRAY_SPACING 18
#define TRAY_VISIBLE 7

/* sci-fi cyan/teal palette: locked rings read cool-dim, the active ring is
 * bright cyan, solved rings are teal-green. */
#define COL_HUB_OUTER    PACKRGB888(40, 70, 90)
#define COL_HUB_INNER    PACKRGB888(90, 170, 200)
#define COL_TRACK_LOCKED PACKRGB888(40, 48, 58)
#define COL_TRACK_ACTIVE PACKRGB888(30, 110, 150)
#define COL_TRACK_SOLVED PACKRGB888(20, 70, 70)
#define COL_TOOTH_LOCKED PACKRGB888(80, 95, 110)
#define COL_TOOTH_ACTIVE PACKRGB888(120, 225, 255)
#define COL_TOOTH_SOLVED PACKRGB888(40, 230, 170)
#define COL_FIT_HILITE   x11_deep_sky_blue	/* "this pick fits" cue */
#define COL_CAND_FIT     x11_spring_green	/* candidate preview, fits */
#define COL_CAND_NO      RED			/* candidate preview, collision */

#define ANIM_ROTATE_MS 110
#define ANIM_INSERT_MS 180
#define ANIM_UNDO_MS 160
#define ANIM_SELECT_MS 90
#define ANIM_WIN_MS 900

struct key {
	unsigned short prongs;	/* prong pattern in the pick's own coordinates */
	unsigned char rotation;	/* current rotation, 0..N_SLOTS-1 */
	unsigned char used;
};

struct undo_entry {
	unsigned char key_index;
	unsigned char ring_index;
	unsigned short placed_mask;	/* absolute slots this pick filled */
};

static unsigned short rings[MAX_RINGS];	/* bit s set => slot s is solid */
static int num_rings;
static struct key keys[MAX_KEYS];
static int num_keys;
static int sel_key;		/* index of the currently selected pick */
static int active_ring;		/* outermost unsolved ring, or -1 when the lock is open */
static struct undo_entry undo_stack[MAX_KEYS];
static int undo_count;
static int level;
static unsigned int rng_state;
static int screen_changed;

enum anim_kind_t {
	ANIM_NONE,
	ANIM_ROTATE,
	ANIM_INSERT,
	ANIM_UNDO,
	ANIM_SELECT,
	ANIM_WIN,
};

static enum anim_kind_t anim_kind;
static unsigned int anim_start_ms;
static unsigned int anim_duration_ms;
static unsigned short anim_mask;
static signed char anim_ring;
static signed char anim_key;

enum digipick_state_t {
	DIGIPICK_INIT,
	DIGIPICK_SPLASH,
	DIGIPICK_NEWLEVEL,
	DIGIPICK_PLAY,
	DIGIPICK_WIN,
	DIGIPICK_EXIT,
};

static enum digipick_state_t digipick_state = DIGIPICK_INIT;

/* rotate a N_SLOTS-bit mask left by r */
static unsigned short rotl(unsigned short x, int r)
{
	r %= N_SLOTS;
	if (r < 0)
		r += N_SLOTS;
	return (unsigned short)(((x << r) | (x >> (N_SLOTS - r))) & FULL_MASK);
}

static int popcount16(unsigned short x)
{
	int n = 0;
	while (x) {
		n += x & 1;
		x >>= 1;
	}
	return n;
}

static int random_num(int n)
{
	if (n <= 0)
		return 0;
	return (int)(xorshift(&rng_state) % (unsigned int)n);
}

static unsigned int now_ms(void)
{
	return (unsigned int)rtc_get_ms_since_boot();
}

static void start_anim(enum anim_kind_t kind, unsigned int duration,
		       signed char ring, signed char key, unsigned short mask)
{
	anim_kind = kind;
	anim_start_ms = now_ms();
	anim_duration_ms = duration;
	anim_ring = ring;
	anim_key = key;
	anim_mask = mask;
	screen_changed = 1;
}

static int anim_active(void)
{
	unsigned int elapsed;

	if (anim_kind == ANIM_NONE)
		return 0;
	elapsed = now_ms() - anim_start_ms;
	if (elapsed >= anim_duration_ms) {
		anim_kind = ANIM_NONE;
		return 1;	/* draw one cleanup frame with the overlay gone */
	}
	return 1;
}

static int anim_progress255(void)
{
	unsigned int elapsed;

	if (anim_kind == ANIM_NONE || anim_duration_ms == 0)
		return 255;
	elapsed = now_ms() - anim_start_ms;
	if (elapsed >= anim_duration_ms)
		return 255;
	return (int)((elapsed * 255u) / anim_duration_ms);
}

static unsigned short key_mask(const struct key *k)
{
	return rotl(k->prongs, k->rotation);
}

/* The pick fits the active ring if all its prongs land on empty gaps. */
static int key_fits_active(const struct key *k)
{
	unsigned short m;

	if (active_ring < 0)
		return 0;
	m = key_mask(k);
	if (m == 0)
		return 0;
	return (m & rings[active_ring]) == 0;
}

/* True if this pick could fit the active ring at *some* rotation (the blue
 * highlight hint -- note a pick may also fit a ring it doesn't belong to). */
static int key_fits_active_any_rot(const struct key *k)
{
	int r;

	if (active_ring < 0)
		return 0;
	for (r = 0; r < N_SLOTS; r++) {
		unsigned short m = rotl(k->prongs, r);

		if (m && (m & rings[active_ring]) == 0)
			return 1;
	}
	return 0;
}

/* Work from the outermost ring inward, like Starfield.  active_ring is -1
 * once every ring is solved. */
static void update_active_ring(void)
{
	int r;

	active_ring = -1;
	for (r = num_rings - 1; r >= 0; r--) {
		if (rings[r] != FULL_MASK) {
			active_ring = r;
			return;
		}
	}
}

static int count_unused_keys(void)
{
	int i, n = 0;

	for (i = 0; i < num_keys; i++)
		if (!keys[i].used)
			n++;
	return n;
}

static void select_step(int dir)
{
	int i;

	if (count_unused_keys() == 0)
		return;
	for (i = 0; i < num_keys; i++) {
		sel_key = (sel_key + dir + num_keys) % num_keys;
		if (!keys[sel_key].used)
			return;
	}
}

static void shuffle_keys(void)
{
	int i, j;
	struct key tmp;

	for (i = num_keys - 1; i > 0; i--) {
		j = random_num(i + 1);
		tmp = keys[i];
		keys[i] = keys[j];
		keys[j] = tmp;
	}
}

static void add_key_from_mask(unsigned short abs_mask)
{
	int rot0;

	if (num_keys >= MAX_KEYS)
		return;
	/* The pick lines up with abs_mask when rotated by rot0, so store the
	 * prongs rotated back the other way and start it mis-rotated. */
	rot0 = random_num(N_SLOTS);
	keys[num_keys].prongs = rotl(abs_mask, N_SLOTS - rot0);
	keys[num_keys].rotation = (unsigned char)random_num(N_SLOTS);
	keys[num_keys].used = 0;
	num_keys++;
}

/* Collect the gap positions of a ring into positions[] and shuffle them.
 * Returns the gap count. */
static int collect_gap_positions(unsigned short gaps, int positions[N_SLOTS])
{
	int s, i, np = 0;

	for (s = 0; s < N_SLOTS; s++)
		if (gaps & (1u << s))
			positions[np++] = s;
	for (i = np - 1; i > 0; i--) {
		int j = random_num(i + 1);
		int t = positions[i];
		positions[i] = positions[j];
		positions[j] = t;
	}
	return np;
}

/* partition the gaps into picks of 2-3 prongs */
static void build_ring_keys(int r, const int positions[N_SLOTS], int np)
{
	int i = 0;

	(void)r;
	while (i < np) {
		int chunk = 2 + random_num(2);
		unsigned short abs_mask = 0;
		int j;

		if (chunk > np - i)
			chunk = np - i;
		for (j = 0; j < chunk; j++)
			abs_mask |= (unsigned short)(1u << positions[i + j]);
		i += chunk;
		add_key_from_mask(abs_mask);
	}
}

static void build_ring(int r)
{
	unsigned short gaps = 0;
	int ngaps = 5 + random_num(5);		/* 5..9 gaps */
	int positions[N_SLOTS];
	int np;

	while (popcount16(gaps) < ngaps)
		gaps |= (unsigned short)(1u << random_num(N_SLOTS));
	rings[r] = (unsigned short)(FULL_MASK & ~gaps);

	np = collect_gap_positions(gaps, positions);
	build_ring_keys(r, positions, np);
}

/* throw in some decoy picks that don't belong to any ring */
static void build_decoys(void)
{
	int decoys = level - 1;
	int i;

	if (decoys > MAX_DECOYS)
		decoys = MAX_DECOYS;
	for (i = 0; i < decoys && num_keys < MAX_KEYS; i++) {
		unsigned short m = 0;
		int prongs = 2 + random_num(2);

		while (popcount16(m) < prongs)
			m |= (unsigned short)(1u << random_num(N_SLOTS));
		keys[num_keys].prongs = m;
		keys[num_keys].rotation = (unsigned char)random_num(N_SLOTS);
		keys[num_keys].used = 0;
		num_keys++;
	}
}

static void build_level(void)
{
	int r;

	num_rings = 2 + (level - 1);
	if (num_rings > MAX_RINGS)
		num_rings = MAX_RINGS;
	num_keys = 0;
	undo_count = 0;

	for (r = 0; r < num_rings; r++)
		build_ring(r);
	build_decoys();
	shuffle_keys();
}

static void generate_level(void)
{
	build_level();
	sel_key = 0;
	update_active_ring();
}

static void commit_key(void)
{
	struct key *k = &keys[sel_key];
	unsigned short m;

	if (active_ring < 0 || k->used || !key_fits_active(k))
		return;

	m = key_mask(k);
	start_anim(ANIM_INSERT, ANIM_INSERT_MS, (signed char)active_ring,
		   (signed char)sel_key, m);
	rings[active_ring] |= m;
	k->used = 1;

	undo_stack[undo_count].key_index = (unsigned char)sel_key;
	undo_stack[undo_count].ring_index = (unsigned char)active_ring;
	undo_stack[undo_count].placed_mask = m;
	undo_count++;

	update_active_ring();
	if (keys[sel_key].used)
		select_step(1);
}

static void undo_key(void)
{
	struct undo_entry *u;

	if (undo_count == 0)
		return;
	u = &undo_stack[--undo_count];
	rings[u->ring_index] &= (unsigned short)~u->placed_mask;
	keys[u->key_index].used = 0;
	sel_key = u->key_index;
	start_anim(ANIM_UNDO, ANIM_UNDO_MS, (signed char)u->ring_index,
		   (signed char)u->key_index, u->placed_mask);
	update_active_ring();
}

static int wrap_angle(int a)
{
	a %= 128;
	if (a < 0)
		a += 128;
	return a;
}

static void polar_at(int cx, int cy, int radius, int angle, int *x, int *y)
{
	angle = wrap_angle(angle);
	*x = cx + (radius * cosine(angle)) / 256;
	*y = cy - (radius * sine(angle)) / 256;
}

static void filled_disc(int cx, int cy, int r, unsigned short color)
{
	int rr;

	FbColor(color);
	for (rr = r; rr >= 0; rr--)
		FbDDACircle(cx, cy, rr);
}

/* Fill a ring "tooth": the solid wedge of the annulus [rin, rout] spanning the
 * angle a_center +/- half.  We scan the tooth's bounding box and keep pixels
 * inside the radius band (squared-distance test) and between the two boundary
 * rays (half-plane / cross-product test) -- a crisp, gap-free fill. */
static void wedge_at(int cx, int cy, int rin, int rout, int a_center, int half, unsigned short color)
{
	int cosL = cosine(wrap_angle(a_center - half));
	int sinL = sine(wrap_angle(a_center - half));
	int cosR = cosine(wrap_angle(a_center + half));
	int sinR = sine(wrap_angle(a_center + half));
	int rin2 = rin * rin, rout2 = rout * rout;
	int minx, maxx, miny, maxy, px, py, i;
	int bx[6], by[6];

	/* tight bounding box from the wedge corners and outer-arc midpoint */
	polar_at(cx, cy, rout, a_center - half, &bx[0], &by[0]);
	polar_at(cx, cy, rout, a_center, &bx[1], &by[1]);
	polar_at(cx, cy, rout, a_center + half, &bx[2], &by[2]);
	polar_at(cx, cy, rin, a_center - half, &bx[3], &by[3]);
	polar_at(cx, cy, rin, a_center, &bx[4], &by[4]);
	polar_at(cx, cy, rin, a_center + half, &bx[5], &by[5]);
	minx = maxx = bx[0];
	miny = maxy = by[0];
	for (i = 1; i < 6; i++) {
		if (bx[i] < minx) minx = bx[i];
		if (bx[i] > maxx) maxx = bx[i];
		if (by[i] < miny) miny = by[i];
		if (by[i] > maxy) maxy = by[i];
	}
	minx--; miny--; maxx++; maxy++;
	if (minx < 0) minx = 0;
	if (miny < 0) miny = 0;
	if (maxx >= LCD_XSIZE) maxx = LCD_XSIZE - 1;
	if (maxy >= LCD_YSIZE) maxy = LCD_YSIZE - 1;

	FbColor(color);
	for (py = miny; py <= maxy; py++) {
		for (px = minx; px <= maxx; px++) {
			int dx = px - cx;
			int dy = cy - py;		/* flip to math (up = +) */
			int d2 = dx * dx + dy * dy;

			if (d2 < rin2 || d2 > rout2)
				continue;
			/* inside the sector: CCW of the left ray, CW of the right ray */
			if (cosL * dy - sinL * dx < 0)
				continue;
			if (cosR * dy - sinR * dx > 0)
				continue;
			FbPoint(px, py);
		}
	}
}

static void fill_wedge(int rin, int rout, int a_center, int half, unsigned short color)
{
	wedge_at(LOCK_CX, LOCK_CY, rin, rout, a_center, half, color);
}

static void draw_slot_ticks(int rin, int rout, unsigned short color)
{
	int s;

	FbColor(color);
	for (s = 0; s < N_SLOTS; s++) {
		int x0, y0, x1, y1;
		int a = s * ANGLE_PER_SLOT;

		polar_at(LOCK_CX, LOCK_CY, rin - 1, a, &x0, &y0);
		polar_at(LOCK_CX, LOCK_CY, rout + 2, a, &x1, &y1);
		FbLine(x0, y0, x1, y1);
	}
}

static void draw_prong_glints(int rin, int rout, unsigned short mask,
			      int grow, unsigned short color)
{
	int s;

	FbColor(color);
	for (s = 0; s < N_SLOTS; s++) {
		int x0, y0, x1, y1;
		int a;

		if (!((mask >> s) & 1))
			continue;
		a = s * ANGLE_PER_SLOT;
		polar_at(LOCK_CX, LOCK_CY, rin + 1, a, &x0, &y0);
		polar_at(LOCK_CX, LOCK_CY, rout + grow, a, &x1, &y1);
		FbLine(x0, y0, x1, y1);
	}
}


/* does the selected pick fit the active ring at its current rotation? */
static int selected_pick_fits(void)
{
	return active_ring >= 0 && count_unused_keys() > 0 &&
		!keys[sel_key].used && key_fits_active(&keys[sel_key]);
}

/* true while a selected, unused pick is being held over the active ring */
static int candidate_held(void)
{
	return active_ring >= 0 && count_unused_keys() > 0 && !keys[sel_key].used;
}

static void draw_hub(void)
{
	filled_disc(LOCK_CX, LOCK_CY, HUB_RADIUS, COL_HUB_OUTER);
	filled_disc(LOCK_CX, LOCK_CY, HUB_RADIUS - 3, COL_HUB_INNER);
	FbColor(BLACK);
	FbDDACircle(LOCK_CX, LOCK_CY, 2);
	FbLine(LOCK_CX, LOCK_CY, LOCK_CX, LOCK_CY + 4);
}

static void draw_ring(int r, int sel_fits)
{
	int rin = RING_INNER0 + r * RING_STEP;
	int rout = rin + RING_BAND;
	int solved = (r > active_ring);	/* outer rings get solved first */
	int is_active = (r == active_ring);
	/* the active ring glows brighter blue when the held pick fits it */
	unsigned short track = is_active ? (sel_fits ? COL_FIT_HILITE : COL_TRACK_ACTIVE)
					 : (solved ? COL_TRACK_SOLVED : COL_TRACK_LOCKED);
	unsigned short tooth = solved ? COL_TOOTH_SOLVED
				      : (is_active ? COL_TOOTH_ACTIVE : COL_TOOTH_LOCKED);
	int s;

	/* groove that the teeth sit in */
	FbColor(track);
	FbDDACircle(LOCK_CX, LOCK_CY, rin);
	FbDDACircle(LOCK_CX, LOCK_CY, rout);
	if (is_active)
		draw_slot_ticks(rin, rout, sel_fits ? CYAN : x11_gray40);

	for (s = 0; s < N_SLOTS; s++)
		if ((rings[r] >> s) & 1)
			fill_wedge(rin, rout, s * ANGLE_PER_SLOT, TOOTH_HALF, tooth);
}

/* action feedback: ghost the previous pick position, spark inserts, and
 * briefly show the holes restored by undo. */
static void draw_action_feedback(int anim_p)
{
	int rin, rout, s;

	if (anim_kind == ANIM_NONE || anim_ring < 0)
		return;
	rin = RING_INNER0 + anim_ring * RING_STEP + 1;
	rout = rin + RING_BAND - 2;

	if (anim_kind == ANIM_ROTATE) {
		for (s = 0; s < N_SLOTS; s++)
			if ((anim_mask >> s) & 1)
				fill_wedge(rin, rout, s * ANGLE_PER_SLOT,
					   TOOTH_HALF - 1,
					   anim_p < 120 ? x11_steel_blue : x11_gray40);
	} else if (anim_kind == ANIM_INSERT) {
		int grow = 1 + anim_p / 64;
		unsigned short flash = anim_p < 90 ? WHITE : x11_spring_green;

		for (s = 0; s < N_SLOTS; s++)
			if ((anim_mask >> s) & 1)
				fill_wedge(rin, rout, s * ANGLE_PER_SLOT,
					   TOOTH_HALF, flash);
		draw_prong_glints(rin, rout, anim_mask, grow, WHITE);
	} else if (anim_kind == ANIM_UNDO) {
		for (s = 0; s < N_SLOTS; s++)
			if ((anim_mask >> s) & 1)
				fill_wedge(rin, rout, s * ANGLE_PER_SLOT,
					   TOOTH_HALF - 1,
					   anim_p < 110 ? RED : x11_gray40);
	}
}

/* preview where the selected pick's prongs would land on the active ring */
static void draw_candidate_preview(void)
{
	struct key *k;
	unsigned short m, c;
	int fits, rin, rout, s;

	if (!candidate_held())
		return;
	k = &keys[sel_key];
	m = key_mask(k);
	fits = key_fits_active(k);
	c = fits ? x11_spring_green : RED;
	rin = RING_INNER0 + active_ring * RING_STEP + 1;
	rout = rin + RING_BAND - 2;

	for (s = 0; s < N_SLOTS; s++)
		if ((m >> s) & 1)
			fill_wedge(rin, rout, s * ANGLE_PER_SLOT, TOOTH_HALF - 1, c);
}

static void draw_lock(void)
{
	int r;
	int anim_p = anim_progress255();
	int sel_fits = selected_pick_fits();

	draw_hub();
	for (r = 0; r < num_rings; r++)
		draw_ring(r, sel_fits);
	draw_action_feedback(anim_p);
	draw_candidate_preview();
}

/* a small lock-shaped icon for one pick in the tray.  fits_layer picks turn
 * blue, the Starfield "this key matches the active layer" hint. */
/* double box around the currently selected pick, white-pulsed on a select anim */
static void draw_pick_selection(int cx, int cy, int key_index)
{
	int pulse = (anim_kind == ANIM_SELECT && anim_key == key_index &&
		     anim_progress255() < 140);

	FbColor(pulse ? WHITE : CYAN);
	FbMove(cx - TRAY_ICON_R - 3, cy - TRAY_ICON_R - 3);
	FbRectangle(2 * (TRAY_ICON_R + 3) + 1, 2 * (TRAY_ICON_R + 3) + 1);
	FbColor(CYAN);
	FbMove(cx - TRAY_ICON_R - 1, cy - TRAY_ICON_R - 1);
	FbRectangle(2 * (TRAY_ICON_R + 1) + 1, 2 * (TRAY_ICON_R + 1) + 1);
}

static void draw_pick_icon(int cx, int cy, unsigned short mask, int selected, int fits_layer, int key_index)
{
	unsigned short prong = fits_layer ? COL_FIT_HILITE : COL_TOOTH_LOCKED;
	int s;

	if (selected)
		draw_pick_selection(cx, cy, key_index);

	filled_disc(cx, cy, 2, COL_HUB_INNER);
	FbColor(COL_TRACK_LOCKED);
	FbDDACircle(cx, cy, TRAY_ICON_R);

	for (s = 0; s < N_SLOTS; s++) {
		if (!((mask >> s) & 1))
			continue;
		wedge_at(cx, cy, TRAY_ICON_R - 3, TRAY_ICON_R, s * ANGLE_PER_SLOT, 1, prong);
	}
}

/* the inventory of available picks, scrolled to keep the selected one centered */
static void draw_tray(void)
{
	int avail[MAX_KEYS];
	int n = 0, i, selpos = 0, vcount, start, x0;

	for (i = 0; i < num_keys; i++) {
		if (keys[i].used)
			continue;
		if (i == sel_key)
			selpos = n;
		avail[n++] = i;
	}
	if (n == 0)
		return;

	vcount = n < TRAY_VISIBLE ? n : TRAY_VISIBLE;
	start = selpos - vcount / 2;
	if (start < 0)
		start = 0;
	if (start > n - vcount)
		start = n - vcount;

	x0 = LOCK_CX - ((vcount - 1) * TRAY_SPACING) / 2;
	for (i = 0; i < vcount; i++) {
		int idx = avail[start + i];

		draw_pick_icon(x0 + i * TRAY_SPACING, TRAY_Y, key_mask(&keys[idx]),
			       idx == sel_key, key_fits_active_any_rot(&keys[idx]), idx);
	}

	FbColor(WHITE);
	if (start > 0) {
		FbMove(4, TRAY_Y - 4);
		FbWriteLine("<");
	}
	if (start + vcount < n) {
		FbMove(150, TRAY_Y - 4);
		FbWriteLine(">");
	}
}

static void draw_hud(void)
{
	char line[24];
	int fits = selected_pick_fits();

	FbColor(x11_gray20);
	FbMove(0, 10);
	FbRectangle(LCD_XSIZE, 1);
	FbMove(0, 109);
	FbRectangle(LCD_XSIZE, 1);

	FbColor(WHITE);
	FbMove(2, 1);
	snprintf(line, sizeof(line), "LV%d  PICKS:%d", level, count_unused_keys());
	FbWriteLine(line);

	FbColor(fits ? x11_spring_green : RED);
	FbMove(136, 1);
	FbWriteLine(fits ? "FIT" : "NO");

	FbColor(x11_gray40);
	FbMove(2, 111);
	FbWriteLine("L/R turn");
	FbMove(2, 120);
	FbWriteLine("A set REW undo B out");
}

static void draw_play(void)
{
	if (!screen_changed && !anim_active())
		return;
	FbClear();
	draw_lock();
	draw_tray();
	draw_hud();
	FbSwapBuffers();
	screen_changed = anim_active();
}

/* decorative lock emblem at the top of the splash screen */
static void draw_splash_emblem(void)
{
	filled_disc(LOCK_CX, 31, 18, COL_TRACK_LOCKED);
	FbColor(COL_FIT_HILITE);
	FbDDACircle(LOCK_CX, 31, 18);
	FbDDACircle(LOCK_CX, 31, 12);
	wedge_at(LOCK_CX, 31, 12, 18, 0, 2, COL_TOOTH_ACTIVE);
	wedge_at(LOCK_CX, 31, 12, 18, 32, 2, COL_TOOTH_ACTIVE);
	wedge_at(LOCK_CX, 31, 12, 18, 72, 2, COL_TOOTH_ACTIVE);
	filled_disc(LOCK_CX, 31, 5, COL_HUB_INNER);
	FbColor(BLACK);
	FbLine(LOCK_CX, 31, LOCK_CX, 36);
}

static void draw_splash(void)
{
	if (!screen_changed)
		return;
	FbClear();
	draw_splash_emblem();

	FbColor(CYAN);
	FbMove(28, 2);
	FbWriteLine("DIGIPICK");
	FbColor(WHITE);
	FbMove(6, 52);
	FbWriteString("Pick these\n"
		      "digital locks.\n\n"
		      "L/R turn pick\n"
		      "U/D change pick\n"
		      "A insert  REW undo\n\n"
		      "A to start  B quit");
	FbSwapBuffers();
	screen_changed = 0;
}

static void draw_win_banners(void)
{
	FbPlaceFilledRectangle(0, 0, LCD_XSIZE, 11, BLACK);
	FbColor(x11_spring_green);
	FbMove(44, 2);
	FbWriteLine("UNLOCKED!");

	FbPlaceFilledRectangle(0, 110, LCD_XSIZE, 18, BLACK);
	FbColor(WHITE);
	FbMove(2, 111);
	FbWriteLine("A:next lock");
	FbMove(2, 120);
	FbWriteLine("B:quit");
}

static void draw_win(void)
{
	int p;

	if (!screen_changed && !anim_active())
		return;
	FbClear();
	draw_lock();		/* every ring is solved now, so the lock shows all green */
	p = anim_progress255();

	if (anim_kind == ANIM_WIN) {
		int outer = RING_INNER0 + num_rings * RING_STEP + 2 + p / 42;

		FbColor(p < 170 ? WHITE : x11_spring_green);
		FbDDACircle(LOCK_CX, LOCK_CY, outer);
		FbDDACircle(LOCK_CX, LOCK_CY, outer + 2);
	}

	draw_win_banners();
	FbSwapBuffers();
	screen_changed = anim_active();
}

static void digipick_init(void)
{
	FbInit();
	FbClear();
	rng_state = (unsigned int)rtc_get_ms_since_boot();
	if (rng_state == 0)
		rng_state = 0xa5a5a5a5;
	level = 1;
	anim_kind = ANIM_NONE;
	screen_changed = 1;
	digipick_state = DIGIPICK_SPLASH;
}

static void digipick_splash(void)
{
	int down_latches = button_down_latches();

	draw_splash();
	if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
		digipick_state = DIGIPICK_NEWLEVEL;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
		digipick_state = DIGIPICK_EXIT;
	}
}

static void digipick_newlevel(void)
{
	generate_level();
	screen_changed = 1;
	digipick_state = DIGIPICK_PLAY;
}

static void digipick_play(void)
{
	int down_latches = button_down_latches();

	if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
		digipick_state = DIGIPICK_EXIT;
		return;
	}
	if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down_latches)) {
		/* LEFT turns the pick counterclockwise (slot angle increases) */
		start_anim(ANIM_ROTATE, ANIM_ROTATE_MS, (signed char)active_ring,
			   (signed char)sel_key, key_mask(&keys[sel_key]));
		keys[sel_key].rotation = (unsigned char)((keys[sel_key].rotation + 1) % N_SLOTS);
		screen_changed = 1;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches)) {
		/* RIGHT turns the pick clockwise */
		start_anim(ANIM_ROTATE, ANIM_ROTATE_MS, (signed char)active_ring,
			   (signed char)sel_key, key_mask(&keys[sel_key]));
		keys[sel_key].rotation = (unsigned char)((keys[sel_key].rotation + N_SLOTS - 1) % N_SLOTS);
		screen_changed = 1;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches)) {
		select_step(-1);
		start_anim(ANIM_SELECT, ANIM_SELECT_MS, (signed char)active_ring,
			   (signed char)sel_key, key_mask(&keys[sel_key]));
		screen_changed = 1;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches)) {
		select_step(1);
		start_anim(ANIM_SELECT, ANIM_SELECT_MS, (signed char)active_ring,
			   (signed char)sel_key, key_mask(&keys[sel_key]));
		screen_changed = 1;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
		commit_key();
		screen_changed = 1;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_REWIND, down_latches)) {
		undo_key();
		screen_changed = 1;
	}

	if (active_ring < 0) {
		start_anim(ANIM_WIN, ANIM_WIN_MS, -1, -1, 0);
		screen_changed = 1;
		digipick_state = DIGIPICK_WIN;
		return;
	}

	draw_play();
}

static void digipick_win(void)
{
	int down_latches = button_down_latches();

	draw_win();
	if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
		level++;
		digipick_state = DIGIPICK_NEWLEVEL;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
		digipick_state = DIGIPICK_EXIT;
	}
}

static void digipick_exit(void)
{
	digipick_state = DIGIPICK_INIT; /* so we don't immediately exit next time */
	pop_app();
}

void digipick_cb(struct badge_app *app)
{
	if (app->wake_up) {	/* another app disturbed the screen; force a redraw */
		screen_changed = 1;
		app->wake_up = 0;
	}

	switch (digipick_state) {
	case DIGIPICK_INIT:
		digipick_init();
		break;
	case DIGIPICK_SPLASH:
		digipick_splash();
		break;
	case DIGIPICK_NEWLEVEL:
		digipick_newlevel();
		break;
	case DIGIPICK_PLAY:
		digipick_play();
		break;
	case DIGIPICK_WIN:
		digipick_win();
		break;
	case DIGIPICK_EXIT:
		digipick_exit();
		break;
	default:
		break;
	}
}
