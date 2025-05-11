#ifndef BADGE_H
#define BADGE_H

#include <stdint.h>
#include <stdbool.h>

#define PREPRODUCTION_FIRMWARE 1

#define MAX_APP_STACK_DEPTH 10

#define BADGE_FRAME_RATE_FPS (30) /**< Target frame rate for the badge. */

struct menu_t;

struct badge_app {
	void (*app_func)(struct badge_app *app);
	void *app_context;
	/* menu and current_selection are filled in when app_func() is called from apps/default_menu_app */
	struct menu_t *menu; 
	int current_selection;
	int wake_up;
};

void push_app(struct badge_app app);
void pop_app(void);
void exec_app(struct badge_app app);
void use_carousel_menu_cb(struct badge_app *app);
void use_default_menu_cb(struct badge_app *app);
void use_tape_deck_menu_cb(struct badge_app *app);

/* Apps can call this to know whether the screensaver was active since the
 * app was last called, and if so, know that they must redraw the whole screen.
 */
bool screensaver_was_active(void);

/* Apps can call this when they know they have redrawn their whole screen since
 * the screensaver was last active.
 */
void screensaver_activity_reset(void);

typedef struct {
    char name[16];
    uint64_t badgeId;
    char sekrits[8];
    char achievements[8];

    /*
       prefs
    */
    unsigned char ledBrightness;  /* 1 byte */
    unsigned char backlight;      /* 1 byte */
    bool mute;
    bool display_inverted;
    bool display_rotated;
    bool screensaver_inverted;
    bool screensaver_disabled;
} SYSTEM_DATA;

SYSTEM_DATA* badge_system_data(void);
void UserInit(void);
uint64_t ProcessIO(void);

#endif
