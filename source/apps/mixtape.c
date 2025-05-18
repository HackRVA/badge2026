#include <stdbool.h>
#include <stdint.h>

#include "menu.h"
#include "button.h"
#include "framebuffer.h"
#include "random.h"
#include "ui.h"
#include "xorshift.h"
#include "palette.h"
#include "colors.h"
#include "rtc.h"
#include "audio.h"
#include "music.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static bool screen_changed = false;

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
	struct tune tune;
};

#define whole_note (2000)
#define half_note (whole_note / 2)
#define quarter_note (whole_note / 4)
#define dotted_quarter ((3 * whole_note) / 8)
#define eighth_note (whole_note / 8)
#define sixteenth_note (whole_note / 16)
#define thirtysecond_note (whole_note / 32)

#define kick_drum { NOTE_A3, 10, }
#define snare_drum { NOTE_B6, 10, }

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
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
        { NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_REST, quarter_note, },
  kick_drum,
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_C3, sixteenth_note, },
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
	{ NOTE_C3, sixteenth_note, },
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
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_C3, sixteenth_note, },
		{ NOTE_Ef5, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
		{ NOTE_G5, sixteenth_note, },
  snare_drum,
	{ NOTE_REST, quarter_note, },
  kick_drum,
	{ NOTE_C3, sixteenth_note, },
		{ NOTE_F5, sixteenth_note, },
	{ NOTE_C3, sixteenth_note, },
		{ NOTE_G5, sixteenth_note, },
  snare_drum,
	{ NOTE_Ef3, sixteenth_note, },
		{ NOTE_Bf5, sixteenth_note, },
	{ NOTE_C3, sixteenth_note, },
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
	{ NOTE_C3, sixteenth_note, },
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
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_C3, sixteenth_note, },
		{ NOTE_Ef5, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
		{ NOTE_G5, sixteenth_note, },
  snare_drum,
	{ NOTE_REST, quarter_note, },
  kick_drum,
	{ NOTE_C3, sixteenth_note, },
		{ NOTE_F5, sixteenth_note, },
	{ NOTE_C3, sixteenth_note, },
		{ NOTE_G5, sixteenth_note, },
  snare_drum,
	{ NOTE_Ef3, sixteenth_note, },
		{ NOTE_Bf5, sixteenth_note, },
	{ NOTE_C3, sixteenth_note, },
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
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
        { NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_REST, quarter_note, },
  kick_drum,
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_REST, quarter_note, },
  snare_drum,
	{ NOTE_REST, quarter_note, },
};

static struct note perez_notes[] = {
  kick_drum,
	{ NOTE_C3, eighth_note, },
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_G3, sixteenth_note, },
	{ NOTE_REST, eighth_note, },
  snare_drum,
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_C3, eighth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_Bf3, eighth_note, },
	{ NOTE_Ef4, eighth_note, },
  kick_drum,
	{ NOTE_G3, sixteenth_note, },
	{ NOTE_Bf3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Bf3, sixteenth_note, },
  snare_drum,
	{ NOTE_D4, eighth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Ef4, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_F4, eighth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_C4, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_G4, sixteenth_note, },
	{ NOTE_Ef4, sixteenth_note, },
	{ NOTE_Bf3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
	{ NOTE_C4, thirtysecond_note, },
	{ NOTE_Ef4, thirtysecond_note, },
	{ NOTE_G4, thirtysecond_note, },
	{ NOTE_Bf4, thirtysecond_note, },
	{ NOTE_Bf3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_Bf3, eighth_note, },
	{ NOTE_REST, eighth_note, },
  kick_drum,
	{ NOTE_Ef4, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Ef4, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_G4, sixteenth_note, },
	{ NOTE_Bf4, sixteenth_note, },
	{ NOTE_Ef5, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_D5, eighth_note, },
};

static struct note slop_notes[] = {
  kick_drum,
	{ NOTE_E3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_E3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_G3, eighth_note, },
	{ NOTE_B3, sixteenth_note, },
	{ NOTE_D4, sixteenth_note, },
  kick_drum,
	{ NOTE_E3, sixteenth_note, },
	{ NOTE_E3, sixteenth_note, },
	{ NOTE_E4, eighth_note, },
  snare_drum,
	{ NOTE_A3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_C4, sixteenth_note, },
	{ NOTE_D4, sixteenth_note, },
  kick_drum,
	{ NOTE_E3, eighth_note, },
	{ NOTE_B2, eighth_note, },
  snare_drum,
	{ NOTE_D4, eighth_note, },
	{ NOTE_E4, eighth_note, },
  kick_drum,
	{ NOTE_E3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_G3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_C4, sixteenth_note, },
	{ NOTE_D4, sixteenth_note, },
	{ NOTE_E4, sixteenth_note, },
	{ NOTE_G4, sixteenth_note, },
  kick_drum,
	{ NOTE_B2, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_E3, eighth_note, },
  snare_drum,
	{ NOTE_D4, eighth_note, },
	{ NOTE_E4, eighth_note, },
  kick_drum,
	{ NOTE_E3, sixteenth_note, },
	{ NOTE_G3, sixteenth_note, },
	{ NOTE_B2, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_A3, eighth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_D4, sixteenth_note, },
  kick_drum,
	{ NOTE_E3, sixteenth_note, },
	{ NOTE_E3, sixteenth_note, },
	{ NOTE_G3, eighth_note, },
  snare_drum,
	{ NOTE_B3, sixteenth_note, },
	{ NOTE_D4, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_E4, sixteenth_note, },
  kick_drum,
	{ NOTE_B2, sixteenth_note, },
	{ NOTE_G3, sixteenth_note, },
	{ NOTE_E3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_C4, sixteenth_note, },
	{ NOTE_E4, sixteenth_note, },
	{ NOTE_G4, sixteenth_note, },
	{ NOTE_B4, sixteenth_note, },
  kick_drum,
	{ NOTE_E3, eighth_note, },
	{ NOTE_REST, eighth_note, },
  snare_drum,
	{ NOTE_A3, eighth_note, },
	{ NOTE_E4, eighth_note, },
  kick_drum,
	{ NOTE_E3, sixteenth_note, },
	{ NOTE_G3, sixteenth_note, },
	{ NOTE_E3, sixteenth_note, },
	{ NOTE_G3, sixteenth_note, },
  snare_drum,
	{ NOTE_D4, sixteenth_note, },
	{ NOTE_E4, sixteenth_note, },
	{ NOTE_Gf4, sixteenth_note, },
	{ NOTE_A4, sixteenth_note, },
};

static struct note natures_jazz_notes[] = {
  kick_drum,
		{ NOTE_Bf5, sixteenth_note, },
		{ NOTE_REST, sixteenth_note, },
	{ NOTE_Bf2, sixteenth_note, },
		{ NOTE_Bf5, sixteenth_note, },
  snare_drum,
	{ NOTE_Ef3, eighth_note, },
	{ NOTE_Bf2, sixteenth_note, },
		{ NOTE_Ef6, sixteenth_note, },
  kick_drum,
		{ NOTE_G5, thirtysecond_note, },
		{ NOTE_Bf5, thirtysecond_note, },
		{ NOTE_REST, sixteenth_note, },
	{ NOTE_C3, sixteenth_note, },
		{ NOTE_F5, sixteenth_note, },
  snare_drum,
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_G3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
		{ NOTE_Ef6, sixteenth_note, },
		{ NOTE_C5, sixteenth_note, },
		{ NOTE_Bf5, sixteenth_note, },
		{ NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_C3, sixteenth_note, },
		{ NOTE_C5, sixteenth_note, },
	{ NOTE_F3, eighth_note, },
  kick_drum,
		{ NOTE_Ef6, eighth_note, },
	{ NOTE_C3, eighth_note, },
  snare_drum,
		{ NOTE_F5, eighth_note, },
		{ NOTE_G5, eighth_note, },
  kick_drum,
	{ NOTE_C3, sixteenth_note, },
		{ NOTE_G5, sixteenth_note, },
		{ NOTE_Ef5, sixteenth_note, },
		{ NOTE_Bf4, sixteenth_note, },
  snare_drum,
	{ NOTE_Ef3, eighth_note, },
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
		{ NOTE_F5, thirtysecond_note, },
		{ NOTE_F5, thirtysecond_note, },
		{ NOTE_F5, thirtysecond_note, },
		{ NOTE_F5, thirtysecond_note, },
		{ NOTE_G5, thirtysecond_note, },
		{ NOTE_G5, thirtysecond_note, },
		{ NOTE_G5, thirtysecond_note, },
		{ NOTE_G5, thirtysecond_note, },
  snare_drum,
	{ NOTE_C3, sixteenth_note, },
		{ NOTE_C5, sixteenth_note, },
	{ NOTE_C3, sixteenth_note, },
		{ NOTE_Ef5, sixteenth_note, },
  kick_drum,
	{ NOTE_Ef3, sixteenth_note, },
		{ NOTE_G5, sixteenth_note, },
	{ NOTE_Bf2, eighth_note, },
  snare_drum,
		{ NOTE_Bf5, sixteenth_note, },
		{ NOTE_REST, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
		{ NOTE_C6, sixteenth_note, },
  kick_drum,
	{ NOTE_C3, sixteenth_note, },
		{ NOTE_Ef6, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
		{ NOTE_F6, sixteenth_note, },
  snare_drum,
		{ NOTE_G6, sixteenth_note, },
		{ NOTE_Bf5, sixteenth_note, },
};

static struct note tape_eater_notes[] = {
  kick_drum,
		{ NOTE_REST, eighth_note, },
	{ NOTE_Bf2, sixteenth_note, },
		{ NOTE_Ef6, sixteenth_note, },
  snare_drum,
	{ NOTE_G3, eighth_note, },
	{ NOTE_Ef3, sixteenth_note, },
		{ NOTE_Ef6, sixteenth_note, },
  kick_drum,
	{ NOTE_REST, sixteenth_note, },
		{ NOTE_C6, sixteenth_note, },
	{ NOTE_F3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  snare_drum,
	{ NOTE_C3, sixteenth_note, },
		{ NOTE_Ef5, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
  kick_drum,
		{ NOTE_F5, thirtysecond_note, },
		{ NOTE_G5, thirtysecond_note, },
		{ NOTE_F5, sixteenth_note, },
		{ NOTE_Ef5, sixteenth_note, },
		{ NOTE_C5, sixteenth_note, },
  snare_drum,
	{ NOTE_Ef3, sixteenth_note, },
		{ NOTE_F4, sixteenth_note, },
	{ NOTE_F3, eighth_note, },
  kick_drum,
		{ NOTE_C5, sixteenth_note, },
		{ NOTE_F5, sixteenth_note, },
	{ NOTE_C3, eighth_note, },
  snare_drum,
		{ NOTE_Bf5, sixteenth_note, },
		{ NOTE_REST, sixteenth_note, },
		{ NOTE_C6, sixteenth_note, },
		{ NOTE_F5, sixteenth_note, },
  kick_drum,

	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Ef3, eighth_note, },
  snare_drum,
		{ NOTE_F5, thirtysecond_note, },
		{ NOTE_G5, thirtysecond_note, },
		{ NOTE_F5, thirtysecond_note, },
		{ NOTE_G5, thirtysecond_note, },
	{ NOTE_C3, sixteenth_note, },
		{ NOTE_G5, sixteenth_note, },
  kick_drum,
		{ NOTE_F5, sixteenth_note, },
		{ NOTE_Ef5, sixteenth_note, },
		{ NOTE_C5, sixteenth_note, },
		{ NOTE_Bf4, sixteenth_note, },
  snare_drum,
	{ NOTE_C3, sixteenth_note, },
		{ NOTE_Bf4, sixteenth_note, },
	{ NOTE_F3, sixteenth_note, },
		{ NOTE_Bf4, sixteenth_note, },
  kick_drum,
	{ NOTE_Ef3, sixteenth_note, },
		{ NOTE_Bf4, sixteenth_note, },
	{ NOTE_Bf2, eighth_note, },
  snare_drum,
		{ NOTE_G4, thirtysecond_note, },
		{ NOTE_Bf4, thirtysecond_note, },
		{ NOTE_REST, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
		{ NOTE_Bf4, sixteenth_note, },
  kick_drum,
	{ NOTE_C3, sixteenth_note, },
	{ NOTE_REST, sixteenth_note, },
	{ NOTE_Ef3, sixteenth_note, },
		{ NOTE_C5, sixteenth_note, },
  snare_drum,
	{ NOTE_Bf2, eighth_note, },
		{ NOTE_F5, thirtysecond_note, },
		{ NOTE_G5, thirtysecond_note, },
		{ NOTE_Bf5, thirtysecond_note, },
		{ NOTE_C6, thirtysecond_note, },
};

static struct track playlist[] = {
	{ "nature's jazz",   { .num_notes = ARRAY_SIZE(natures_jazz_notes), .note = natures_jazz_notes} },
	{ "tape eater",   { .num_notes = ARRAY_SIZE(tape_eater_notes), .note = tape_eater_notes} },
	{ "puzzle-attack",   { .num_notes = ARRAY_SIZE(puzzle_attack_theme_notes), .note = puzzle_attack_theme_notes } },
	{ "perez jumper",   { .num_notes = ARRAY_SIZE(perez_notes), .note = perez_notes} },
	{ "slop",   { .num_notes = ARRAY_SIZE(slop_notes), .note = slop_notes} },
};

#define NUM_TRACKS (ARRAY_SIZE(playlist))

static int   current_track = 0;
static bool  playing       = false;
static uint64_t track_start_time;
static int   theme_index;
static uint64_t theme_note_start;
static int   track_duration_ms;

static void start_track(void) {
	struct tune *t = &playlist[current_track].tune;

	track_duration_ms = 0;
	for (size_t i = 0; i < (size_t)t->num_notes; i++)
		track_duration_ms += t->note[i].duration;

	theme_index      = 0;
	theme_note_start = rtc_get_ms_since_boot();
	track_start_time = theme_note_start;
	playing = true;
	audio_out_beep(t->note[0].freq, t->note[0].duration);
}

static void stop_track(void) {
    playing = false;
}

static void update_audio(uint64_t now) {
	if (!playing) return;

	struct tune *t = &playlist[current_track].tune;
	const struct note *n = &t->note[theme_index];
	if (now < theme_note_start + n->duration) return;

	theme_index = (theme_index + 1) % t->num_notes;
	theme_note_start = now;

	if (theme_index == 0) {
		track_start_time = now;
	}

	const struct note *next = &t->note[theme_index];
	audio_out_beep(next->freq, next->duration);
}

static void previous_track(void)
{
	current_track = (current_track + NUM_TRACKS - 1) % NUM_TRACKS;
	start_track();
	screen_changed = 1;
}
static void next_track(void)
{
	current_track = (current_track + 1) % NUM_TRACKS;
	start_track();
	screen_changed = 1;
}

static void handle_play(void)
{
	if (playing) stop_track(); else start_track();
	screen_changed = 1;
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
	else if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches))
		mixtape_state = MIXTAPE_EXIT;
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

	return;
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
	uint32_t elapsed = playing
		? now - track_start_time
		: theme_note_start - track_start_time;
	if (elapsed > (uint32_t)track_duration_ms) elapsed = track_duration_ms;

	int pct = (elapsed * 100) / track_duration_ms;
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

	/* palette_draw_grid(default_palette,0, 32, 8); */
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
	screen_changed = false;
}

static void mixtape_init(void)
{
	FbInit();
	FbClear();

	mixtape_state = MIXTAPE_RUN;
	start_track();
	screen_changed = 1;
}

void mixtape_cb(__attribute__((unused)) struct menu_t *m)
{
	screen_changed = true;
	switch (mixtape_state) {
	case MIXTAPE_INIT:
		mixtape_init();
		break;
	case MIXTAPE_RUN: {
		check_buttons();
		update_audio(rtc_get_ms_since_boot());
		if (screen_changed)
			draw_screen();
		break;
	}
	case MIXTAPE_EXIT:
		mixtape_state = MIXTAPE_INIT;
		pop_app();
		break;
	default:
		break;
	}
}
