/*********************************************

4.  In source/core/menu.c:

    - Include your new app header.
    - Add a new entry in the `games_m` menu structure for your app.
    - Optional: To start the simulator running your app (skipping the menus), change the runningApp variable to your
      app callback.

    For example:

menu.c:
...
//Apps
...
#include "myapp.h"
...

...
void (*runningApp)() = app_cb; // Don't commit this change; just for local testing
...

...
const struct menu_t games_m[] = {
   {"Blinkenlights", VERT_ITEM|DEFAULT_ITEM, FUNCTION, { .func = blinkenlights_cb}},
   ...
   {"Sample App",    VERT_ITEM, FUNCTION, { .func = microban_cb}, NULL },
   {"Back",	     VERT_ITEM|LAST_ITEM, BACK, {NULL}, NULL},
};
...

5.  Build the linux program, from the top level of the repository

    # (you only need to run cmake after modifying a CMakeLists.txt file)
    cmake -S . -B build_sim/ -DCMAKE_BUILD_TYPE=Debug -DTARGET=SIMULATOR -G "Unix Makefiles"

    cd build_sim
    make

6.  Delete all these instruction comments from your copy of the file.

7.  Modify the program to make it do what you want.

**********************************************/

#include "colors.h"
#include "menu.h"
#include "button.h"
#include "framebuffer.h"
#include "badge.h"

/* Program states.  Initial state is MICROBAN_INIT */
enum microban_state_t {
	MICROBAN_INIT,
	MICROBAN_RUN,
	MICROBAN_EXIT,
};

static enum microban_state_t microban_state = MICROBAN_INIT;
static int screen_changed = 0;

static void microban_init(void)
{
	FbInit();
	FbClear();
	microban_state = MICROBAN_RUN;
	screen_changed = 1;
}

static void check_buttons(void)
{
    int down_latches = button_down_latches();
	if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down_latches)) {
	} else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches)) {
	} else if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches)) {
	} else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches)) {
	} else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
		microban_state = MICROBAN_EXIT;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
		microban_state = MICROBAN_EXIT;
	}
}

static void draw_screen(void)
{
	if (!screen_changed)
		return;
	FbColor(WHITE);
	FbMove(10, LCD_YSIZE / 2);
	FbWriteLine("HOWDY!");
	FbSwapBuffers();
	screen_changed = 0;
}

static void microban_run(void)
{
	check_buttons();
	draw_screen();
}

static void microban_exit(void)
{
	microban_state = MICROBAN_INIT; /* So that when we start again, we do not immediately exit */
	pop_app();
}

/* You will need to rename microban_cb() something else. */
void microban_cb(__attribute__((unused)) struct badge_app *app)
{
	switch (microban_state) {
	case MICROBAN_INIT:
		microban_init();
		break;
	case MICROBAN_RUN:
		microban_run();
		break;
	case MICROBAN_EXIT:
		microban_exit();
		break;
	default:
		break;
	}
}

