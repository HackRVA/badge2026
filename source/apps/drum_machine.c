#include <stdio.h>
#include <string.h>

#include "colors.h"
#include "menu.h"
#include "button.h"
#include "framebuffer.h"
#include "badge.h"

/* Program states.  Initial state is DRUM_MACHINE_INIT */
enum drum_machine_state_t {
	DRUM_MACHINE_INIT,
	DRUM_MACHINE_RUN,
	DRUM_MACHINE_EXIT,
};

#define MAX_DRUM_PATTERNS 20
#define HITS_PER_MEASURE 16
/* you can't really change NINSTS, as it's 8 because there are 8 bits in a byte */
#define NINSTS 8
static struct drum_pattern {
	unsigned char hit[HITS_PER_MEASURE]; /* each bit is 1 instrument */
} drum_pattern[MAX_DRUM_PATTERNS];

#define MAX_DRUM_PATTERNS_PER_SONG 100
static struct drum_song {
	unsigned char pattern[MAX_DRUM_PATTERNS_PER_SONG];
	int npatterns;
} drum_song = { 0 };

#define DRUM_CRASH (1 << 0)
#define DRUM_RIDE (1 << 1)
#define DRUM_CHH (1 << 2)
#define DRUM_OHH (1 << 3)
#define DRUM_TOM2 (1 << 4)
#define DRUM_TOM1 (1 << 5)
#define DRUM_SNARE (1 << 6)
#define DRUM_BASS (1 << 7)

static const struct drum_name_t {
	const char *short_name;
	const char *long_name;
} drum_name[] = {
	{ "CR", "CRASH", },
	{ "RD", "RIDE", },
	{ "CH", "CLOSED HAT" },
	{ "OH", "OPEN HAT" },
	{ "T2", "TOM 2", },
	{ "T1", "TOM 1", },
	{ "SN", "SNARE", },
	{ "BA", "BASS", },
};

static int tempo = 120 * 256; /* BPM * 256 */
static int current_pattern = 0;
static int current_inst = 7;
static int current_hit = 0;
static int current_measure = 0;
static enum drum_mode { pattern_mode, song_mode } drum_mode = pattern_mode;

static enum drum_machine_state_t drum_machine_state = DRUM_MACHINE_INIT;
static int screen_changed = 0;

struct drum_button {
	int x, y;
	char *label;
};

static const struct drum_button_list {
	int nbuttons;
	struct drum_button btn[6];
} pattern_buttons = {
	6,
	{
		{ 10, 100, "PREV" },
		{ 60, 100, "NEXT" },
		{ 10, 110, "STOP" },
		{ 60, 110, "PLAY" },
		{ 110, 110, "SONG" },
		{ 10, 120, "QUIT" },
	},
};
int current_pattern_button = -1;

static const struct drum_button_list song_buttons = {
	3,
	{
		{ 0, 108, "SAVE" },
		{ 50, 108, "LOAD" },
		{ 100, 108, "PATTERN" },
		{ 0, 0, "" },
		{ 0, 0, "" },
		{ 0, 0, "" },
	},
};
int current_song_button = -1;

static int find_next_button(const struct drum_button_list *button, int x, int y, int dx, int dy, int not_this_button)
{
	int nearest = -1;
	int nd = 0;
	int d;
	for (int i = 0; i < button->nbuttons; i++) {
		if (i == not_this_button)
			continue;
		d = 1000 * 1000;
		if (dx == 0) { /* x must match */
			if (button->btn[i].x == x) {
				d = button->btn[i].y - y;
				if (dy < 0 && d < 0)
					d = d * d;
				else if (dy > 0 && d > 0)
					d = d * d;
				else d = 1000 * 1000;
			} else {
				continue;
			}
		} else if (dy == 0) { /* y must match */
			if (button->btn[i].y == y) {
				d = button->btn[i].x - x;
				if (dx < 0 && d < 0)
					d = d * d;
				else if (dx > 0 && d > 0)
					d = d * d;
				else d = 1000 * 1000;
			} else {
				continue;
			}
		}
		if ((nearest < 0 || d < nd) && i != not_this_button && d != 1000 * 1000) {
			nearest = i;
			nd = d;
		}
	}
	return nearest;
}

static void drum_machine_init(void)
{
	FbInit();
	FbClear();
	drum_machine_state = DRUM_MACHINE_RUN;
	screen_changed = 1;
	memset(drum_song.pattern, 255, sizeof(drum_song.pattern));
}

static void check_buttons(void)
{
    int down_latches = button_down_latches();
	if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down_latches)) {
		if (drum_mode == pattern_mode) {
			if (current_pattern_button == -1) {
				if (current_hit > 0) {
					current_hit--;
				} else if (current_pattern > 0) {
					current_pattern--;
					current_hit = HITS_PER_MEASURE - 1;
				}
			} else {
				int x, y;
				x = pattern_buttons.btn[current_pattern_button].x;
				y = pattern_buttons.btn[current_pattern_button].y;
				int b = find_next_button(&pattern_buttons, x, y, -1, 0, current_pattern_button);
				if (b >= 0)
					current_pattern_button = b;
			}
			screen_changed = 1;
		} else if (drum_mode == song_mode) {
			if (current_song_button == -1) {
				if (current_measure > 0)
					current_measure--;
			} else {
				int x, y;
				x = song_buttons.btn[current_song_button].x;
				y = song_buttons.btn[current_song_button].y;
				int b = find_next_button(&song_buttons, x, y, -1, 0, current_song_button);
				if (b >= 0)
					current_song_button = b;
			}
			screen_changed = 1;
		}
	} else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches)) {
		if (drum_mode == pattern_mode) {
			if (current_pattern_button == -1) {
				if (current_hit < HITS_PER_MEASURE - 1) {
					current_hit++;
				} else if (current_pattern < MAX_DRUM_PATTERNS - 1) {
					current_pattern++;
					current_hit = 0;
				}
			} else {
				int x, y;
				x = pattern_buttons.btn[current_pattern_button].x;
				y = pattern_buttons.btn[current_pattern_button].y;
				int b = find_next_button(&pattern_buttons, x, y, 1, 0, current_pattern_button);
				if (b >= 0)
					current_pattern_button = b;
			}
			screen_changed = 1;
		} else if (drum_mode == song_mode) {
			if (current_song_button == -1) {
				if (current_measure < MAX_DRUM_PATTERNS_PER_SONG - 1)
					current_measure++;
			} else {
				int x, y;
				x = song_buttons.btn[current_song_button].x;
				y = song_buttons.btn[current_song_button].y;
				int b = find_next_button(&song_buttons, x, y, 1, 0, current_song_button);
				if (b >= 0)
					current_song_button = b;
			}
			screen_changed = 1;
		}
	} else if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches)) {
		if (drum_mode == pattern_mode) {
			if (current_pattern_button == -1 && drum_mode == pattern_mode) {
				if (current_inst > 0)
					current_inst--;
			} else {
				if (current_pattern_button != -1) {
					int x, y;
					x = pattern_buttons.btn[current_pattern_button].x;
					y = pattern_buttons.btn[current_pattern_button].y;
					int b = find_next_button(&pattern_buttons, x, y, 0, -1, current_pattern_button);
					if (b >= 0)
						current_pattern_button = b;
					else
						current_pattern_button = -1;
				}
			}
		} else if (drum_mode == song_mode) {
			if (current_song_button == -1) {
				if (current_pattern > 0)
					current_pattern--;
			} else {
				int x, y;
				x = song_buttons.btn[current_song_button].x;
				y = song_buttons.btn[current_song_button].y;
				int b = find_next_button(&song_buttons, x, y, 0, -1, current_song_button);
				if (b >= 0)
					current_song_button = b;
			}
		}
		screen_changed = 1;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches)) {
		if (drum_mode == pattern_mode) {
			if (current_pattern_button == -1 && current_inst < 7) {
				current_inst++;
			} else {
				int x, y;
				if (current_pattern_button == -1) {
					current_pattern_button = 0;
				} else {
					x = pattern_buttons.btn[current_pattern_button].x;
					y = pattern_buttons.btn[current_pattern_button].y;
				}
				int b = find_next_button(&pattern_buttons, x, y, 0, 1, current_pattern_button);
				if (b >= 0)
					current_pattern_button = b;
			}
			screen_changed = 1;
		} else if (drum_mode == song_mode) {
			if (current_song_button == -1) {
				if (current_pattern < MAX_DRUM_PATTERNS - 1)
					current_pattern++;
			} else {
				int x, y;
				x = song_buttons.btn[current_song_button].x;
				y = song_buttons.btn[current_song_button].y;
				int b = find_next_button(&song_buttons, x, y, 0, 1, current_song_button);
				if (b >= 0)
					current_song_button = b;
			}
			screen_changed = 1;
		}
	} else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
		if (drum_mode == pattern_mode) {
			if (current_pattern_button == -1) {
				int v = drum_pattern[current_pattern].hit[current_hit] & (1 << current_inst);
				if (v)
					drum_pattern[current_pattern].hit[current_hit] &= ~(1 << current_inst);
				else
					drum_pattern[current_pattern].hit[current_hit] |= (1 << current_inst);
				screen_changed = 1;
			} else {
				switch (current_pattern_button) {
				case 0: /* prev pattern */
					if (current_pattern > 0)
						current_pattern--;
					break;
				case 1: /* next pattern */
					if (current_pattern < MAX_DRUM_PATTERNS - 1)
						current_pattern++;
					break;
				case 2: /* stop */
					/* TODO: implement this */
					break;
				case 3: /* play */
					/* TODO: implement this */
					break;
				case 4: /* song */
					drum_mode = song_mode;
					screen_changed = 1;
					break;
				case 5: /* quit */
					drum_machine_state = DRUM_MACHINE_EXIT;
					screen_changed = 1;
					break;
				}
			}
			screen_changed = 1;
		} else if (drum_mode == song_mode) {
			if (current_song_button == -1) {
				if (drum_song.pattern[current_measure] == current_pattern)
					drum_song.pattern[current_measure] = 255; /* Nothing played this measure */
				else
					drum_song.pattern[current_measure] = current_pattern;
			} else {
				switch (current_song_button) {
				case 0: /* save */
					/* TODO: implement this */
					break;
				case 1: /* load */
					/* TODO: implement this */
					break;
				case 2: /* pattern */
					drum_mode = pattern_mode;
					screen_changed = 1;
					break;
				}
			}
		}
	} else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
		if (drum_mode == pattern_mode) {
			if (current_pattern_button == -1)
				current_pattern_button = 0;
			else
				current_pattern_button = -1;
			screen_changed = 1;
		} else if (drum_mode == song_mode) {
			if (current_song_button == -1)
				current_song_button = 0;
			else
				current_song_button = -1;
			screen_changed = 1;
		}
	} else if (BUTTON_PRESSED(BADGE_BUTTON_FASTFORWARD, down_latches)) {
		if (drum_mode == pattern_mode) {
			if (current_pattern < MAX_DRUM_PATTERNS - 1) {
				current_pattern++;
				screen_changed = 1;
			}
		}
	} else if (BUTTON_PRESSED(BADGE_BUTTON_REWIND, down_latches)) {
		if (drum_mode == pattern_mode) {
			if (current_pattern > 0) {
				current_pattern--;
				screen_changed = 1;
			}
		}
	}
}

static void draw_buttons(const struct drum_button_list *dbl, int current_button)
{
	FbColor(GREEN);
	for (int i = 0; i < dbl->nbuttons; i++) {
		if (i == current_button) {
			FbColor(BLACK);
			FbBackgroundColor(GREEN);
		} else {
			FbColor(GREEN);
			FbBackgroundColor(BLACK);
		}
		FbMove(dbl->btn[i].x, dbl->btn[i].y);
		FbWriteString(dbl->btn[i].label);
	}
	FbColor(WHITE);
	FbBackgroundColor(BLACK);
}

static void draw_pattern_screen(void)
{
	char buffer[20];
	snprintf(buffer, sizeof(buffer), "PATTERN %d", current_pattern);
	FbMove(0, 0);
	FbWriteString(buffer);
	FbMove(40, 84);
	FbColor(RED);
	FbWriteString(drum_name[current_inst].long_name);
	FbColor(WHITE);
	for (int i = 0; i < 8; i++) {
		int y = 8 * (i + 2);
		if (current_inst == i)
			FbColor(YELLOW);
		else
			FbColor(WHITE);
		FbMove(0, y);
		FbWriteString(drum_name[i].short_name);
		y = y + 4;
		for (int j = 0; j < HITS_PER_MEASURE; j++) {
			int x = 8 * (j + 4);
			if (drum_pattern[current_pattern].hit[j] & (1 << i)) {
				FbColor(WHITE);
				FbMove(x - 3, y - 3);
				FbRectangle(7, 7);
			} else {
				FbColor(WHITE);
				FbPoint(x, y);
				if ((j % 4) == 0)
					FbPoint(x, y - 4);
			}
			if (i == current_inst && j == current_hit && current_pattern_button == -1) {
				FbColor(YELLOW);
				FbMove(x - 2, y - 2);
				FbRectangle(5, 5);
			}
		}
	}
	draw_buttons(&pattern_buttons, current_pattern_button);
}

static void draw_song_screen(void)
{
	char buffer[20];
	FbColor(WHITE);
	FbBackgroundColor(BLACK);
	FbClear();
	int sx, sy;

	sy = 16;
	for (int i = current_pattern - 4; i < current_pattern + 5; i++) {
		sx = 0;
		sy += 8;
		if (i < 0)
			continue;
		if (i > MAX_DRUM_PATTERNS - 1)
			continue;
		if (i == current_pattern)
			FbColor(YELLOW);
		else
			FbColor(WHITE);
		snprintf(buffer, sizeof(buffer), "%02d", i);
		FbMove(sx, sy);
		FbWriteString(buffer);
		sx = 24;
		for (int j = current_measure - 5; j < current_measure + 5; j++) {
			sx += 8;
			if (j < 0)
				continue;
			if (j > 99)
				continue;
			FbColor(WHITE);
			FbPoint(sx + 4, sy + 4);
			if (drum_song.pattern[j] == i) {
				FbMove(sx, sy);
				FbRectangle(7, 7);
			}
			if (j == current_measure && i == current_pattern) {
				snprintf(buffer, sizeof(buffer), "%d", current_measure);
				FbColor(GREEN);
				FbMove(sx, 8);
				FbWriteString(buffer);
				FbMove(8, 8);
				FbWriteString("MEASURE");
				FbColor(YELLOW);
				FbMove(0, 16);
				FbWriteString("PATTERN ");
				snprintf(buffer, sizeof(buffer), "%d", current_pattern);
				FbWriteString(buffer);
				FbColor(RED);
				FbMove(sx, sy);
				FbRectangle(7, 7);
			}
		}
	}
	draw_buttons(&song_buttons, current_song_button);
}

static void draw_screen(void)
{
	if (!screen_changed)
		return;
	FbColor(WHITE);
	FbBackgroundColor(BLACK);

	switch (drum_mode) {
	case pattern_mode:
		draw_pattern_screen();
		break;
	case song_mode:
		draw_song_screen();
		break;
	}

	FbSwapBuffers();
	screen_changed = 0;
}

static void drum_machine_run(void)
{
	check_buttons();
	draw_screen();
}

static void drum_machine_exit(void)
{
	drum_machine_state = DRUM_MACHINE_INIT; /* So that when we start again, we do not immediately exit */
	pop_app();
}

void drum_machine_cb(__attribute__((unused)) struct badge_app *app)
{
	switch (drum_machine_state) {
	case DRUM_MACHINE_INIT:
		drum_machine_init();
		break;
	case DRUM_MACHINE_RUN:
		drum_machine_run();
		break;
	case DRUM_MACHINE_EXIT:
		drum_machine_exit();
		break;
	default:
		break;
	}
}

