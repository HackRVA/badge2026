#include <stdio.h>
#include <string.h>

#include "colors.h"
#include "menu.h"
#include "button.h"
#include "framebuffer.h"
#include "badge.h"
#include "dynmenu.h"
#include "utils.h"
#include "key_value_storage.h"
#include "audio.h"
#include "music.h"

#define HAVE_MIXING 1

/* Program states.  Initial state is DRUM_MACHINE_INIT */
enum drum_machine_state_t {
	DRUM_MACHINE_INIT,
	DRUM_MACHINE_RUN,
	DRUM_MACHINE_SAVE,
	DRUM_MACHINE_LOAD,
	DRUM_MACHINE_ERROR,
	DRUM_MACHINE_SET_TEMPO,
	DRUM_MACHINE_EXIT,
};

#define MAX_DRUM_PATTERNS 10
#define HITS_PER_MEASURE 16
/* you can't really change NINSTS, as it's 8 because there are 8 bits in a byte */
#define NINSTS 8
struct drum_pattern {
	unsigned char hit[HITS_PER_MEASURE]; /* each bit is 1 instrument */
};

#define MAX_DRUM_PATTERNS_PER_SONG 100
static struct drum_song {
	struct drum_pattern pattern[MAX_DRUM_PATTERNS];
	unsigned char measure[MAX_DRUM_PATTERNS_PER_SONG];
	int nmeasures;
} drum_song = { 0 };

static char drum_machine_err_msg[100];

#define BASS_FREQ 240
#define SNARE_FREQ 5500
#define CRASH_FREQ 5000
#define TOM1_FREQ 440
#define TOM2_FREQ 550
#define OHH_FREQ 3700
#define CHH_FREQ 3800
#define RIDE_FREQ 600

#define BASS_DUR 100 
#define SNARE_DUR 100
#define CRASH_DUR 16
#define TOM1_DUR 16
#define TOM2_DUR 16
#define OHH_DUR 16
#define CHH_DUR 16
#define RIDE_DUR 250

/* 8 channels x 16 notes per measure x max measures, plus 1 for silence at end of section */
static struct audio_out_note drumsong_notes[8 * 16 + 1];
static struct audio_out_section drumtune = {
	.length = 0,
	.notes = drumsong_notes,
};

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
static int copied_pattern = -1;

static enum drum_machine_state_t drum_machine_state = DRUM_MACHINE_INIT;
static int screen_changed = 0;

struct drum_button {
	int x, y;
	char *label;
};

static const struct drum_button_list {
	int nbuttons;
	struct drum_button btn[9];
} pattern_buttons = {
	9,
	{
		{ 10, 100, "PREV" },
		{ 60, 100, "NEXT" },
		{ 110, 100, "COPY" },
		{ 10, 110, "STOP" },
		{ 60, 110, "PLAY" },
		{ 110, 110, "PASTE" },
		{ 10, 120, "TEMPO" },
		{ 60, 120, "SONG" },
		{ 110, 120, "QUIT" },
	},
};
int current_pattern_button = -1;

static const struct drum_button_list song_buttons = {
	6,
	{
		{ 0, 108, "SAVE" },
		{ 50, 108, "LOAD" },
		{ 100, 108, "PATTERN" },
		{ 0, 118, "PLAY" },
		{ 50, 118, "STOP" },
		{ 100, 118, "TEMPO" },
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
	memset(drum_song.measure, 255, sizeof(drum_song.measure));
}

static int add_ride_cymbal_note(int voice, int start_time,
		struct audio_out_section *t, uint16_t freq, uint16_t duration_ms)
{
	int sixteenth_ms = (256 * 60000) / (tempo * 4); /* times 4, because 4 beats per measure */
	if (duration_ms > sixteenth_ms)
		duration_ms = sixteenth_ms;
	if (t->length >= (uint32_t) ARRAY_SIZE(drumsong_notes))
		return 0;

	int i = t->length;

	/* Note that t->notes == &drumsong_notes[0], but we can't access
	 * it through t->notes[] because it's const.
	 */
	drumsong_notes[i].spec.callback = NULL;
	drumsong_notes[i].spec.frequency_hz = freq;
	drumsong_notes[i].spec.duration_ms = duration_ms;
	drumsong_notes[i].spec.envelope = -3;
	drumsong_notes[i].spec.phase = 0;
	drumsong_notes[i].spec.amplitude_dBFS = -3;
	drumsong_notes[i].spec.restart = false;
	drumsong_notes[i].spec.type = AUDIO_OUT_TYPE_SQUARE;
	drumsong_notes[i].spec.square.duty_cycle = 64;
	drumsong_notes[i].ms = start_time;
	drumsong_notes[i].v = voice;
	t->length++;
	return duration_ms;
}

static int add_snare_drum_note(int voice, int start_time,
		struct audio_out_section *t, uint16_t freq, uint16_t duration_ms)
{
	int sixteenth_ms = (256 * 60000) / (tempo * 4); /* times 4, because 4 beats per measure */
	if (duration_ms > sixteenth_ms)
		duration_ms = sixteenth_ms;
	if (t->length >= (uint32_t) ARRAY_SIZE(drumsong_notes))
		return 0;

	int i = t->length;

	/* Note that t->notes == &drumsong_notes[0], but we can't access
	 * it through t->notes[] because it's const.
	 */
	drumsong_notes[i].spec.callback = NULL;
	drumsong_notes[i].spec.frequency_hz = freq;
	drumsong_notes[i].spec.duration_ms = duration_ms;
	drumsong_notes[i].spec.envelope = -5;
	drumsong_notes[i].spec.phase = 0;
	drumsong_notes[i].spec.amplitude_dBFS = -3;
	drumsong_notes[i].spec.restart = false;
	drumsong_notes[i].spec.type = AUDIO_OUT_TYPE_NES_NOISE;
	drumsong_notes[i].spec.square.duty_cycle = 64;
	drumsong_notes[i].ms = start_time;
	drumsong_notes[i].v = voice;
	t->length++;
	return duration_ms;
}

static int add_drum_note(int voice, int start_time,
		struct audio_out_section *t, uint16_t freq, uint16_t duration_ms)
{
	int sixteenth_ms = (256 * 60000) / (tempo * 4); /* times 4, because 4 beats per measure */
	if (duration_ms > sixteenth_ms)
		duration_ms = sixteenth_ms;
	if (t->length >= (uint32_t) ARRAY_SIZE(drumsong_notes))
		return 0;

	int i = t->length;

	/* Note that t->notes == &drumsong_notes[0], but we can't access
	 * it through t->notes[] because it's const.
	 */
	drumsong_notes[i].spec.callback = NULL;
	drumsong_notes[i].spec.frequency_hz = freq;
	drumsong_notes[i].spec.duration_ms = duration_ms;
	drumsong_notes[i].spec.envelope = -1;
	drumsong_notes[i].spec.phase = 0;
	drumsong_notes[i].spec.amplitude_dBFS = -3;
	drumsong_notes[i].spec.restart = false;
	drumsong_notes[i].spec.type = AUDIO_OUT_TYPE_SQUARE;
	drumsong_notes[i].spec.square.duty_cycle = 127;
	drumsong_notes[i].ms = start_time;
	drumsong_notes[i].v = voice;
	t->length++;
	return duration_ms;
}

static int add_silence(int voice, int start_time, struct audio_out_section *t, uint16_t duration_ms)
{
	int sixteenth_ms = (256 * 60000) / (tempo * 4); /* times 4, because 4 beats per measure */
	if (duration_ms > sixteenth_ms)
		duration_ms = sixteenth_ms;
	if (t->length >= (uint32_t) ARRAY_SIZE(drumsong_notes))
		return 0;

	int i = t->length;

	/* Note that t->notes == &drumsong_notes[0], but we can't access
	 * it through t->notes[] because it's const.
	 */
	memset(&drumsong_notes[i], 0, sizeof(drumsong_notes[i]));
	drumsong_notes[i].spec.type = AUDIO_OUT_TYPE_NONE;
	drumsong_notes[i].ms = start_time;
	drumsong_notes[i].spec.duration_ms = duration_ms;
	drumsong_notes[i].v = voice;
	t->length++;
	return duration_ms;
}

static int add_drum_hit(int start_time, struct audio_out_section *t, unsigned char instruments)
{
	int dur = 0;
	if (instruments == 0) {
		dur = add_silence(0, start_time, t, CRASH_DUR);
	} else {
		int d;
		if (instruments & DRUM_CRASH) {
			d = add_drum_note(0, start_time, t, CRASH_FREQ, CRASH_DUR);
			if (d > dur)
				dur = d;
		}
		if (instruments & DRUM_RIDE) {
			d = add_ride_cymbal_note(1, start_time, t, RIDE_FREQ, RIDE_DUR);
			if (d > dur)
				dur = d;
		}
		if (instruments & DRUM_CHH) {
			d = add_drum_note(2, start_time, t, CHH_FREQ, CHH_DUR);
			if (d > dur)
				dur = d;
		}
		if (instruments & DRUM_OHH) {
			d = add_drum_note(3, start_time, t, OHH_FREQ, OHH_DUR);
			if (d > dur)
				dur = d;
		}
		if (instruments & DRUM_TOM2) {
			d = add_drum_note(4, start_time, t, TOM2_FREQ, TOM2_DUR);
			if (d > dur)
				dur = d;
		}
		if (instruments & DRUM_TOM1) {
			d = add_drum_note(5, start_time, t, TOM1_FREQ, TOM1_DUR);
			if (d > dur)
				dur = d;
		}
		if (instruments & DRUM_SNARE) {
			d = add_snare_drum_note(6, start_time, t, SNARE_FREQ, SNARE_DUR);
			if (d > dur)
				dur = d;
		}
		if (instruments & DRUM_BASS) {
			d = add_snare_drum_note(7, start_time, t, BASS_FREQ, BASS_DUR);
			if (d > dur)
				dur = d;
		}
	}
	return dur;
}

static const struct audio_out_section *repeat_current_tune(const struct audio_out_section *prev)
{
	return prev;
}

void start_playing_tune(const struct audio_out_section *tune)
{
	audio_out_music_play(tune, repeat_current_tune);
}

static void play_pattern(int current_pattern)
{
	const int sixteenth_ms = (256 * 60000) / (tempo * 4); /* times 4, because 4 beats per measure */
	int start_time_ms = 0;
	drumtune.length = 0;
	memset(&drumsong_notes, 0, sizeof(drumsong_notes));
	struct drum_pattern *pattern = &drum_song.pattern[current_pattern];
	int t;
	for (int i = 0; i < HITS_PER_MEASURE; i++) {
		t = add_drum_hit(start_time_ms, &drumtune, pattern->hit[i]);
		start_time_ms += sixteenth_ms;
	}
	/* We need to add silence to the end of the section so it doesn't start
	 * replaying the section too soon
	 */
	add_silence(0, start_time_ms - sixteenth_ms + t, &drumtune, sixteenth_ms - t);
	start_playing_tune(&drumtune);
}

static const struct audio_out_section *get_next_measure(const struct audio_out_section *prev);

static void play_one_pattern_of_song(int which_pattern)
{
	const int sixteenth_ms = (256 * 60000) / (tempo * 4); /* times 4, because 4 beats per measure */
	int start_time_ms = 0;
	drumtune.length = 0;
	memset(&drumsong_notes, 0, sizeof(drumsong_notes));
	struct drum_pattern *pattern = &drum_song.pattern[which_pattern];
	int t;
	for (int i = 0; i < HITS_PER_MEASURE; i++) {
		t = add_drum_hit(start_time_ms, &drumtune, pattern->hit[i]);
		start_time_ms += sixteenth_ms;
	}
	/* We need to add silence to the end of the section so it doesn't start
	 * replaying the section too soon
	 */
	add_silence(0, start_time_ms - sixteenth_ms + t, &drumtune, sixteenth_ms - t);
	audio_out_music_play(&drumtune, get_next_measure);
}

static int playing_measure = 0;

static int get_next_measure_number(void)
{
	int next_measure = (playing_measure + 1) % ARRAY_SIZE(drum_song.measure);
	int wrapped = playing_measure;
	int p = drum_song.measure[next_measure];

	while (p == 255) {
		int hit_end_of_array = 0;
		next_measure++;
		if (next_measure >= (int) ARRAY_SIZE(drum_song.measure)) {
			hit_end_of_array = 1;
			next_measure = 0;
		}
		p = drum_song.measure[next_measure];
		if (hit_end_of_array)
			next_measure = -1;
		if (next_measure == wrapped) /* nothing to play */
			return -1;
	}
	return next_measure;
}

static const struct audio_out_section *get_next_measure(__attribute__((unused)) const struct audio_out_section *prev)
{
	playing_measure = get_next_measure_number();
	if (playing_measure < 0)
		return NULL;
	int p = drum_song.measure[playing_measure];

	const int sixteenth_ms = (256 * 60000) / (tempo * 4); /* times 4, because 4 beats per measure */
	int start_time_ms = 0;
	drumtune.length = 0;
	memset(&drumsong_notes, 0, sizeof(drumsong_notes));
	struct drum_pattern *pattern = &drum_song.pattern[p];
	int t;
	for (int i = 0; i < HITS_PER_MEASURE; i++) {
		t = add_drum_hit(start_time_ms, &drumtune, pattern->hit[i]);
		start_time_ms += sixteenth_ms;
	}
	/* We need to add silence to the end of the section so it doesn't start
	 * playing the next section too soon
	 */
	add_silence(0, start_time_ms - sixteenth_ms + t, &drumtune, sixteenth_ms - t);
	return &drumtune;
}

static void play_song(void)
{
	playing_measure = -1;
	playing_measure = get_next_measure_number();
	if (playing_measure == -1)
		return;
	int p = drum_song.measure[playing_measure];
	play_one_pattern_of_song(p);
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
				int v = drum_song.pattern[current_pattern].hit[current_hit] & (1 << current_inst);
				if (v)
					drum_song.pattern[current_pattern].hit[current_hit] &= ~(1 << current_inst);
				else
#if HAVE_MIXING
					drum_song.pattern[current_pattern].hit[current_hit] |= (1 << current_inst);
#else
					drum_song.pattern[current_pattern].hit[current_hit] = (1 << current_inst);
#endif
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
				case 2: /* copy */
					copied_pattern = current_pattern;
					break;
				case 3: /* stop */
					audio_out_music_stop();
					break;
				case 4: /* play */
					play_pattern(current_pattern);
					break;
				case 5: /* paste */
					if (copied_pattern != -1 && current_pattern != copied_pattern) {
						drum_song.pattern[current_pattern] = drum_song.pattern[copied_pattern];
						screen_changed = 1;
					}
					break;
				case 6: /* tempo */
					drum_machine_state = DRUM_MACHINE_SET_TEMPO;
					screen_changed = 1;
					break;
				case 7: /* song */
					drum_mode = song_mode;
					screen_changed = 1;
					break;
				case 8: /* quit */
					drum_machine_state = DRUM_MACHINE_EXIT;
					screen_changed = 1;
					break;
				}
			}
			screen_changed = 1;
		} else if (drum_mode == song_mode) {
			if (current_song_button == -1) {
				if (drum_song.measure[current_measure] == current_pattern)
					drum_song.measure[current_measure] = 255; /* Nothing played this measure */
				else
					drum_song.measure[current_measure] = current_pattern;
			} else {
				switch (current_song_button) {
				case 0: /* save */
					drum_machine_state = DRUM_MACHINE_SAVE;
					screen_changed = 1;
					break;
				case 1: /* load */
					drum_machine_state = DRUM_MACHINE_LOAD;
					screen_changed = 1;
					break;
				case 2: /* pattern */
					drum_mode = pattern_mode;
					screen_changed = 1;
					break;
				case 3: /* play */
					play_song();
					break;
				case 4: /* stop */
					audio_out_music_stop();
					break;
				case 5: /* tempo */
					drum_machine_state = DRUM_MACHINE_SET_TEMPO;
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

static void draw_pattern_screen(int draw_count)
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
			if (drum_song.pattern[current_pattern].hit[j] & (1 << i)) {
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
				if (draw_count & 0x8) { /* make cursor blink */
					FbColor(YELLOW);
					FbMove(x - 2, y - 2);
					FbRectangle(5, 5);
				}
			}
		}
	}
	draw_buttons(&pattern_buttons, current_pattern_button);
}

static void draw_song_screen(int draw_count)
{
	char buffer[20];
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
			if (drum_song.measure[j] == i) {
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
				if (draw_count & 0x8) {
					FbColor(RED);
					FbMove(sx, sy);
					FbRectangle(7, 7);
				}
			}
		}
	}
	draw_buttons(&song_buttons, current_song_button);
}

static void draw_screen(void)
{
	static int draw_count = 0;

	draw_count++;
	if (draw_count > 32)
		draw_count = 0;

	if ((draw_count & 0x8) != ((draw_count + 1) & 0x8))  /* for blinking cursor */
		screen_changed = 1;
	if (!screen_changed)
		return;

	FbColor(WHITE);
	FbBackgroundColor(BLACK);
	FbClear();

	switch (drum_mode) {
	case pattern_mode:
		draw_pattern_screen(draw_count);
		break;
	case song_mode:
		draw_song_screen(draw_count);
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

static struct dynmenu dm_saveload_menu;
static struct dynmenu_item dm_saveload_menu_item[6];

static void drum_machine_setup_saveload_menu(void)
{
	static char menu_setup = 0;

	if (!menu_setup) {
		dynmenu_clear(&dm_saveload_menu);
		dynmenu_init(&dm_saveload_menu, dm_saveload_menu_item, ARRAY_SIZE(dm_saveload_menu_item));
		dynmenu_set_title(&dm_saveload_menu, "CHOOSE STORAGE", "SLOT:", "");
		dynmenu_add_item(&dm_saveload_menu, "SLOT 1", 0, 0);
		dynmenu_add_item(&dm_saveload_menu, "SLOT 2", 1, 1);
		dynmenu_add_item(&dm_saveload_menu, "SLOT 3", 1, 2);
		dynmenu_add_item(&dm_saveload_menu, "SLOT 4", 1, 3);
		dynmenu_add_item(&dm_saveload_menu, "SLOT 5", 1, 4);
		dynmenu_add_item(&dm_saveload_menu, "EXIT", 1, 255);
		menu_setup = 1;
	}
#if 0
	if (!dynmenu_let_user_choose(&board_ship_menu))
		return;

	switch (dynmenu_get_user_choice(&board_ship_menu)) {
	default:
#endif
}

enum dm_save_or_load { dm_save, dm_load };

static void drum_machine_do_save_load(int slot, enum dm_save_or_load action)
{
	char key[20];
	bool ok;

	snprintf(key, sizeof(key), "DRUM_SONG%d", slot);
	if (action == dm_load)
		ok = flash_kv_get_binary(key, &drum_song, sizeof(drum_song));
	else
		ok = flash_kv_store_binary(key, &drum_song, sizeof(drum_song));
	if (!ok) {
		snprintf(drum_machine_err_msg, sizeof(drum_machine_err_msg),
				"ERROR %s\nSONG FROM\nFLASH", action == dm_load ? "LOADING" : "SAVING");
		drum_machine_state = DRUM_MACHINE_ERROR;
		screen_changed = 1;
		return;
	}
	current_pattern = 0;
	current_measure = 0;
	drum_machine_state = DRUM_MACHINE_RUN;
	screen_changed = 1;
	return;
}

static void drum_machine_save_load(enum dm_save_or_load action)
{
	drum_machine_setup_saveload_menu();

	if (!dynmenu_let_user_choose(&dm_saveload_menu))
		return;

	int choice = dynmenu_get_user_choice(&dm_saveload_menu);
	switch (choice) {
	case 255:
		drum_machine_state = DRUM_MACHINE_RUN;
		screen_changed = 1;
		break;
	case 0 ... 4:
		if (action == dm_save)
			drum_machine_do_save_load(choice, dm_save);
		else
			drum_machine_do_save_load(choice, dm_load);
		break;
	}
	return;
}

static int bpm_delta(int tempo)
{
	if (tempo >= 120 * 256)
		return 256;
	if (tempo >= 60 * 256)
		return 128;
	if (tempo >= 30 * 256)
		return 64;
	return 32;
}

static void drum_machine_set_tempo(void)
{
	static int screen_changed = 1;
	char buffer[20];

	if (screen_changed) {
		FbColor(WHITE);
		FbBackgroundColor(BLACK);
		FbClear();
		FbMove(0, 0);
		FbWriteString("SET TEMPO IN BPM\n");
		snprintf(buffer, sizeof(buffer), " %d.%d BPM", tempo / 256, ((tempo & 0x0ff) * 100) / 256);
		FbWriteString(buffer);
		FbSwapBuffers();
		screen_changed = 0;
	}

	int down_latches = button_down_latches();
	if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches)) {
		tempo += bpm_delta(tempo);
		screen_changed = 1;
	}
	if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches)) {
		tempo -= bpm_delta(tempo);
		screen_changed = 1;
	}
	if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches) ||
		BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
		drum_machine_state = DRUM_MACHINE_RUN;
		screen_changed = 1;
	}
	/* 20 seems like a reasonable minimum BPM */
	if (tempo < 20 * 256)
		tempo = 20 * 256;
	/* 256 seems like a reasonable max BPM */
	if (tempo > 255 * 256)
		tempo = 256 * 256;
		
}

static void drum_machine_error(void)
{
	if (screen_changed) {
		FbClear();
		FbColor(WHITE);
		FbBackgroundColor(BLACK);
		FbMove(0, 0);
		FbWriteString(drum_machine_err_msg);
		FbSwapBuffers();
	}

	int down_latches = button_down_latches();

	if (down_latches) {
		drum_machine_state = DRUM_MACHINE_RUN;
		screen_changed = 0;
	}
}

void drum_machine_cb(__attribute__((unused)) struct badge_app *app)
{
	if (app->wake_up) {
		screen_changed = 1;
		app->wake_up = 0;
	}

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
	case DRUM_MACHINE_LOAD:
		drum_machine_save_load(dm_load);
		break;
	case DRUM_MACHINE_SAVE:
		drum_machine_save_load(dm_save);
		break;
	case DRUM_MACHINE_SET_TEMPO:
		drum_machine_set_tempo();
		break;
	case DRUM_MACHINE_ERROR:
		drum_machine_error();
		break;
	default:
		break;
	}
}

