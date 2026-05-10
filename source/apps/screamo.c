#include "colors.h"
#include "menu.h"
#include "button.h"
#include "framebuffer.h"
#include "badge.h"

/* Program states.  Initial state is SCREAMO_INIT */
enum screamo_state_t {
	SCREAMO_INIT,
	SCREAMO_RUN,
	SCREAMO_EXIT,
};

static enum screamo_state_t screamo_state = SCREAMO_INIT;
static int screen_changed = 0;
static int screamo_counter = 0;

static void screamo_init(void)
{
	FbInit();
	FbClear();
	screamo_state = SCREAMO_RUN;
	screen_changed = 1;
	screamo_counter = 100;
}

static void check_buttons(void)
{
    int down_latches = button_down_latches();
	if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down_latches)) {
	} else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches)) {
	} else if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches)) {
	} else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches)) {
	} else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
		screamo_state = SCREAMO_EXIT;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
		screamo_state = SCREAMO_EXIT;
	}
}

static void draw_screen(void)
{
	char buf[100];
	if (!screen_changed)
		return;
	FbColor(WHITE);
	FbClear();
	FbMove(1, 0);
	if (screamo_counter > 0) {
		FbWriteString("GET READY TO\nSCREAM INTO\nTHE BADGE!\n\nHIGHER VOLUME AND\nHIGHER PITCH\nSCORES HIGHER\n\n");
		snprintf(buf, sizeof(buf), "\nGET READY TO\nSCREAM!\n\n       %d\n", screamo_counter / 20);
		FbWriteString(buf);
		screen_changed = 1;
	} else if (screamo_counter <= 0 && screamo_counter > -100) {
		FbWriteString("\n\n  JUDGING SCREAM\n");
		screen_changed = 1;
	} else if (screamo_counter <= -100 && screamo_counter > -200) {
		FbWriteString("\n\n  PRETTY GOOD\n  SCREAM!\n\n  TRY TO MAKE\n  IT LOUDER AND\n  HIGHER NEXT\n  TIME!\n");
		screen_changed = 1;
	} else {
		screamo_state = SCREAMO_EXIT;
		screamo_counter = 30;
		screen_changed = 1;
	}
	FbSwapBuffers();
}

static void screamo_run(void)
{
	check_buttons();
	draw_screen();
	screamo_counter--;
}

static void screamo_exit(void)
{
	screamo_state = SCREAMO_INIT; /* So that when we start again, we do not immediately exit */
	pop_app();
}

/* You will need to rename screamo_cb() something else. */
void screamo_cb(struct badge_app *app)
{
	if (app->wake_up) {
		screen_changed = 1;
		app->wake_up = 0;
	}

	switch (screamo_state) {
	case SCREAMO_INIT:
		screamo_init();
		break;
	case SCREAMO_RUN:
		screamo_run();
		break;
	case SCREAMO_EXIT:
		screamo_exit();
		break;
	default:
		break;
	}
}

