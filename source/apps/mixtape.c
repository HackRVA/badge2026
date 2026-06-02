#include <stdbool.h>
#include <stdint.h>

#include "menu.h"
#include "button.h"
#include "framebuffer.h"
#include "mixtape-assets/button_masher.h"
#include "random.h"
#include "ui.h"
#include "xorshift.h"
#include "palette.h"
#include "colors.h"
#include "rtc.h"
#include "audio.h"
#include "utils.h"

#include "mixtape-assets/flippy.h"
#include "mixtape-assets/dr_bad_guy.h"
#include "mixtape-assets/hooper_hero.h"
#include "mixtape-assets/stroodle_doodle.h"
#include "mixtape-assets/nerd_buster.h"
#include "mixtape-assets/coders_digest.h"
#include "mixtape-assets/button_masher.h"

enum mixtape_state_t {
	MIXTAPE_INIT = 0,
	MIXTAPE_RUN,
	MIXTAPE_EXIT,
};
static enum mixtape_state_t mixtape_state = MIXTAPE_INIT;
static struct palette default_palette = {
	.colors =
		{
			PACKRGB888(254, 0, 0),
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

static const uint8_t play_icon[8] = {
	0b00011000,
	0b00011100,
	0b00011110,
	0b00011111,
	0b00011111,
	0b00011110,
	0b00011100,
	0b00011000,
};

static const uint8_t pause_icon[8] = {
	0b11001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b11001100,
};

#define ICON_WIDTH 8
#define ICON_HEIGHT 8
#define BUTTON_WIDTH 30
#define BUTTON_HEIGHT 20

struct track {
	const char *name;
	const struct audio_out_section *tune;
};

static struct track playlist[] = {
	{ "flippy", &FLIPPY },
	{ "stroodle-doodle", &STROODLE_DOODLE },
	{ "nerd-buster", &NERD_BUSTER },
	{ "hooper-hero", &HOOPER_HERO },
	{ "dr-bad-guy", &DR_BAD_GUY },
	{ "coders-digest", &CODERS_DIGEST },
	{ "button-masher", &BUTTON_MASHER },
	/* new_song seems to be broken and needs a better name anyways */
	/* { "new_song", &NEW_SONG }, */
};

#define NUM_TRACKS (ARRAY_SIZE(playlist))

static int   current_track = 0;
static bool  playing       = false;
static uint64_t track_start_time;
static uint32_t track_duration_ms;

static uint32_t section_duration_ms(const struct audio_out_section *s)
{
	uint32_t max_end = 0;
	for (uint32_t i = 0; i < s->length; i++) {
		uint32_t end = s->notes[i].ms + s->notes[i].spec.duration_ms;
		if (end > max_end)
			max_end = end;
	}
	return max_end;
}

static void start_track(void) {
	const struct audio_out_section *s = playlist[current_track].tune;

	track_duration_ms = section_duration_ms(s);
	track_start_time = rtc_get_ms_since_boot();
	playing = true;

	(void) audio_out_music_stop();
	(void) audio_out_music_play(s, NULL);
}

static void stop_track(void) {
	(void) audio_out_music_stop();
	playing = false;
}

static void previous_track(void)
{
	current_track = (current_track + NUM_TRACKS - 1) % NUM_TRACKS;
	start_track();
}
static void next_track(void)
{
	current_track = (current_track + 1) % NUM_TRACKS;
	start_track();
}

static void handle_play(void)
{
	if (playing) stop_track(); else start_track();
}

static void check_buttons(void)
{
	int down_latches = button_down_latches();

	if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches))
		mixtape_state = MIXTAPE_EXIT;
	else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches))
		mixtape_state = MIXTAPE_EXIT;
	else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches))
		mixtape_state = MIXTAPE_EXIT;
	else if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down_latches))
		previous_track();
	else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches))
		next_track();
	else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches))
		mixtape_state = MIXTAPE_EXIT;
	else if (BUTTON_PRESSED(BADGE_BUTTON_RECORD, down_latches))
		mixtape_state = MIXTAPE_EXIT;
	else if (BUTTON_PRESSED(BADGE_BUTTON_PLAY, down_latches))
		handle_play();
	else if (BUTTON_PRESSED(BADGE_BUTTON_FASTFORWARD, down_latches))
		mixtape_state = MIXTAPE_EXIT;
	else if (BUTTON_PRESSED(BADGE_BUTTON_REWIND, down_latches))
		mixtape_state = MIXTAPE_EXIT;
}

static void draw_bitmap(
	const uint8_t *bitmap, int x, int y,  int width, int height)
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

static void draw_screen(void)
{
	FbClear();

	FbMove(ui_center_text_x(playlist[current_track].name, 0, LCD_XSIZE), 10);
	FbWriteString(playlist[current_track].name);

	uint64_t now = rtc_get_ms_since_boot();
	uint32_t elapsed = playing ? (uint32_t)(now - track_start_time) : 0;
	if (elapsed > track_duration_ms) elapsed = track_duration_ms;

	int pct = track_duration_ms ? (elapsed * 100) / track_duration_ms : 0;
	struct ui_progress_bar pb = {
		.x = 10,
		.y = 24,
		.width = LCD_XSIZE - 20,
		.height = 10,
		.outline_size = 1,
		.fill_color = palette_color_from_index(default_palette, 13),
		.empty_color = palette_color_from_index(default_palette, 9),
		.outline_color = palette_color_from_index(default_palette, 7),
		.fill = ui_progress_bar_calculate_fill_percentage(pct),
	};
	ui_progress_bar_draw(pb);

	int spacing = (LCD_XSIZE - 3*BUTTON_WIDTH) / 4;
	struct ui_button btn_prev = {
		.x = spacing,
		.y = LCD_YSIZE - BUTTON_HEIGHT - 5,
		.width = BUTTON_WIDTH,
		.height = BUTTON_HEIGHT,
		.text = "<<",
		.outline_size = 1,
		.outline_color = WHITE,
		.fill_color = BLACK,
		.text_color = WHITE,
	};
	struct ui_button btn_play = {
		.x = spacing * 2 + BUTTON_WIDTH,
		.y = LCD_YSIZE - BUTTON_HEIGHT - 5,
		.width = BUTTON_WIDTH,
		.height = BUTTON_HEIGHT,
		.text = " ",
		.outline_color = WHITE,
		.fill_color = BLACK,
		.text_color = WHITE
	};
	struct ui_button btn_next = {
		.x = spacing * 3 + BUTTON_WIDTH * 2,
		.y = LCD_YSIZE - BUTTON_HEIGHT - 5,
		.width = BUTTON_WIDTH,
		.height = BUTTON_HEIGHT,
		.text = ">>",
		.outline_size = 1,
		.outline_color = WHITE,
		.fill_color = BLACK,
		.text_color = WHITE,
	};
	ui_button_fill(btn_prev, btn_prev.fill_color);
	ui_button_draw_outline(btn_prev, btn_prev.outline_color);
	ui_button_draw_label(btn_prev, btn_prev.text_color);

	ui_button_fill(btn_play, btn_play.fill_color);
	ui_button_draw_outline(btn_play, btn_play.outline_color);

	ui_button_fill(btn_next, btn_next.fill_color);
	ui_button_draw_outline(btn_next, btn_next.outline_color);
	ui_button_draw_label(btn_next, btn_next.text_color);

	int icon_x = btn_play.x + (BUTTON_WIDTH - (playing ? ICON_WIDTH : ICON_WIDTH)) / 2;
	int icon_y = btn_play.y + (BUTTON_HEIGHT - ICON_HEIGHT) / 2;

	if (playing) {
		FbColor(palette_color_from_index(default_palette, 15));
		draw_bitmap(play_icon, icon_x, icon_y, ICON_WIDTH, ICON_HEIGHT);
	} else {
		FbColor(palette_color_from_index(default_palette, 15));
		draw_bitmap(pause_icon, icon_x, icon_y, ICON_WIDTH,  ICON_HEIGHT);
	}

	FbSwapBuffers();
}

static void mixtape_init(void)
{
	FbInit();
	FbClear();

	mixtape_state = MIXTAPE_RUN;
	start_track();
}

void mixtape_cb(struct badge_app *app)
{
	if (app->wake_up)
		app->wake_up = 0;

	switch (mixtape_state) {
	case MIXTAPE_INIT:
		mixtape_init();
		break;
	case MIXTAPE_RUN: {
		check_buttons();
		draw_screen();
		break;
	}
	case MIXTAPE_EXIT:
		stop_track();
		mixtape_state = MIXTAPE_INIT;
		pop_app();
		break;
	default:
		break;
	}
}
