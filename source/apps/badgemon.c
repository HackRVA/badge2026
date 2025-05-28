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

#if TARGET_SIMULATOR
#define DEV_OPTIONS_ENABLED 1
#else
#define DEV_OPTIONS_ENABLED 0
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define NUM_MENU_ITEMS ARRAY_SIZE(menu_items)
#define MENU_ITEM_SPACING 30
#define MENU_ITEM_WIDTH 120
#define MENU_ITEM_HEIGHT 20
#define MENU_X (LCD_XSIZE/2 - MENU_ITEM_WIDTH/2)
#define MENU_Y (LCD_YSIZE/2 - MENU_ITEM_HEIGHT/2)
#define MAX_SPARKLES 16
#define SPARKLE_LIFETIME_MS 400

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
static enum badgemon_state_t last_state = BADGEMON_INIT;

static const struct palette default_palette = {
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
	uint32_t born_ms;
};
static uint64_t sparkle_cooldown = 0;
static unsigned sparkle_state = 2463534242;
static struct sparkle sparkles[MAX_SPARKLES];
static uint8_t sparkle_head = 0;
static const struct point sparkle_defs[MAX_SPARKLES] = {
	{ 90, 38 }, { 50, 75 }, {105, 42 }, { 65, 85 },
	{ 75, 30 }, { 95, 80 }, { 85, 55 }, { 60, 40 },
	{ 55, 50 }, {110, 70 }, { 45, 35 }, {100, 60 },
	{ 40, 60 }, { 70, 65 }, { 80, 90 }, {115, 55 },
};

static const char *progress_lines[] = {
	"", "", "monster unlock", "progress", ""
};
static const char *help_lines[] = {
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
static const char *trade_lines[] = {
	"trading monsters",
	"",
	"be brave",
	"",
	"point your badge",
	"at another badge",
	"to send/receive",
	"",
	"<---------->",
	"",
	"",
};

static const struct {
    const char *name;
    const char *description;
    const struct asset2 *image;
} monster_info[] = {
	{.name = "birdo", .description = "Spitting eggs? In this economy?", .image = &badge_monster_birdo},
	{.name = "bowser", .description = "Firewall admin gone rogue. Kidnaps root access daily.", .image = &badge_monster_bowser},
	{.name = "jason voorhees", .description = "Persistence malware in hockey-mask form. Won't stay deleted.", .image = &badge_monster_jason_voorhees},
	{.name = "crawler", .description = "Crawls your brain like a botnet—except squishier.", .image = &badge_monster_crawler},
	{.name = "ghosts", .description = "really into cardio, for beings without hearts.", .image = &badge_monster_ghosts},
	{.name = "siren", .description = "Spams alerts until you rage-quit the SOC.", .image = &badge_monster_harpy},
	{.name = "medusa", .description = "Social engineer with a killer stare-don't click that link!", .image = &badge_monster_medusa_head},
	{.name = "nettler", .description = "Evasion expert. IDS can't catch these moves.", .image = &badge_monster_nettler},
	{.name = "bigeyes", .description = "Knows your password. And your secrets. Probably shoulder surfed.", .image = &badge_monster_odd_eye},
	{.name = "shredder", .description = "Encrypts everything. Demands pizza for decryption.", .image = &badge_monster_shredder},
	{.name = "slime", .description = "Credential-stuffing blob. Oozes through weak auth.", .image = &badge_monster_slime},
	{.name = "metall", .description = "Immutable config. Patch-resistant. Judges your uptime.", .image = &badge_monster_metall},
	{.name = "moblin", .description = "Brute force incarnate. Loud, obvious, occasionally effective.", .image = &badge_monster_moblin},
	{.name = "mother brain", .description = "C2 server supreme. Coordinates attacks from a jar.", .image = &badge_monster_mother_brain},
	{.name = "stay puff", .description = "Looks harmless. Actually a phishing campaign in disguise.", .image = &badge_monster_stay_puff},
	{.name = "sinistar", .description = "Voice in your head. Probably a red team implant.", .image = &badge_monster_sinistar},

	{.name = "beetlejuice", .description = "Say its name three times and your logs vanish.", .image = &badge_monster_beetlejuice},
	{.name = "chet", .description = "Definitely installed something weird on the network printer.", .image = &badge_monster_chet},
	{.name = "chucky", .description = "Portable USB menace. Small. Stabby. Plugs in without asking.", .image = &badge_monster_chucky},
	{.name = "drago", .description = "Punches ice. Punches people. Punches fate.", .image = &badge_monster_drago},
	{.name = "ed rooney", .description = "Overbearing access control. Gets owned anyway.", .image = &badge_monster_ed_rooney},
	{.name = "freddy krueger", .description = "Haunts your threat model. Shows up in incident response dreams.", .image = &badge_monster_freddy_krueger},
	{.name = "gopher", .description = "Legacy protocol. Won't die. Still exfiltrates snacks.", .image = &badge_monster_gopher_caddyshack},
	{.name = "gizmo", .description = "Post-update chaos agent. Breaks everything after midnight.", .image = &badge_monster_gremlin},
	{.name = "hans gruber", .description = "Ransomware with an accent and a plan.", .image = &badge_monster_hans_gruber},
	{.name = "jack torrance", .description = "Rogue sysadmin with an axe to grind.", .image = &badge_monster_jack_torrance},
	{.name = "joker", .description = "Injects chaos into secure environments. Laughs while doing it.", .image = &badge_monster_joker},
	{.name = "khan", .description = "Cries 'Khaaaaaan!' in mirror every morning.", .image = &badge_monster_khan},
	{.name = "richard vernon", .description = "Legacy system admin. Still hasn't enabled 2FA.", .image = &badge_monster_richard_vernon},
	{.name = "skeletor", .description = "Yells about power, has none over his cat.", .image = &badge_monster_skeltor},
};

static uint32_t unlocked_mask;
static uint32_t shiny_mask;

bool is_unlocked(int i) { return unlocked_mask & (1u << i); }
void set_unlocked(int i,bool v) {
	if (v) unlocked_mask |= (1u << i);
	else unlocked_mask &= ~(1u << i);
}

bool is_shiny(int i) { return shiny_mask & (1u << i); }
void set_shiny(int i,bool v) {
	if (v) shiny_mask |= (1u << i);
	else shiny_mask &= ~(1u << i);
}

static char kvbuf[32];
static const char *flash_key_from_monster(int id)
{
	snprintf(kvbuf, sizeof(kvbuf), "monster/%s", monster_info[id].name);
	return kvbuf;
}

static void load_monsters_from_flash(void)
{
	for (int i = 0; i < (int)ARRAY_SIZE(monster_info); i++) {
		int val = 0;
		flash_kv_get_int(flash_key_from_monster(i), &val);
		set_unlocked(i, (val != 0));
	}
}

static void save_monsters_to_flash(void)
{
	for (int i = 0; i < (int)ARRAY_SIZE(monster_info); i++)
		flash_kv_store_int(flash_key_from_monster(i), is_unlocked(i) ? 1 : 0);
}

static void spawn_sparkle(uint32_t now) {
	uint8_t idx = sparkle_head++;
	if (sparkle_head >= MAX_SPARKLES) sparkle_head = 0;
	sparkles[idx].born_ms = now ? now : 1;
}

static void draw_sparkles(uint32_t now) {
	for (int i = 0; i < MAX_SPARKLES; i++) {
		uint32_t born = sparkles[i].born_ms;
		if (!born) continue;
		if ((now - born) > SPARKLE_LIFETIME_MS) {
			sparkles[i].born_ms = 0;
			continue;
		}
		uint8_t x = sparkle_defs[i].x;
		uint8_t y = sparkle_defs[i].y;

		FbColor(WHITE);
		FbPoint(x, y);
		FbPoint(x - 1, y);
		FbPoint(x + 1, y);
		FbPoint(x, y - 1);
		FbPoint(x, y + 1);
	}
}

static bool show_description = false;
static unsigned int current_monster_id = 0;

static void draw_monster_avatar(const struct asset2 *img)
{
	FbMove(LCD_XSIZE/2 - (img->x/2), LCD_YSIZE/2 - (img->y/2));
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

	unsigned int state = (unsigned int)mosaic_seed;
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
	const size_t id = current_monster_id;
	if (!is_unlocked(id)) {
		draw_monster_avatar_screen_locked(now);
		return;
	}

	FbClear();
	draw_monster_avatar(monster_info[id].image);

	FbMove(0, 0);
	FbColor(WHITE);
	FbWriteString(monster_info[id].name);

	if (id == initial_mon) {
		FbMove(100, 8);
		FbColor(YELLOW);
		FbWriteString("starter");
	}

	if (show_description) {
		struct ui_text_box box = {
			.x = 8,
			.y = LCD_YSIZE - 60,
			.width = LCD_XSIZE - 16,
			.height = 48,
			.outline_size = 2,
			.outline_color = palette_color_from_index(default_palette, 13),
			.fill_color = palette_color_from_index(default_palette, 0),
			.text_color = palette_color_from_index(default_palette, 11),
			.text = monster_info[id].description,
		};
		ui_text_box_draw(box);
	}

	const char *ctrl = "a: desc | b: back";
	FbColor(WHITE);
	FbMove(ui_center_text_x(ctrl, 0, LCD_XSIZE), LCD_YSIZE - 8);
	FbWriteLine(ctrl);

	char idbuf[8];
	snprintf(idbuf, sizeof(idbuf), "#%03u", (unsigned)(id + 1));
	FbMove(LCD_XSIZE - 4 * 8, 0);
	FbWriteString(idbuf);

	if (is_shiny(id) && now >= sparkle_cooldown) {
		FbMove(100, 16);
		FbColor(WHITE);
		FbWriteString("*shiny");
		spawn_sparkle(now);
		sparkle_cooldown = now + SPARKLE_LIFETIME_MS;
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
	last_state = badgemon_state;
	badgemon_state = BADGEMON_MONSTER_AVATAR;
	screen_changed = true;
}
static void top_menu_action_show_progress_page(void)
{
	last_state = badgemon_state;
	badgemon_state = BADGEMON_PROGRESS;
	screen_changed = true;
}
static void top_menu_action_trade_monsters(void)
{
	last_state = badgemon_state;
	badgemon_state = BADGEMON_TRADE_MONSTERS;
	start_scanline_animation();
	screen_changed = true;
}
static void top_menu_action_help_screen(void)
{
	last_state = badgemon_state;
	badgemon_state = BADGEMON_HELP_SCREEN;
	screen_changed = true;
}
static void top_menu_action_unlock_all(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(monster_info); i++)
		set_unlocked(i, true);
}
static void top_menu_action_lock_all(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(monster_info); i++)
		set_unlocked(i, false);
}
static void top_menu_action_exit(void)
{
	last_state = badgemon_state;
	badgemon_state = BADGEMON_EXIT;
}
static const char *menu_items[] = {
	"monsters",
	"progress",
	"trade monsters",
	"how to play",
#if DEV_OPTIONS_ENABLED
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
#if DEV_OPTIONS_ENABLED
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
	last_state = badgemon_state;
	badgemon_state = BADGEMON_TOP_MENU;
	screen_changed = true;
	scan_animating = false;
	scanline_animation_y = 0;
}

static void check_buttons_avatar_screen(void)
{
	int down = button_down_latches();
	int n = ARRAY_SIZE(monster_info);

	if (BUTTON_PRESSED(BADGE_BUTTON_UP, down)) {
		current_monster_id = (current_monster_id + n - 1) % n;
		show_description = false;
		screen_changed = true;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down)) {
		current_monster_id = (current_monster_id + 1) % n;
		show_description = false;
		screen_changed = true;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down)) {
		current_monster_id = (current_monster_id + n - 1) % n;
		show_description = false;
		screen_changed = true;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down)) {
		current_monster_id = (current_monster_id + 1) % n;
		show_description = false;
		screen_changed = true;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_A, down)) {
		show_description = !show_description;
		screen_changed = true;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_B, down)) {
		last_state = badgemon_state;
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
		last_state = badgemon_state;
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
			.outline_size = 1,
			.outline_color = palette_color_from_index(default_palette, 13),
			.fill_color = palette_color_from_index(default_palette, 0),
			.text_color = palette_color_from_index(default_palette,11),
		};
		if (i == current_menu_item) {
			b.outline_color = palette_color_from_index(default_palette, 6);
			b.fill_color = palette_color_from_index(default_palette, 1);
			if (current_menu_item_selected)
				b.fill_color = palette_color_from_index(default_palette,5);
		}

		if (b.y < 0 || b.y > LCD_YSIZE) continue;
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

static void draw_trade_monsters_screen(void)
{
	FbClear();

	draw_centered_text_page(trade_lines, ARRAY_SIZE(trade_lines), 16, 8);

	if (scan_animating)
		draw_scanline_animation();
}


/* TODO: it's probably better to not calculate this every time */
static int get_unlocked_count(void)
{
	int total = ARRAY_SIZE(monster_info);
	int unlocked_count = 0;
	for (int i = 0; i < total; i++) {
		if (is_unlocked(i)) {
			unlocked_count++;
		}
	}
	return unlocked_count;
}

static void draw_progress_menu(void)
{
	FbClear();

	int total = ARRAY_SIZE(monster_info);
	int unlocked_count = get_unlocked_count();

	int pct = (unlocked_count * 100) / total;

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

	char buf[16];
	snprintf(buf, sizeof(buf), "%d/%d", unlocked_count, total);
	FbMove(ui_center_text_x(buf,0,LCD_XSIZE), pb.y + pb.height + 8);
	FbColor(WHITE);
	FbWriteString(buf);

	draw_centered_text_page(progress_lines, ARRAY_SIZE(progress_lines), 16, 16);
}


static void draw_help_screen(void)
{
	FbClear();

	draw_centered_text_page(help_lines, sizeof(help_lines) / sizeof(help_lines[0]), 0, 8);
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
		last_state = badgemon_state;
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
		unsigned int rand_interval = (xorshift(&sparkle_state) % 1000) + 1000;
		stop_time = rtc_get_ms_since_boot() + rand_interval;
		just_begun = 0;
	} else {
		uint64_t now = rtc_get_ms_since_boot();
		if (now > stop_time) {
			just_begun = 1;
			trading_monsters_enabled = false;
			last_state = badgemon_state;
			badgemon_state = BADGEMON_TRADE_MONSTERS;
		}
	}

	check_buttons_noop_screen();
}

static void badgemon_init(void)
{
	FbInit();
	FbClear();
	last_state = badgemon_state;
	badgemon_state = BADGEMON_TOP_MENU;
	screen_changed = true;

	load_monsters_from_flash();
	register_ir_packet_callback(ir_packet_callback);

	sparkle_cooldown = rtc_get_ms_since_boot();
	initial_mon = badge_system_data()->badgeId % 16;
	current_monster_id = initial_mon;
	set_shiny(initial_mon, true);
	set_unlocked(initial_mon, true);

	set_shiny(4, true);
	set_shiny(8, true);
	set_shiny(19, true);
	set_shiny(22, true);
	set_shiny(26, true);

	screen_changed  = true;
}

void badgemon_cb(__attribute__((unused)) struct badge_app *app)
{
	if (app->wake_up) {
		screen_changed = true;
		app->wake_up = 0;
	}
	if (badgemon_state != last_state) {
		screen_changed = true;
		last_state = badgemon_state;
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
	case BADGEMON_TRADE_MONSTERS: {
		trade_monsters();
		check_for_incoming_packets();
		break;
	}
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
		unregister_ir_packet_callback(ir_packet_callback);
		save_monsters_to_flash();
		pop_app();
		break;
	}
	if (screen_changed) {
		FbSwapBuffers();
		screen_changed = false;
	}
}

void badgemon_draw_screen_saver_monster(void)
{
	static unsigned char current_index = 0;
	current_index++;
	current_index %= ARRAY_SIZE(monster_info) - 1;
	const struct asset2 *img = monster_info[current_index].image;

	FbClear();
	FbColor(BLACK);
	FbMove(LCD_XSIZE/2 - (img->x/2), LCD_YSIZE/2 - (img->y/2));
	FbImage4bit2(img, 0);
}

void badgemon_unlock_monster(int monster_id)
{
	flash_kv_store_int(flash_key_from_monster(monster_id), 1);
	set_unlocked(monster_id, 1);

	audio_out_beep(1200, 600);
}
