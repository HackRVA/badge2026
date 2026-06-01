#include <stdbool.h>
#include <stdio.h>

#include "badge.h"
#include "button.h"
#include "colors.h"
#include "framebuffer.h"
#include "palette.h"
#include "rtc.h"
#include "ui.h"
#include "xorshift.h"

#define DECK_N 52
#define HAND_MAX 12

/* Tuned card-table palette; PC(i) fetches a color by index (as in daywalker). */
static const struct palette bj_palette = {
	.colors = {
		PACKRGB888(0, 0, 0),       /* 0 transparent / shadow */
		PACKRGB888(245, 245, 240), /* 1 card face white */
		PACKRGB888(200, 40, 50),   /* 2 red suit */
		PACKRGB888(22, 22, 30),    /* 3 black suit / ink */
		PACKRGB888(18, 105, 58),   /* 4 felt green (deep) */
		PACKRGB888(32, 140, 80),   /* 5 felt green (light) */
		PACKRGB888(235, 205, 95),  /* 6 gold */
		PACKRGB888(150, 120, 45),  /* 7 dark gold */
		PACKRGB888(45, 95, 175),   /* 8 card-back blue */
		PACKRGB888(95, 155, 225),  /* 9 card-back light */
		PACKRGB888(120, 122, 135), /* 10 card border grey */
		PACKRGB888(10, 50, 30),    /* 11 felt shadow */
	},
};

#define PC(i) palette_color_from_index(bj_palette, (i))

enum bj_state {
	BJ_INIT,
	BJ_BET,
	BJ_PLAYER,
	BJ_DEALER,
	BJ_RESULT,
	BJ_BROKE,
	BJ_EXIT
};

static enum bj_state state = BJ_INIT;
static unsigned int rng_state;
static unsigned char deck[DECK_N], deck_pos;
static unsigned char player[HAND_MAX], dealer[HAND_MAX];
static int player_n, dealer_n;
static int bankroll, bet, round_bet, message_timer, screen_changed;
static int anim_timer, anim_kind, anim_hand, anim_index;
static char message[32];

enum anim_kind {
	ANIM_NONE,
	ANIM_DEAL,
	ANIM_SETTLE,
	ANIM_DOUBLE
};

static int rnd(int n)
{
	if (n <= 0)
		return 0;
	return (int)(xorshift(&rng_state) % (unsigned int)n);
}

static void rect(int x, int y, int w, int h, unsigned short c)
{
	if (w > 0 && h > 0)
		FbPlaceFilledRectangle(x, y, w, h, c);
}

static void point(int x, int y, unsigned short c)
{
	FbColor(c);
	FbPoint(x, y);
}

static void line(int x0, int y0, int x1, int y1, unsigned short c)
{
	FbColor(c);
	FbLine(x0, y0, x1, y1);
}

static void fill_disc(int cx, int cy, int r, unsigned short c)
{
	int dx, dy, r2 = r * r;

	FbColor(c);
	for (dy = -r; dy <= r; dy++)
		for (dx = -r; dx <= r; dx++)
			if (dx * dx + dy * dy <= r2)
				FbPoint(cx + dx, cy + dy);
}

static void text_at(int x, int y, const char *s, unsigned short c)
{
	FbColor(c);
	FbMove(x, y);
	FbWriteString(s);
}

/* Draw text horizontally centered within [box_x, box_x + box_w). */
static void centered_line(int box_x, int box_w, int y, const char *s, unsigned short c)
{
	text_at(ui_center_text_x(s, box_x, box_w), y, s, c);
}

static void set_msg(const char *s)
{
	snprintf(message, sizeof(message), "%s", s);
	message_timer = 60;
}

static void start_anim(int kind, int hand, int index, int frames)
{
	anim_kind = kind;
	anim_hand = hand;
	anim_index = index;
	anim_timer = frames;
	screen_changed = 1;
}

static void tick_anim(void)
{
	if (anim_timer > 0) {
		anim_timer--;
		screen_changed = 1;
		if (anim_timer == 0)
			anim_kind = ANIM_NONE;
	}
}

static void tick_message(void)
{
	if (message_timer > 0) {
		message_timer--;
		screen_changed = 1;
	}
}

static int rank_of(unsigned char c)
{
	return (c % 13) + 1;
}

static int suit_of(unsigned char c)
{
	return c / 13;
}

static int card_value(unsigned char c)
{
	int r = rank_of(c);
	if (r == 1)
		return 11;
	if (r >= 10)
		return 10;
	return r;
}

static int hand_value(unsigned char *hand, int n, int *soft)
{
	int i, total = 0, aces = 0;

	for (i = 0; i < n; i++) {
		total += card_value(hand[i]);
		if (rank_of(hand[i]) == 1)
			aces++;
	}
	while (total > 21 && aces > 0) {
		total -= 10;
		aces--;
	}
	if (soft)
		*soft = aces > 0;
	return total;
}

static int blackjack(unsigned char *hand, int n)
{
	return n == 2 && hand_value(hand, n, 0) == 21;
}

static void shuffle(void)
{
	int i;

	for (i = 0; i < DECK_N; i++)
		deck[i] = (unsigned char)i;
	for (i = DECK_N - 1; i > 0; i--) {
		int j = rnd(i + 1);
		unsigned char t = deck[i];
		deck[i] = deck[j];
		deck[j] = t;
	}
	deck_pos = 0;
}

static unsigned char draw_card(void)
{
	if (deck_pos >= DECK_N - 8)
		shuffle();
	return deck[deck_pos++];
}

static void deal_card(unsigned char *hand, int *n)
{
	if (*n < HAND_MAX)
		hand[(*n)++] = draw_card();
}

/* Apply the round outcome to the bankroll and set the result message. */
static void settle_payout(void)
{
	int pv = hand_value(player, player_n, 0);
	int dv = hand_value(dealer, dealer_n, 0);

	if (pv > 21) {
		bankroll -= round_bet;
		set_msg("Bust. Dealer wins.");
	} else if (dv > 21) {
		bankroll += round_bet;
		set_msg("Dealer busts!");
	} else if (blackjack(player, player_n) && !blackjack(dealer, dealer_n)) {
		bankroll += round_bet * 3 / 2;
		set_msg("Blackjack pays!");
	} else if (blackjack(dealer, dealer_n) && !blackjack(player, player_n)) {
		bankroll -= round_bet;
		set_msg("Dealer blackjack.");
	} else if (pv > dv) {
		bankroll += round_bet;
		set_msg("You win!");
	} else if (pv < dv) {
		bankroll -= round_bet;
		set_msg("Dealer wins.");
	} else {
		set_msg("Push.");
	}
}

/* Move to the broke or result state depending on the remaining bankroll. */
static void finish_round(void)
{
	if (bankroll < 10) {
		if (bankroll < 0)
			bankroll = 0;
		state = BJ_BROKE;
		return;
	}
	if (bet > bankroll)
		bet = bankroll;
	state = BJ_RESULT;
}

static void settle_round(void)
{
	settle_payout();
	start_anim(ANIM_SETTLE, 0, 0, 20);
	finish_round();
	screen_changed = 1;
}

static void start_round(void)
{
	if (bet > bankroll)
		bet = bankroll;
	bet = (bet / 10) * 10;
	if (bet < 10)
		bet = 10;
	round_bet = bet;
	player_n = dealer_n = 0;
	deal_card(player, &player_n);
	deal_card(dealer, &dealer_n);
	deal_card(player, &player_n);
	deal_card(dealer, &dealer_n);
	start_anim(ANIM_DEAL, 1, player_n - 1, 16);
	if (blackjack(player, player_n) || blackjack(dealer, dealer_n)) {
		settle_round();
	} else {
		set_msg("Hit or stand");
		state = BJ_PLAYER;
	}
	screen_changed = 1;
}

static bool dealer_should_hit(void)
{
	int soft;
	int v = hand_value(dealer, dealer_n, &soft);

	/* Dealer hits soft 17 (H17 rule). */
	return v < 17 || (v == 17 && soft);
}

static void dealer_play(void)
{
	int drew = 0;

	while (dealer_n < HAND_MAX && dealer_should_hit()) {
		deal_card(dealer, &dealer_n);
		drew = 1;
	}
	if (drew)
		start_anim(ANIM_DEAL, 0, dealer_n - 1, 14);
	settle_round();
}

static void player_hit(void)
{
	deal_card(player, &player_n);
	start_anim(ANIM_DEAL, 1, player_n - 1, 16);
	if (hand_value(player, player_n, 0) > 21)
		settle_round();
	else
		screen_changed = 1;
}

static void double_down(void)
{
	if (bankroll < round_bet * 2 || player_n != 2) {
		set_msg("Can't double");
		screen_changed = 1;
		return;
	}
	round_bet *= 2;
	start_anim(ANIM_DOUBLE, 1, player_n, 14);
	deal_card(player, &player_n);
	start_anim(ANIM_DEAL, 1, player_n - 1, 16);
	if (hand_value(player, player_n, 0) > 21)
		settle_round();
	else
		dealer_play();
}

static const unsigned char sprite_heart[] = {
	0x02, 0x20, 0x22, 0x00, 0x22, 0x22, 0x22, 0x20,
	0x22, 0x22, 0x22, 0x20, 0x22, 0x22, 0x22, 0x20,
	0x02, 0x22, 0x22, 0x00, 0x00, 0x22, 0x20, 0x00,
	0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
static const unsigned char sprite_spade[] = {
	0x00, 0x03, 0x00, 0x00, 0x00, 0x33, 0x30, 0x00,
	0x03, 0x33, 0x33, 0x00, 0x33, 0x33, 0x33, 0x30,
	0x33, 0x33, 0x33, 0x30, 0x03, 0x33, 0x33, 0x00,
	0x00, 0x03, 0x00, 0x00, 0x00, 0x33, 0x30, 0x00,
};
static const unsigned char sprite_diamond[] = {
	0x00, 0x02, 0x00, 0x00, 0x00, 0x22, 0x20, 0x00,
	0x02, 0x22, 0x22, 0x00, 0x22, 0x22, 0x22, 0x20,
	0x02, 0x22, 0x22, 0x00, 0x00, 0x22, 0x20, 0x00,
	0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
static const unsigned char sprite_club[] = {
	0x00, 0x33, 0x30, 0x00, 0x00, 0x33, 0x30, 0x00,
	0x03, 0x33, 0x33, 0x00, 0x33, 0x33, 0x33, 0x30,
	0x33, 0x33, 0x33, 0x30, 0x00, 0x03, 0x00, 0x00,
	0x00, 0x33, 0x30, 0x00, 0x03, 0x33, 0x33, 0x00,
};
static const unsigned char *const suit_sprites[4] = {
	sprite_heart, sprite_spade, sprite_diamond, sprite_club
};

static void draw_sprite(int sx, int sy, const unsigned char *data)
{
	for (int row = 0; row < 8; row++) {
		int dy = sy + row;

		if (dy < 0 || dy >= LCD_YSIZE)
			continue;
		for (int b = 0; b < 4; b++) {
			unsigned char byte = data[row * 4 + b];
			unsigned char hi = (byte >> 4) & 0x0F;
			unsigned char lo = byte & 0x0F;
			int dx = sx + b * 2;

			if (hi && dx >= 0 && dx < LCD_XSIZE)
				point(dx, dy, PC(hi));
			if (lo && dx + 1 >= 0 && dx + 1 < LCD_XSIZE)
				point(dx + 1, dy, PC(lo));
		}
	}
}

/* the rank glyph only ("A", "2".."10", "J", "Q", "K") */
static void rank_label(unsigned char c, char *buf, int n)
{
	int r = rank_of(c);

	if (r == 1)
		snprintf(buf, n, "A");
	else if (r == 11)
		snprintf(buf, n, "J");
	else if (r == 12)
		snprintf(buf, n, "Q");
	else if (r == 13)
		snprintf(buf, n, "K");
	else
		snprintf(buf, n, "%d", r);
}

static void draw_card_back(int x, int y)
{
	int xx, yy;

	rect(x + 3, y + 3, 16, 24, PC(8));
	for (yy = y + 4; yy < y + 27; yy += 2)
		for (xx = x + 4 + ((yy & 1) ? 1 : 0); xx < x + 19; xx += 2)
			point(xx, yy, PC(9));
}

static void draw_card_box(int x, int y, unsigned char c, bool hidden)
{
	char buf[4];
	int suit = suit_of(c);
	unsigned short ink = (suit == 0 || suit == 2) ? PC(2) : PC(3);

	rect(x + 2, y + 2, 22, 30, PC(0));	/* drop shadow */
	rect(x, y, 22, 30, PC(10));		/* border */
	rect(x + 1, y + 1, 20, 28, PC(1));	/* white face */
	point(x + 1, y + 1, PC(10));		/* rounded corners */
	point(x + 20, y + 1, PC(10));
	point(x + 1, y + 28, PC(10));
	point(x + 20, y + 28, PC(10));
	if (hidden) {
		draw_card_back(x, y);
		return;
	}
	rank_label(c, buf, sizeof(buf));
	/* draw the rank on the white face: glyph background must match the card,
	 * not the default black, so it doesn't leave a dark box. */
	FbBackgroundColor(PC(1));
	text_at(x + 3, y + 2, buf, ink);
	FbBackgroundColor(BLACK);
	draw_sprite(x + 7, y + 16, suit_sprites[suit]);
}

static void draw_hand(unsigned char *hand, int n, int x, int y, bool hide_first, int owner)
{
	int i, step = 24;

	if (n > 6)
		step = 18;
	for (i = 0; i < n; i++) {
		int yy = y;
		int xx = x + i * step;

		if (anim_kind == ANIM_DEAL && anim_timer > 0 &&
		    anim_hand == owner && anim_index == i) {
			int lift = anim_timer > 8 ? anim_timer - 8 : anim_timer;

			yy -= lift;
			xx += (anim_timer > 8) ? 2 : 0;
		}
		draw_card_box(xx, yy, hand[i], hide_first && i == 0);
	}
}

/* A round poker chip: white rim, colored body, four rim notches and a pip. */
static void draw_chip(int x, int y, unsigned short c)
{
	fill_disc(x, y, 6, WHITE);	/* rim */
	fill_disc(x, y, 4, c);		/* colored body */
	point(x, y - 5, c);		/* edge spots */
	point(x, y + 5, c);
	point(x - 5, y, c);
	point(x + 5, y, c);
	point(x, y, WHITE);		/* center pip */
}

static void draw_badge_box(int x, int y, int w, const char *label, unsigned short c)
{
	rect(x + 1, y + 1, w, 12, BLACK);
	rect(x, y, w, 12, x11_gray40);
	FbColor(c);
	FbMove(x, y);
	FbRectangle(w, 12);
	text_at(x + 3, y + 2, label, c);
}

static void draw_felt(void)
{
	int x, y;

	rect(0, 0, LCD_XSIZE, LCD_YSIZE, PC(0));
	rect(0, 12, LCD_XSIZE, 102, PC(11));	/* dark green rail */
	rect(4, 16, 152, 94, PC(5));		/* light green border */
	rect(8, 20, 144, 86, PC(4));		/* deep green felt */
	for (y = 24; y < 104; y += 8)
		for (x = 12 + ((y / 8) & 1) * 4; x < 150; x += 12)
			point(x, y, PC(11));
	rect(0, 0, LCD_XSIZE, 12, PC(0));
	rect(0, 114, LCD_XSIZE, 14, PC(0));
	line(0, 63, LCD_XSIZE - 1, 63, PC(6));
	line(4, 16, 155, 16, PC(6));
	line(4, 110, 155, 110, PC(7));
}

static void draw_status_bar(void)
{
	char buf[32];

	snprintf(buf, sizeof(buf), "$%d  BET:%d", bankroll,
		 state == BJ_BET ? bet : round_bet);
	if (anim_kind == ANIM_DOUBLE && anim_timer > 0)
		rect(0, 0, 92, 12, x11_goldenrod4);
	text_at(2, 2, buf, WHITE);
	draw_chip(144, 6, anim_kind == ANIM_DOUBLE && anim_timer > 0 ? RED : x11_gold);
}

static void draw_dealer_area(bool hide)
{
	char buf[32];

	draw_badge_box(9, 18, 44, "DEALER", x11_gold);
	draw_hand(dealer, dealer_n, 12, 32, hide, 0);
	if (hide) {
		draw_badge_box(128, 32, 21, "?", CYAN);
		return;
	}
	snprintf(buf, sizeof(buf), "%d", hand_value(dealer, dealer_n, 0));
	draw_badge_box(128, 32, 21, buf, CYAN);
}

static void draw_player_area(void)
{
	char buf[32];

	draw_badge_box(9, 68, 44, "PLAYER", x11_gold);
	draw_hand(player, player_n, 12, 82, false, 1);
	snprintf(buf, sizeof(buf), "%d", hand_value(player, player_n, 0));
	draw_badge_box(128, 82, 21, buf, CYAN);
}

/* Boxed result banner that grows during the settle animation. */
static void draw_settle_banner(void)
{
	int w = 70 + (20 - anim_timer);
	int x = (LCD_XSIZE - w) / 2;
	struct ui_text_box box = {
		.x = x, .y = 54, .width = w, .height = 16,
		.outline_size = 1,
		.outline_color = x11_gold,
		.fill_color = BLACK,
		.text_color = x11_gold,
		.text = "",
	};

	ui_text_box_fill(box);
	ui_text_box_draw_outline(box);
	centered_line(box.x, box.width, 58, message, x11_gold);
}

static void draw_message_popup(void)
{
	centered_line(0, LCD_XSIZE, 116, message, x11_gold);
}

static void draw_table(void)
{
	bool hide = state == BJ_PLAYER;

	FbClear();
	draw_felt();
	draw_status_bar();
	draw_dealer_area(hide);
	draw_player_area();
	if (message_timer > 0)
		draw_message_popup();
	if (anim_kind == ANIM_SETTLE && anim_timer > 0)
		draw_settle_banner();
}

static void handle_bet_input(int down)
{
	if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down)) {
		bet -= 10;
		if (bet < 10)
			bet = 10;
		screen_changed = 1;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down)) {
		bet += 10;
		if (bet > bankroll)
			bet = bankroll;
		bet = (bet / 10) * 10;
		if (bet < 10)
			bet = 10;
		screen_changed = 1;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_A, down)) {
		start_round();
	}
}

static void update_bet(void)
{
	int down = button_down_latches();

	tick_anim();
	tick_message();
	if (BUTTON_PRESSED(BADGE_BUTTON_B, down) || BUTTON_PRESSED(BADGE_BUTTON_REWIND, down)) {
		state = BJ_EXIT;
		return;
	}
	handle_bet_input(down);
}

static void update_player(void)
{
	int down = button_down_latches();

	tick_anim();
	tick_message();
	if (BUTTON_PRESSED(BADGE_BUTTON_B, down) || BUTTON_PRESSED(BADGE_BUTTON_REWIND, down))
		state = BJ_EXIT;
	else if (BUTTON_PRESSED(BADGE_BUTTON_UP, down))
		player_hit();
	else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down))
		dealer_play();
	else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down))
		double_down();
}

static void update_result(void)
{
	int down = button_down_latches();

	tick_anim();
	tick_message();
	if (BUTTON_PRESSED(BADGE_BUTTON_B, down) || BUTTON_PRESSED(BADGE_BUTTON_REWIND, down)) {
		state = BJ_EXIT;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_A, down) || BUTTON_PRESSED(BADGE_BUTTON_DOWN, down)) {
		state = BJ_BET;
		set_msg("Place your bet");
		screen_changed = 1;
	}
}

static void draw_bet(void)
{
	char buf[32];

	if (!screen_changed)
		return;
	FbClear();
	draw_felt();
	centered_line(0, LCD_XSIZE, 18, "BLACKJACK", x11_gold);
	snprintf(buf, sizeof(buf), "Bankroll: $%d", bankroll);
	draw_badge_box(33, 42, 94, buf, WHITE);
	snprintf(buf, sizeof(buf), "Bet: $%d", bet);
	draw_badge_box(48, 60, 64, buf, CYAN);
	draw_chip(52, 88, x11_gold);
	draw_chip(80, 88, RED);
	draw_chip(108, 88, CYAN);
	centered_line(0, LCD_XSIZE, 100, "H17  3:2 BJ  no split", x11_gray40);
	centered_line(0, LCD_XSIZE, 116, "L/R bet A deal B quit", x11_gold);
	FbSwapBuffers();
	screen_changed = 0;
}

static void draw_play(void)
{
	if (!screen_changed)
		return;
	draw_table();
	if (state == BJ_PLAYER)
		text_at(6, 116, "UP hit  DN stand  RT dbl", x11_gray40);
	else if (state == BJ_RESULT)
		text_at(18, 116, "A next round   B quit", x11_gray40);
	FbSwapBuffers();
	screen_changed = 0;
}

static void draw_broke_panel(void)
{
	struct ui_text_box box = {
		.x = 18, .y = 20, .width = LCD_XSIZE - 36, .height = 80,
		.outline_size = 1,
		.outline_color = RED,
		.fill_color = BLACK,
		.text_color = WHITE,
		.text = "",
	};

	FbClear();
	rect(0, 0, LCD_XSIZE, LCD_YSIZE, BLACK);
	ui_text_box_fill(box);
	ui_text_box_draw_outline(box);
	centered_line(box.x, box.width, 28, "BROKE", RED);
	centered_line(box.x, box.width, 52, "House wins.", WHITE);
	centered_line(box.x, box.width, 86, "A restart  B quit", x11_gray40);
	FbSwapBuffers();
}

static void handle_broke_input(void)
{
	int down = button_down_latches();

	if (BUTTON_PRESSED(BADGE_BUTTON_A, down)) {
		bankroll = 100;
		bet = 10;
		round_bet = bet;
		state = BJ_BET;
		set_msg("Place your bet");
		screen_changed = 1;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_B, down) || BUTTON_PRESSED(BADGE_BUTTON_REWIND, down)) {
		state = BJ_EXIT;
	}
}

static void draw_broke(void)
{
	if (screen_changed) {
		draw_broke_panel();
		screen_changed = 0;
	}
	handle_broke_input();
}

static void bj_init(void)
{
	FbInit();
	FbClear();
	rng_state = (unsigned int)rtc_get_ms_since_boot();
	if (rng_state == 0)
		rng_state = 0xb1ac0ace;
	shuffle();
	bankroll = 100;
	bet = 10;
	round_bet = bet;
	anim_kind = ANIM_NONE;
	anim_timer = 0;
	set_msg("Place your bet");
	state = BJ_BET;
	screen_changed = 1;
}

static void bj_exit(void)
{
	state = BJ_INIT;
	pop_app();
}

void blackjack_cb(struct badge_app *app)
{
	if (app->wake_up) {
		screen_changed = 1;
		app->wake_up = 0;
	}
	switch (state) {
	case BJ_INIT:
		bj_init();
		break;
	case BJ_BET:
		update_bet();
		draw_bet();
		break;
	case BJ_PLAYER:
		update_player();
		draw_play();
		break;
	case BJ_DEALER:
		dealer_play();
		draw_play();
		break;
	case BJ_RESULT:
		update_result();
		draw_play();
		break;
	case BJ_BROKE:
		draw_broke();
		break;
	case BJ_EXIT:
		bj_exit();
		break;
	default:
		break;
	}
}
