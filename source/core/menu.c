/*
   simple menu system
   Author: Paul Bruggeman
   paul@Killercats.com
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef TARGET_SIMULATOR
#include <unistd.h>
#include <signal.h> /* for raise() */
#endif
#include "menu.h"
#include "settings.h"
#include "colors.h"
#include "ir.h"
#include "assetList.h"
#include "button.h"
#include "framebuffer.h"
#include "display.h"
#include "audio.h"
#include "led_pwm.h"
#include "music.h"
#include "stacktrace.h"
#include "key_value_storage.h"

// Apps
#include "about_badge.h"
/* #include "new_badge_monsters/new_badge_monsters.h" */
#include "battlezone.h"
#include "game_of_life.h"
#include "hacking_simulator.h"
#include "lunarlander.h"
// #include "pong.h"
#include "qc.h"
#include "smashout.h"
#include "simonSays.h"
#include "username.h"
#include "slot_machine.h"
// #include "gulag.h"
#include "asteroids.h"
// #include "etch-a-sketch.h"
// #include "magic-8-ball.h"
#include "rvasec_splash.h"
#include "test-screensavers.h"
// #include "tank-vs-tank.h"
#include "clue.h"
#include "moon-patrol.h"
#include "badgey.h"
#include "badge-app-template.h"
#include "aagunner.h"
// #include "rover_adventure.h"
#include "2048.h"
#include "puzzle-attack.h"
#include "microban.h"
#include "drum_machine.h"
#include "badgemon.h"

/* BUILD_IMAGE_TEST_PROGRAM is defined (or not) in top level CMakelists.txt */
#ifdef BUILD_IMAGE_TEST_PROGRAM
#include "image-test.h"
#endif

#define MAIN_MENU_BKG_COLOR GREY2


extern const struct menu_t schedule_m[]; /* defined in core/schedule.c */

static const struct menu_t games_m[] = {
	// {"Sample App", VERT_ITEM, FUNCTION, { .func = myprogram_cb }, NULL },
	// {"Rover Adventure", VERT_ITEM|DEFAULT_ITEM, FUNCTION, { .func = rover_adventure_cb }, NULL, },
	{"Microban", VERT_ITEM | TAPE_DECK, FUNCTION, { .func = microban_cb }, NULL },
	{"badgemon", VERT_ITEM, FUNCTION, { .func = badgemon_cb }, NULL, },
	/* {"Badge Monsters",VERT_ITEM, FUNCTION, { .func = badge_monsters_cb }, NULL, }, */
	{"RVAsec Quest", VERT_ITEM, FUNCTION, { .func = badgey_cb }, NULL, },
	{"Moon Patrol", VERT_ITEM, FUNCTION, { .func = moonpatrol_cb }, NULL, },
	{"AA Gunner", VERT_ITEM, FUNCTION, { .func = aagunner_cb }, NULL, },
	{"Clue", VERT_ITEM, FUNCTION, { .func = clue_cb }, NULL, },
	// {"Badgey", VERT_ITEM, FUNCTION, { .func = badgey_cb }, NULL, },
	{"Asteroids", VERT_ITEM, FUNCTION, { .func = asteroids_cb }, NULL, },
	{"Lunar Rescue",  VERT_ITEM, FUNCTION, { .func = lunarlander_cb}, NULL, },
	{"Battlezone", VERT_ITEM, FUNCTION, { .func = battlezone_cb }, NULL, },
	{"Slot Machine", VERT_ITEM, FUNCTION, { .func = slot_machine_cb }, NULL, },
	{"Smashout",      VERT_ITEM, FUNCTION, { .func = smashout_cb }, NULL, },
	{"Simon Says",      VERT_ITEM, FUNCTION, { .func = simonSays_cb }, NULL, },
	{"Puzzle Attack", VERT_ITEM, FUNCTION, { .func = puzzle_attack_cb }, NULL },
	{"2048", VERT_ITEM, FUNCTION, { .func = twenty_forty_eight_cb }, NULL },
	{"Hacking Sim",   VERT_ITEM, FUNCTION, { .func = hacking_simulator_cb }, NULL, },
	{"Game of Life", VERT_ITEM, FUNCTION, { .func = game_of_life_cb }, NULL, },
	{"Drum Machine", VERT_ITEM, FUNCTION, { .func = drum_machine_cb }, NULL, },
#ifdef BUILD_IMAGE_TEST_PROGRAM
	{"Image Test", VERT_ITEM, FUNCTION, { .func = image_test_cb }, NULL },
#endif
	// {"Etch-a-Sketch", VERT_ITEM, FUNCTION, { .func = etch_a_sketch_cb }, NULL, },
	// {"Magic-8-Ball",     VERT_ITEM, FUNCTION, { .func = magic_8_ball_cb }, NULL, },
	// {"Goodbye Gulag", VERT_ITEM, FUNCTION, { .func = gulag_cb }, NULL, },
	// {"Pong", VERT_ITEM, FUNCTION, { .func = pong_cb }, NULL, },
	// {"Tank vs Tank", VERT_ITEM, FUNCTION, { .func = tank_vs_tank_cb }, NULL, },
	{"Back",         VERT_ITEM|LAST_ITEM, BACK, { NULL }, NULL, },
};

static const struct menu_t settings_m[] = {
   {"Backlight", VERT_ITEM, MENU, { .menu = backlight_m }, NULL, },
   {"LED", VERT_ITEM, MENU, { .menu = LEDlight_m }, NULL, },
   {"Audio", VERT_ITEM|DEFAULT_ITEM, MENU, { .menu = audio_m }, NULL, },
   {"Invert Display", VERT_ITEM, MENU, { .menu = rotate_m, }, NULL, },
   {"User Name", VERT_ITEM, FUNCTION, { .func = username_cb }, NULL, },
   {"Screensaver", VERT_ITEM, MENU, { .menu = screen_lock_m }, NULL, },
   {"ID", VERT_ITEM, MENU, { .menu = myBadgeid_m }, NULL, },
   {"QC",  VERT_ITEM, FUNCTION, { .func = QC_cb }, NULL, },
   {"Clear NVRAM", VERT_ITEM, FUNCTION, { .func = clear_nvram_cb }, NULL, },
   {"Default menu", VERT_ITEM, FUNCTION, { .func = use_default_menu_cb }, NULL },
   {"Tape Deck menu", VERT_ITEM, FUNCTION, { .func = use_tape_deck_menu_cb }, NULL },
   {"Back",         VERT_ITEM|LAST_ITEM, BACK, {NULL}, NULL, },
};

const struct menu_t main_m[] = {
   {"Games",       VERT_ITEM|DEFAULT_ITEM|TAPE_DECK, MENU, { .menu = games_m }, NULL, },
   {"Schedule",    VERT_ITEM, MENU, { .menu = schedule_m }, NULL, },
   {"Settings",    VERT_ITEM, MENU, { .menu = settings_m }, NULL, },
   // {"Test SS",	VERT_ITEM, FUNCTION, { .func = test_screensavers_cb }, NULL, },
   {"About Badge",    VERT_ITEM|LAST_ITEM, FUNCTION, { .func = about_badge_cb }, NULL, },
};

#if TARGET_SIMULATOR

#include <signal.h>

static void check_menu_strings(const struct menu_t *m)
{
	for (int i = 0; ; i++) {
		size_t x = strlen(m[i].name);
		if (x >= sizeof(m[i].name)) {
			fprintf(stderr, "Menu item '%s' is too long (%lu vs %lu)\n", m[i].name, x, sizeof(m[i].name));
			fflush(stderr);
			raise(SIGABRT);
		}
		if (m[i].type == MENU)
			check_menu_strings(m[i].data.menu);
		if (m[i].attrib & LAST_ITEM)
			break;
	}
}

void sanity_check_menu_strings(void)
{
	check_menu_strings(main_m);
}
#endif

