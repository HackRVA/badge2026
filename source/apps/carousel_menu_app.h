#ifndef CAROUSEL_MENU_APP_H__
#define CAROUSEL_MENU_APP_H__
#include "badge.h"
#include "menu.h"

#include "default_menu_app.h"

#if 0
struct default_menu_app_context {
	struct menu_t *menu;
	int top_item;
	int current_item;
	int selected_item;
	int screen_changed;
};

void init_default_menu_app_context(struct default_menu_app_context *c, struct menu_t *menu);
#endif

extern struct badge_app carousel_menu_app;

void carousel_menu_app_cb(struct badge_app *app);

#endif
