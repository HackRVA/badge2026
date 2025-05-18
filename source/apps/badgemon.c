#include <stdbool.h>
#include <stdio.h>

#include "rtc.h"
#include "menu.h"
#include "button.h"
#include "framebuffer.h"
#include "random.h"
#include "ui.h"
#include "xorshift.h"
#include "palette.h"
#include "colors.h"
#include "2024-badge-monsters/2024-badge-monsters.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define MAX_SPARKLES 16
#define SPARKLE_INTERVAL_MS 250
#define SPARKLE_LIFETIME_MS 400

static bool screen_changed = false;
static bool show_info = false;
static bool show_description = false;
static int current_monster = 0;

enum badgemon_state_t {
	BADGEMON_INIT = 0,
	BADGEMON_RUN,
	BADGEMON_EXIT,
};
static enum badgemon_state_t badgemon_state = BADGEMON_INIT;
static struct palette default_palette = {
	.colors =
		{
			PACKRGB888(0, 0, 0),
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

struct sparkle {
	int x, y;
	uint64_t birth_ms;
	bool alive;
};
static struct sparkle sparkles[MAX_SPARKLES];
static uint64_t sparkle_cooldown = 0;
static unsigned int sparkle_state    = 2463534242;

struct monster {
	char *name;
	char *description;
	bool unlocked;
	bool shiny;
	const struct asset2 *image;
};

#define NUM_MONSTERS 11 
static const struct monster monsters[] = {
  {.name = "mircrabanx", .description = "A biotech or nano-themed creature", .unlocked = true, .shiny = false, .image = &bm1},
  {.name = "rvasekor", .description = "a mythical, security-themed being", .unlocked = true, .shiny = true, .image = &bm2},
  {.name = "lunatrox", .description = "a galactic scout", .unlocked = true, .shiny = true, .image = &bm3},
  {.name = "gunneraam", .description = "heavily armored turret monster", .unlocked = true, .shiny = false, .image = &bm3},
  {.name = "astrolith", .description = "Rock-type, possibly a meteor creature", .unlocked = true, .shiny = false, .image = &bm4},
  {.name = "lunarex", .description = "A noble, moon-themed savior creature", .unlocked = true, .shiny = false, .image = &bm5},
  {.name = "zonetrax", .description = "A tank-like war monster", .unlocked = true, .shiny = false, .image = &bm6},
  {.name = "jackpanda", .description = "A lucky, coin-flipping creature", .unlocked = true, .shiny = true, .image = &bm7},
  {.name = "smashikong", .description = "Heavy-hitting brawler, ape-like", .unlocked = true, .shiny = false, .image = &bm8},
  {.name = "puzzlith", .description = "a psychic fairy", .unlocked = true, .shiny = false, .image = &bm9},
  {.name = "numbrion", .description = "evolves every 1024 levels", .unlocked = true, .shiny = false, .image = &bm10},
  {.name = "hacktrix", .description = "A virtual-type infiltrator", .unlocked = true, .shiny = false, .image = &bm11},
  {.name = "celluna", .description = "Cellular, evolving creature with logic-based moves", .unlocked = true, .shiny = false, .image = &bm12},
  {.name = "beatdramon", .description = "rhythm", .unlocked = true, .shiny = false, .image = &bm13},
};

static void check_buttons(void)
{
	int down_latches = button_down_latches();

	if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches)) {
		current_monster = (current_monster + NUM_MONSTERS - 1) % NUM_MONSTERS;
	}
	else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches)) {
		current_monster = (current_monster + 1) % NUM_MONSTERS;
	}
	else if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down_latches)) {
		show_info = !show_info;     
		show_description = false;
	}
	else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches)) {
		show_info = false;     
		show_description = !show_description;
	}
	else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches))
		badgemon_state = BADGEMON_EXIT;
	else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches))
		badgemon_state = BADGEMON_EXIT;

	return;
}

static void spawn_sparkles(uint64_t now, int x, int y, int w, int h) {
	for (int s = 0; s < 3; s++) {
		for (int i = 0; i < MAX_SPARKLES; i++) {
			if (!sparkles[i].alive) {
				sparkles[i].alive = true;
				sparkles[i].birth_ms = now;
				uint32_t r1 = xorshift(&sparkle_state);
				uint32_t r2 = xorshift(&sparkle_state);
				sparkles[i].x = x + (r1 % w);
				sparkles[i].y = y + (r2 % h);
				break;
			}
		}
	}
}

static void draw_sparkles(uint64_t now) {
	for (int i = 0; i < MAX_SPARKLES; i++) {
		if (!sparkles[i].alive) continue;
			uint64_t age = now - sparkles[i].birth_ms;
			if (age > SPARKLE_LIFETIME_MS) {
				sparkles[i].alive = false;
				continue;
			}
			FbColor(palette_color_from_index(default_palette, i));
			if (i % 2 == 0)
				FbColor(WHITE);
			int x = sparkles[i].x, y = sparkles[i].y;
			FbPoint(x, y);
			FbPoint(x - 1, y);
			FbPoint(x + 1, y);
			FbPoint(x, y - 1);
			FbPoint(x, y + 1);
	}
}


static void draw_monster_avatar(const struct asset2 *img)
{
	int x = 16;
	int y = 4;

	FbMove(x, y);
	FbImage4bit2(img, 0);
}

static void draw_monster_avatar_screen(void)
{
	draw_monster_avatar(monsters[current_monster].image);

	int width = 140;
	int height = 64;
	int outline_size = 3;
	int margin = 2;
	int y_offset = 16;

	struct ui_text_box info_box = {
		.x = (LCD_XSIZE/2) - (width/2),
		.y = LCD_YSIZE - height - outline_size - margin - y_offset,
		.width = width,
		.height = height,
		.text = monsters[current_monster].name,
		.outline_size = outline_size,
		.outline_color = palette_color_from_index(default_palette, 13),
		.fill_color = palette_color_from_index(default_palette, 0),
		.text_color = palette_color_from_index(default_palette, 11),
	};

	if (show_info) {
		ui_text_box_draw(info_box);
	}

	if (show_description) {
		info_box.text = monsters[current_monster].description;
		ui_text_box_draw(info_box);
	}

	char *controls_text = "<back |down| desc>";
	FbMove(ui_center_text_x(controls_text, 0, LCD_XSIZE), 120);
	FbColor(WHITE);
	FbWriteLine(controls_text);

	if (true) {
		FbMove(100, 0);
		FbColor(YELLOW);
		FbWriteString("starter");
	}
	if (monsters[current_monster].shiny) {
		FbMove(100, 8);
		FbColor(WHITE);
		FbWriteString("*shiny");
	}
}

static void draw_screen(void)
{
	/* palette_draw_grid(default_palette,0, 0, 8); */
	if (!screen_changed)
		return;

	FbSwapBuffers();
	screen_changed = false;
}

static void badgemon_init(void)
{
	FbInit();
	FbClear();
	/* load in badge monsters */
	badgemon_state = BADGEMON_RUN;
	sparkle_cooldown = rtc_get_ms_since_boot();
}

static void badgemon_update(void)
{
	uint64_t now = rtc_get_ms_since_boot();

	check_buttons();
	draw_monster_avatar_screen();

	if (monsters[current_monster].shiny) {
		if (now >= sparkle_cooldown) {
			spawn_sparkles(now, 16, 4, LCD_XSIZE, LCD_YSIZE);
			sparkle_cooldown = now + SPARKLE_INTERVAL_MS;
		}
		draw_sparkles(now);
	}

	draw_screen();
}
static void badgemon_exit(void)
{
	badgemon_state = BADGEMON_INIT;
	pop_app();
}

void badgemon_cb(__attribute__((unused)) struct menu_t *m)
{
	screen_changed = true;
	switch (badgemon_state) {
	case BADGEMON_INIT:
		badgemon_init();
		break;
	case BADGEMON_RUN:
		badgemon_update();
		break;
	case BADGEMON_EXIT:
		badgemon_exit();
		break;
	}
}
