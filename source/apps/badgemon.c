#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "rtc.h"
#include "button.h"
#include "framebuffer.h"
#include "random.h"
#include "ui.h"
#include "xorshift.h"
#include "palette.h"
#include "colors.h"
#include "key_value_storage.h"
#include "new_badge_monsters_ir.h"
#include "audio.h"
#include "badge.h"

#define DEFINE_IMAGE_ASSET_DATA
#include "badgemon_asset.h"
#undef DEFINE_IMAGE_ASSET_DATA

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define NUM_MENU_ITEMS ARRAY_SIZE(menu_items)
#define MAX_SPARKLES 16
#define SPARKLE_INTERVAL_MS 250
#define SPARKLE_LIFETIME_MS 400
#define MENU_ITEM_SPACING 30
#define MENU_ITEM_WIDTH 120
#define MENU_ITEM_HEIGHT 20
#define MENU_X (LCD_XSIZE/2 - MENU_ITEM_WIDTH/2)
#define MENU_Y (LCD_YSIZE/2 - MENU_ITEM_HEIGHT/2)

enum badgemon_state_t {
	BADGEMON_INIT = 0,
	BADGEMON_MONSTER_AVATAR,
	BADGEMON_PROGRESS,
	BADGEMON_TRADE_MONSTERS,
	BADGEMON_TRADE_MONSTERS_DELAY,
	BADGEMON_HELP_SCREEN,
	BADGEMON_TOP_MENU,
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
static bool screen_changed = false;
static bool initial_run = true;
static bool scan_animating = false;
static int  current_menu_item = 0;
static bool current_menu_item_selected = false;
static uint8_t scanline_animation_y = 0;
static unsigned int initial_mon;

struct sparkle {
	int x, y;
	uint64_t birth_ms;
	bool alive;
};
static struct sparkle sparkles[MAX_SPARKLES];
static uint64_t sparkle_cooldown = 0;
static unsigned sparkle_state = 2463534242;

struct monster {
	char *name;
	char *description;
	bool unlocked;
	bool shiny;
	const struct asset2 *image;
};

static struct monster monsters[] = {
  {.name = "birdo", .description = "A biotech or nano-themed creature", .shiny = false, .image = &badge_monster_birdo},
  {.name = "bowser", .description = "a mythical, security-themed being", .shiny = true, .image = &badge_monster_bowser},
  {.name = "jason voorhees", .description = "a galactic scout", .shiny = true, .image = &badge_monster_jason_voorhees},
  {.name = "crawler", .description = "heavily armored turret monster", .shiny = false, .image = &badge_monster_crawler},
  {.name = "donkey kong", .description = "Rock-type, possibly a meteor creature", .shiny = false, .image = &badge_monster_donkey_kong},
  {.name = "ghosts", .description = "A noble, moon-themed savior creature", .shiny = false, .image = &badge_monster_ghosts},
  {.name = "harpy", .description = "A tank-like war monster", .shiny = false, .image = &badge_monster_harpy},
  {.name = "medusa", .description = "A lucky, coin-flipping creature", .shiny = true, .image = &badge_monster_medusa_head},
  {.name = "nettler", .description = "Heavy-hitting brawler, ape-like", .shiny = false, .image = &badge_monster_nettler},
  {.name = "odd eye", .description = "a psychic fairy", .shiny = false, .image = &badge_monster_odd_eye},
  {.name = "shredder", .description = "evolves every 1024 levels", .shiny = false, .image = &badge_monster_shredder},
  {.name = "slime", .description = "A virtual-type infiltrator", .shiny = false, .image = &badge_monster_slime},
  {.name = "metall", .description = "Cellular, evolving creature with logic-based moves", .shiny = false, .image = &badge_monster_metall},
  {.name = "moblin", .description = "Cellular, evolving creature with logic-based moves", .shiny = false, .image = &badge_monster_moblin},
  {.name = "mother brain", .description = "Cellular, evolving creature with logic-based moves", .shiny = false, .image = &badge_monster_mother_brain},
  {.name = "stay puff", .description = "Cellular, evolving creature with logic-based moves", .shiny = false, .image = &badge_monster_stay_puff},

  {.name = "beetlejuice", .description = "Cellular, evolving creature with logic-based moves", .shiny = false, .image = &badge_monster_beetlejuice},
  {.name = "chet", .description = "Cellular, evolving creature with logic-based moves", .shiny = false, .image = &badge_monster_chet},
  {.name = "chucky", .description = "Cellular, evolving creature with logic-based moves", .shiny = false, .image = &badge_monster_chucky},
  {.name = "drago", .description = "Cellular, evolving creature with logic-based moves", .shiny = false, .image = &badge_monster_drago},
  {.name = "ed rooney", .description = "Cellular, evolving creature with logic-based moves", .shiny = false, .image = &badge_monster_ed_rooney},
  {.name = "freddy krueger", .description = "Cellular, evolving creature with logic-based moves", .shiny = false, .image = &badge_monster_freddy_krueger},
  {.name = "gopher", .description = "Cellular, evolving creature with logic-based moves", .shiny = false, .image = &badge_monster_gopher_caddyshack},
  {.name = "gremlin", .description = "Cellular, evolving creature with logic-based moves", .shiny = false, .image = &badge_monster_gremlin},
  {.name = "hans gruber", .description = "Cellular, evolving creature with logic-based moves", .shiny = false, .image = &badge_monster_hans_gruber},
  {.name = "jack torrance", .description = "Cellular, evolving creature with logic-based moves", .shiny = false, .image = &badge_monster_jack_torrance},
  {.name = "jason", .description = "Cellular, evolving creature with logic-based moves", .shiny = false, .image = &badge_monster_jason},
  {.name = "joker", .description = "Cellular, evolving creature with logic-based moves", .shiny = false, .image = &badge_monster_joker},
  {.name = "khan", .description = "Cellular, evolving creature with logic-based moves", .shiny = false, .image = &badge_monster_khan},
  {.name = "richard vernon", .description = "Cellular, evolving creature with logic-based moves", .shiny = false, .image = &badge_monster_richard_vernon},
  {.name = "skeletor", .description = "Cellular, evolving creature with logic-based moves", .shiny = false, .image = &badge_monster_skeltor},
  {.name = "stay puft", .description = "Cellular, evolving creature with logic-based moves", .shiny = false, .image = &badge_monster_stay_puft},
};

static char kvbuf[32];
static const char *flash_key_from_monster(int id)
{
	snprintf(kvbuf, sizeof(kvbuf), "monster/%s", monsters[id].name);
	return kvbuf;
}

static void load_monsters_from_flash(void)
{
	for (int i = 0; i < (int)ARRAY_SIZE(monsters); i++) {
		int val = 0;
		flash_kv_get_int(flash_key_from_monster(i), &val);
		monsters[i].unlocked = (val != 0);
	}
}

static void save_monsters_to_flash(void)
{
	for (int i = 0; i < (int)ARRAY_SIZE(monsters); i++)
		flash_kv_store_int(flash_key_from_monster(i), monsters[i].unlocked ? 1 : 0);
}

static void spawn_sparkles(uint64_t now, int x, int y, int w, int h)
{
	for (int s = 0; s < 3; s++) {
		for (int i = 0; i < MAX_SPARKLES; i++) {
			if (!sparkles[i].alive) {
				sparkles[i].alive    = true;
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

static void draw_sparkles(uint64_t now)
{
	for (int i = 0; i < MAX_SPARKLES; i++) {
		if (!sparkles[i].alive) continue;
		uint64_t age = now - sparkles[i].birth_ms;
		if (age > SPARKLE_LIFETIME_MS) {
			sparkles[i].alive = false;
			continue;
		}
		FbColor(WHITE);
		int x = sparkles[i].x, y = sparkles[i].y;
		FbPoint(x, y);
		FbPoint(x-1, y);
		FbPoint(x+1, y);
		FbPoint(x, y-1);
		FbPoint(x, y+1);
	}
}

static bool show_description = false;
static unsigned int current_monster_id = 0;

static void draw_monster_avatar(const struct asset2 *img)
{
	FbMove(LCD_XSIZE/2 - 56, LCD_YSIZE/2 - 56);
	FbImage4bit2(img, 0);
}

#define MOSAIC_INTERVAL_MS 500
static uint64_t mosaic_cooldown = 0;
static uint32_t mosaic_seed = 2463522242;
static void draw_monster_avatar_screen_locked(uint64_t now)
{
	int width = 150;
	int height = 40;
	struct ui_button locked_label = {
		.x = LCD_XSIZE/2 - width/2,
		.y = LCD_YSIZE/2 - height/2,
		.width = width,
		.height = height,
		.text = "? keep searching",
		.outline_size = 3,
		.outline_color = palette_color_from_index(default_palette, current_monster_id % 16),
		.fill_color = BLACK,
		.text_color = palette_color_from_index(default_palette, 11),
	};


	if (now >= mosaic_cooldown) {
		mosaic_cooldown = now + MOSAIC_INTERVAL_MS;
		mosaic_seed = (uint32_t)(now ^ current_monster_id);
		screen_changed = true;
	}

	uint32_t state = mosaic_seed;
	const int block = 4;
	for (int y = 0; y < LCD_YSIZE; y += block) {
		for (int x = 0; x < LCD_XSIZE; x += block) {
			uint8_t idx = xorshift(&state) % 16;
			uint16_t col = palette_color_from_index(default_palette, idx);
			FbColor(col);

			unsigned char w = (x + block <= LCD_XSIZE) ? block : (LCD_XSIZE - x);
			unsigned char h = (y + block <= LCD_YSIZE) ? block : (LCD_YSIZE - y);

			FbMove(x, y);
			FbFilledRectangle(w, h);
		}
	}

	ui_button_draw(locked_label);
	char idbuf[8];
	snprintf(idbuf, sizeof(idbuf), "#%03u", current_monster_id + 1);
	FbMove(locked_label.x + 4, locked_label.y + 4);
	FbWriteString(idbuf);

	char *lower_control_info = "|Down|";
	FbColor(WHITE);
	FbMove(ui_center_text_x(lower_control_info, 0, LCD_XSIZE), LCD_YSIZE-8);
	FbWriteLine(lower_control_info);
}
static void draw_monster_avatar_screen(void)
{
	uint64_t now = rtc_get_ms_since_boot();
	struct monster *m = &monsters[current_monster_id];
	if (!m->unlocked) {
		draw_monster_avatar_screen_locked(now);
		return;
	}

	FbClear();
	draw_monster_avatar(m->image);

	struct ui_text_box box = {
		.x = 8,
		.y = LCD_YSIZE - 30,
		.width = LCD_XSIZE -16,
		.height = 24,
		.outline_size = 2,
		.outline_color = palette_color_from_index(default_palette, 13),
		.fill_color = palette_color_from_index(default_palette, 0),
		.text_color = palette_color_from_index(default_palette, 11),
	};

	if (show_description) {
		box.y = LCD_YSIZE - 60;
		box.height = 48;
		box.text = m->description;
		ui_text_box_draw(box);
	}
	FbMove(0, 0);
	FbColor(WHITE);
	FbWriteString(m->name);
	if (current_monster_id == initial_mon) {
		FbMove(100, 8);
		FbColor(YELLOW);
		FbWriteString("starter");
	}
	if (m->unlocked && m->shiny) {
		FbMove(100, 16);
		FbColor(WHITE);
		FbWriteString("*shiny");
	}

	char *lower_control_info = "<Back|Down|Desc>";
	FbColor(WHITE);
	FbMove(ui_center_text_x(lower_control_info, 0, LCD_XSIZE), LCD_YSIZE-8);
	FbWriteLine(lower_control_info);
	char idbuf[8];
	snprintf(idbuf, sizeof(idbuf), "#%03u", current_monster_id + 1);
	FbMove(LCD_XSIZE - 4*8, 0);
	FbWriteString(idbuf);

	if (monsters[current_monster_id].shiny && now >= sparkle_cooldown) {
		spawn_sparkles(now, 16, 4, LCD_XSIZE, LCD_YSIZE);
		sparkle_cooldown = now + SPARKLE_INTERVAL_MS;
		screen_changed = true;
	}
	draw_sparkles(now);
}

static void start_scanline_animation(void)
{
	scan_animating = true;
	scanline_animation_y = 0;
	screen_changed = true;
}

static void top_menu_action_monsters(void)
{
	badgemon_state = BADGEMON_MONSTER_AVATAR;
	screen_changed = true;
}
static void top_menu_action_show_progress_page(void)
{
	badgemon_state = BADGEMON_PROGRESS;
	screen_changed = true;
}
static void top_menu_action_trade_monsters(void)
{
	badgemon_state = BADGEMON_TRADE_MONSTERS;
	start_scanline_animation();
	screen_changed = true;
}
static void top_menu_action_help_screen(void)
{
	badgemon_state = BADGEMON_HELP_SCREEN;
	screen_changed = true;
}
static void top_menu_action_unlock_all(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(monsters); i++)
		monsters[i].unlocked = true;
}
static void top_menu_action_lock_all(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(monsters); i++)
		monsters[i].unlocked = false;
}
static void top_menu_action_exit(void)
{
	badgemon_state = BADGEMON_EXIT;
}
static const char *menu_items[] = {
	"monsters",
	"progress",
	"trade monsters",
	"how to play",
#ifdef __linux__
	"unlock all",
	"lock all",
#endif
	"exit",
};

static void (*menu_actions[])(void) = {
	top_menu_action_monsters,
	top_menu_action_show_progress_page,
	top_menu_action_trade_monsters,
	top_menu_action_help_screen,
#ifdef __linux__
	top_menu_action_unlock_all,
	top_menu_action_lock_all,
#endif
	top_menu_action_exit,
};
static void previous_menu_item(void)
{
	current_menu_item--;
	screen_changed = true;
	if (current_menu_item < 0)
		current_menu_item = NUM_MENU_ITEMS - 1;
}
static void next_menu_item(void)
{
	current_menu_item++;
	screen_changed = true;
	if (current_menu_item >= (int)NUM_MENU_ITEMS)
		current_menu_item = 0;
}
static void handle_options_top_menu(void)
{
	if (current_menu_item >= 0 && current_menu_item < (int)NUM_MENU_ITEMS)
		menu_actions[current_menu_item]();
}


static void return_to_top_menu(void)
{
	badgemon_state = BADGEMON_TOP_MENU;
	screen_changed = true;
	scan_animating = false;
	scanline_animation_y = 0;
}

static void check_buttons_avatar_screen(void)
{
	int down = button_down_latches();
	int n = ARRAY_SIZE(monsters);

	if (BUTTON_PRESSED(BADGE_BUTTON_UP, down)) {
		current_monster_id = (current_monster_id + n -1) % n;
		screen_changed = true;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down)) {
		current_monster_id = (current_monster_id +1) % n;
		screen_changed = true;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down)) {
		show_description = !show_description;
		screen_changed = true;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down) ||
		BUTTON_PRESSED(BADGE_BUTTON_A, down) ||
		BUTTON_PRESSED(BADGE_BUTTON_B, down)) {
		badgemon_state = BADGEMON_TOP_MENU;
		show_description = false;
		screen_changed = true;
		current_monster_id = initial_mon;
	}
}
static void check_buttons_top_menu(void)
{
	int down = button_down_latches();
	current_menu_item_selected = false;

	if (BUTTON_PRESSED(BADGE_BUTTON_UP, down)) {
		previous_menu_item();
	} else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down)) {
		next_menu_item();
	} else if (BUTTON_PRESSED(BADGE_BUTTON_A, down)) {
		current_menu_item_selected = true;
		handle_options_top_menu();
		screen_changed = true;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_B, down)) {
		badgemon_state = BADGEMON_EXIT;
	}
}

static void check_buttons_noop_screen(void)
{
	int down = button_down_latches();
	if (BUTTON_PRESSED(BADGE_BUTTON_A, down) ||
		BUTTON_PRESSED(BADGE_BUTTON_B, down) ||
		BUTTON_PRESSED(BADGE_BUTTON_UP, down)||
		BUTTON_PRESSED(BADGE_BUTTON_DOWN, down)||
		BUTTON_PRESSED(BADGE_BUTTON_LEFT, down)||
		BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down))
			return_to_top_menu();
}

static void draw_top_menu(void)
{
	for (int i = 0; i < (int)NUM_MENU_ITEMS; i++) {
		int yoffs = (i - current_menu_item)*MENU_ITEM_SPACING;
		struct ui_button b = {
			.x = MENU_X, .y = MENU_Y+yoffs,
			.width = MENU_ITEM_WIDTH,
			.height= MENU_ITEM_HEIGHT,
			.text = menu_items[i],
			.outline_size = 3,
			.outline_color = palette_color_from_index(default_palette, 13),
			.fill_color = palette_color_from_index(default_palette, 0),
			.text_color = palette_color_from_index(default_palette,11),
		};
		if (i == current_menu_item) {
			b.outline_color = palette_color_from_index(default_palette, 6);
			if (current_menu_item_selected)
				b.fill_color = palette_color_from_index(default_palette,5);
		}
		ui_button_dither_fill(b, b.fill_color, palette_color_from_index(default_palette,0),1);
		ui_button_draw_outline(b, b.outline_color);
		ui_button_draw_label(b, b.text_color);
	}
}

static void draw_scanline_animation(void)
{
	if (!scan_animating) return;
	FbHorizontalLine(0, scanline_animation_y, LCD_XSIZE-1, scanline_animation_y);
	if (++scanline_animation_y >= LCD_YSIZE)
		scanline_animation_y = 0;
	screen_changed = true;
}

static void draw_trade_monsters_screen(void)
{
	FbClear();

	const char *lines[] = {
		"trading monsters",
		"",
		"be brave",
		"",
		"point your badge",
		"at another badge",
		"to send/receive",
		"",
		"",
		"",
		"<---------->",
		"",
		"",
	};

	int y = 16;
	for (size_t i = 0; i < sizeof(lines) / sizeof(lines[0]); i++) {
		y += 8;
		if (lines[i][0] == '\0') {
			/* Skip empty lines for spacing */
			continue;
		}
		FbColor(GREEN);
		FbMove(ui_center_text_x(lines[i], 0, LCD_XSIZE), y);
		FbWriteString(lines[i]);
	}

	if (scan_animating)
		draw_scanline_animation();
}

static void draw_centered_text_page(const char *lines[], int n, int start, int h)
{
	int y = start;
	for (int i=0;i<n;i++){
		y+=h;
		if (lines[i][0]){
			FbMove(ui_center_text_x(lines[i],0,LCD_XSIZE), y);
			FbWriteString(lines[i]);
		}
	}
}

static void draw_progress_menu(void)
{
	FbClear();
	const char *lines[] = {
		"", "", "progress menu", "", ""
	};
	draw_centered_text_page(lines, ARRAY_SIZE(lines), 16, 16);
}

static void draw_help_screen(void)
{
	FbClear();

	const char *lines[] = {
		"unlock monsters!",
		"you can share your",
		"starter monster",
		"with other attendees",
		"",
		"interact with other",
		"badge apps to",
		"possibly find more",
		"",
		"do your best to",
		"collect them all",
		"",
		"can you acquire",
		"most of them?",
	};
	draw_centered_text_page(lines, sizeof(lines) / sizeof(lines[0]), 0, 8);
}

static bool trading_monsters_enabled = false;
static void trade_monsters(void)
{
	static int counter = 0;
	counter++;
	if ((counter % 10) == 0) {
		build_and_send_packet(
			BADGE_IR_GAME_ADDRESS,
			BADGE_IR_BROADCAST_ID,
			(OPCODE_XMIT_MONSTER << 12) | (initial_mon & 0x01ff)
		);
		audio_out_beep(500, 100);
		badgemon_state = BADGEMON_TRADE_MONSTERS_DELAY;
	}
	if (!trading_monsters_enabled) {
		trading_monsters_enabled = true;
	}
	check_buttons_noop_screen();
}

static void trade_monsters_delay(void)
{
	static int just_begun = 1;
	static uint64_t stop_time = 0;

	if (just_begun) {
		stop_time = rtc_get_ms_since_boot() + 1000;
		just_begun = 0;
	} else {
		uint64_t now = rtc_get_ms_since_boot();
		if (now > stop_time) {
			just_begun = 1;
			trading_monsters_enabled = false;
			badgemon_state = BADGEMON_TRADE_MONSTERS;
		}
	}

	check_buttons_noop_screen();
}

static void badgemon_init(void)
{
	FbInit();
	FbClear();
	badgemon_state = BADGEMON_TOP_MENU;
	screen_changed = true;

	load_monsters_from_flash();
	register_ir_packet_callback(ir_packet_callback);

	sparkle_cooldown = rtc_get_ms_since_boot();
	initial_mon = badge_system_data()->badgeId % 16;
	current_monster_id = initial_mon;
	monsters[initial_mon].unlocked = true;

	badgemon_state = BADGEMON_TOP_MENU;
	screen_changed  = true;
}

void badgemon_cb(__attribute__((unused)) struct badge_app *app)
{
	if (app->wake_up) {
		screen_changed = true;
	}

	switch (badgemon_state) {
	case BADGEMON_INIT:
		badgemon_init();
		break;
	case BADGEMON_MONSTER_AVATAR: {
		check_buttons_avatar_screen();
		draw_monster_avatar_screen();
		break;
	}
	case BADGEMON_PROGRESS:
		check_buttons_noop_screen();
		draw_progress_menu();
		break;
	case BADGEMON_TRADE_MONSTERS:
		trade_monsters();
		break;
	case BADGEMON_TRADE_MONSTERS_DELAY: {
		trade_monsters_delay();
		check_buttons_noop_screen();
		draw_trade_monsters_screen();
		break;
	}
	case BADGEMON_HELP_SCREEN:
		check_buttons_noop_screen();
		draw_help_screen();
		break;
	case BADGEMON_TOP_MENU:
		if (initial_run)
			screen_changed = true;

		initial_run = false;
		FbClear();
		check_buttons_top_menu();
		draw_top_menu();
		break;
	case BADGEMON_EXIT:
		badgemon_state = BADGEMON_INIT;
		current_menu_item = 0;
		save_monsters_to_flash();
		pop_app();
		break;
	}
	if (screen_changed) {
		FbSwapBuffers();
		screen_changed = false;
	}
}
