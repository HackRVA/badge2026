#include "menu.h"
#include "button.h"
#include "framebuffer.h"
#include "display.h"
#include "colors.h"
#include "led_pwm.h"
#include "badge.h"
#include "screensavers.h"
#include "screensaver_app.h"
#include "xorshift.h"
#include "utils.h"
#include "rtc.h"

#define SCREENSAVER_DURATION_FRAMES (9 * BADGE_FRAME_RATE_FPS) /**< Time spent displaying screensaver. */
#define SCREENSAVER_DARK_FRAMES (10 * BADGE_FRAME_RATE_FPS) /**< Time spent dark. */
#define SCREENSAVER_TIMEOUT_US (300 * 1000 * 1000) /**< Time in app before staying dark forever. */

typedef void (*ss_func)(void);

static const ss_func SS[] = {
	just_the_badge_tips,
	dotty,
	disp_asset_saver,
	matrix,
	qix,
	hyperspace_screen_saver,
	nametag_screensaver,
};

#define SCREENSAVER_IDX_INVALID (ARRAY_SIZE(SS) + 1U)

/** Program states.  Initial state is SCREENSAVER_INIT */
enum screensaver_state_t {
	SCREENSAVER_INIT, /**< Initial state. */
	SCREENSAVER_SHOW, /**< Display screen saver. */
	SCREENSAVER_DARK, /**< Go dark between screen savers. */
	SCREENSAVER_EXIT, /**< Exit state; clanup and restore state. */
};

/** Screensaver app context structure. */
struct ss_app_ctx {
	/** Screensaver app state. */
	enum screensaver_state_t state;
	/** Current index into the screensaver list. */
	unsigned int idx; 
	/** XOR shift PRNG internal state. */
	unsigned int xor_state;
	/** Frame counter to time duration in states. */
	uint32_t cnt_frames;
	/** us since boot when app entered. */
	uint64_t start_us;
};

/** Defualt screensaver app context. */
struct ss_app_ctx m_screensaver_app_context = {
	.state = SCREENSAVER_INIT,
	.idx = SCREENSAVER_IDX_INVALID,
	.xor_state = 0xa5a5a5a5,
	.cnt_frames = 0,
	.start_us = 0,
};

static void choose_screensaver(struct ss_app_ctx *ctx)
{
	unsigned int new = ctx->idx;
	while (new == ctx->idx) {
		new = xorshift(&ctx->xor_state) % ARRAY_SIZE(SS);
	}
	ctx->idx = new;
	
	/* TODO: Refactor this so the state holding the animation count is
	 * passed into the screensavers instead of global requiring this
	 * hack. -PMW
	 */
	screensaver_set_animation_count(0);
	ctx->cnt_frames = 0;
}

static void go_dark(void)
{
	led_pwm_disable(BADGE_LED_RGB_RED);
	led_pwm_disable(BADGE_LED_RGB_GREEN);
	led_pwm_disable(BADGE_LED_RGB_BLUE);
	led_pwm_disable(BADGE_LED_DISPLAY_BACKLIGHT);
	FbColor(BLACK);
	FbBackgroundColor(BLACK);
	FbClear();
	FbPushBuffer();
}

static void go_bright(void)
{
	led_pwm_enable(BADGE_LED_DISPLAY_BACKLIGHT, 
		       badge_system_data()->backlight);
}

/** Exit if any buttons are pressed. */
static void check_buttons(struct ss_app_ctx *ctx)
{
	int down_latches = button_down_latches();
	if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches) ||
		BUTTON_PRESSED(BADGE_BUTTON_LEFT, down_latches) ||
		BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches) ||
		BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches) ||
		BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches) ||
		BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
		ctx->state = SCREENSAVER_EXIT;
	}
}

static void screensaver_init(struct ss_app_ctx *ctx)
{
	/* Reset the frame buffer to give the screen savers a blank slate. */
	FbInit();
	FbBackgroundColor(BLACK);
	FbClear();

	choose_screensaver(ctx);

	/* Mark start time if not already set. */
	if (0 == ctx->start_us) {
		ctx->start_us = rtc_get_us_since_boot();
	}

	/* Next, run/display the chosen screensaver. */
	ctx->state = SCREENSAVER_SHOW;
}

static void screensaver_show(struct ss_app_ctx *ctx)
{
	check_buttons(ctx);

	/* Go to sleep between screensavers. */
	if (ctx->cnt_frames >= SCREENSAVER_DURATION_FRAMES) {
		go_dark();
		ctx->cnt_frames = 0;
		ctx->state = SCREENSAVER_DARK;
	}

	/* Run screensaver. */
	SS[ctx->idx]();
	ctx->cnt_frames++;
}

static void screensaver_dark(struct ss_app_ctx *ctx)
{
	check_buttons(ctx);
	ctx->cnt_frames++;
	if (((rtc_get_us_since_boot() - ctx->start_us) < SCREENSAVER_TIMEOUT_US)
	    && (SCREENSAVER_DARK_FRAMES < ctx->cnt_frames)) {
		go_bright();
		ctx->state = SCREENSAVER_INIT;
	}
}

static void screensaver_exit(struct ss_app_ctx *ctx)
{
	/* Reset app for next invocation. */
	ctx->state = SCREENSAVER_INIT;
	ctx->cnt_frames = 0;
	ctx->start_us = 0;

	/* Always restart display. */
	go_bright();

	pop_app();
}

void screensaver_cb(__attribute__((unused)) struct badge_app *app)
{
	struct ss_app_ctx *ctx = &m_screensaver_app_context;
	switch (ctx->state) {
	case SCREENSAVER_INIT:
		screensaver_init(ctx);
		break;
	case SCREENSAVER_SHOW:
		screensaver_show(ctx);
		break;
	case SCREENSAVER_DARK:
		screensaver_dark(ctx);
		break;
	case SCREENSAVER_EXIT:
		screensaver_exit(ctx);
		break;
	default:
		break;
	}
}

