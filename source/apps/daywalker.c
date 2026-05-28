#include "badge.h"
#include "menu.h"

#include "daywalker.h"

void daywalker_cb(struct badge_app *app)
{
	(void) app;
	pop_app();
}
