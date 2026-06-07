#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "audio.h"
#include "button.h"
#include "colors.h"
#include "display.h"
#include "framebuffer.h"
#include "led_pwm.h"
#include "menu.h"
#include "music.h"
#include "particle.h"
#include "utils.h"
#include "badge.h"
#include "random.h"
#include "trig.h"
#include "fxp_sqrt.h"

#include "rvasec_splash.h"
#include "hack_logo_2.h"
#include "rvasec_splash_assets/rvasec15_glass_asset.h"
#include "rvasec_splash_assets/rvasec15_logo_asset.h"
#include "rvasec_splash_assets/rvasec15_raven_asset.h"
#include "rvasec_splash_assets/sponsor_logo.h"
#include "rvasec_splash_assets/rib.h"

#define SPLASH_SHIFT_DOWN (LCD_YSIZE / 2)
#define SPLASH_LOADBAR_MARGIN_X (4)
#define SPLASH_LOADBAR_HEIGHT_PX (20)
#define SPLASH_LOADBAR_FINAL_EMPTY_PX (5)

#define SPLASH_WAIT_HACK_FRAMES (3 * BADGE_FRAME_RATE_FPS)
#define SPLASH_WAIT_SPONSOR_FRAMES (3 * BADGE_FRAME_RATE_FPS)
#define SPLASH_WAIT_POST_LOADBAR_FRAMES (2 * BADGE_FRAME_RATE_FPS)
#define SPLASH_WORD_THING_FRAMES (3)
#define SPLASH_WAIT_BLINK_FRAMES (2 * BADGE_FRAME_RATE_FPS)
#define SPLASH_RVASEC_SILENT_FRAMES (3 * BADGE_FRAME_RATE_FPS)

#define SPLASH_BOOT_AUDIO_MS     (100)
#define SPLASH_FINISHED_AUDIO_MS (2000)

#define GLASS_RISE_PX (20)

#define SPLASH_PARTICLE_POOL_SIG (0xD00FF00D)
#define SPLASH_FIREWORKS (3)
#define SPLASH_FIREWORKS_COLOR (x11_YellowGreen)
#define SPLASH_FIREWORKS_GRAVITY (1)
#define SPLASH_FIREWORKS_GRAVITY_DECAY_FACTOR (5)
#define SPLASH_FIREWORKS_DECAY_SPEED (2)
#define SPLASH_FIREWORKS_DECAY_SPEED_FACTOR (1)


static const char *splash_word_things[] = {
    "Cognition Module",
    "useless bits",
    "backdoor.sh",
    "exploit inside",
    "lifting tables",
    "flipping tables",
    "personal data",
    "important bits",
    "bitcoin miner",
    "advanced AI(tm)",
    "broken feature",
    "NTFS",
    "ZFS",
    "BTRFS",
    "All the FS",
    "Wall hacks",
    "huawei 5G",
    "Key logger",
    "W****** Defender",
    "xz utils",
    "...wait not yet!",
    "preparing to install",
};

static unsigned int wait = 0;
static unsigned char loading_txt_idx = 0;
static enum splash_state {
    SPLASH_STATE_NO_INIT,
    SPLASH_STATE_LOADBAR,
    SPLASH_STATE_WAIT_FOR_USER,
    SPLASH_STATE_HACK,
    SPLASH_STATE_SPONSOR,
    SPLASH_STATE_RAVEN,
    SPLASH_STATE_RVASEC,
    SPLASH_STATE_DONE,
} m_splash_state = SPLASH_STATE_NO_INIT;

static struct particle_pool *m_sparks;

void prv_spark_xyz_from_aev(int32_t *x, int32_t *y, int32_t *z,
                            int32_t azimuth, int32_t elevation, int32_t velocity)
{
    while (azimuth < 0) azimuth += 128;
    while (elevation < 0) elevation += 128;
    *x = velocity * cosine(elevation) / 256 * cosine(azimuth) / 256;
    *y = velocity * cosine(elevation) / 256 * sine(azimuth) / 256;
    *z = velocity * sine(elevation) / 256;
}

void prv_start_fireworks(void)
{
    m_sparks = get_common_particle_pool();
    m_sparks->nparticles = 0;
    if (claim_particle_pool(m_sparks, SPLASH_PARTICLE_POOL_SIG)) {
        m_sparks->config.maxparticles = MAX_PARTICLES;
    }
    m_sparks->config.gravityy = 1;

    uint32_t random;
    random_insecure_bytes((void *) &random, sizeof(random));

    for (int i = 0; i < MAX_PARTICLES / SPLASH_FIREWORKS; i++) {
        for (int j = 0; j < SPLASH_FIREWORKS; j++) {
            random = random_insecure_u32_congruence(random);
            int azimuth = random % 128;
            random = random_insecure_u32_congruence(random);
            int elevation = (random % 64) - 32;
            random = random_insecure_u32_congruence(random);
            int vel = 100 + (random % 50);
            int vx, vy, vz;
            prv_spark_xyz_from_aev(&vx, &vy, &vz, azimuth, elevation, vel);
            m_sparks->config.add_3d_particle(
                m_sparks,
                ((((LCD_XSIZE - 40) / (SPLASH_FIREWORKS - 1)) * j) + 20) * 256, 20 * 256, 0,
                vx, vy, vz,
                INT16_MAX,
                SPLASH_FIREWORKS_COLOR);
        }
    }
}

void prv_slow_spark(struct particle *p)
{
    /* Don't slow to a stop. */
    int32_t vel = fxp_sqrt((p->vx * p->vx) + (p->vz * p->vz) + (p->vz * p->vz));
    if (vel <= 5) {
        return;
    }

    int azimuth = arctan2(p->vy, p->vx);
    int elevation = arctan2(p->vz, fxp_sqrt((p->vx * p->vx) + (p->vz * p->vz)));
    while (azimuth < 0) azimuth += 128;
    while (elevation < 0) elevation += 128;
    vel -= SPLASH_FIREWORKS_DECAY_SPEED;
    int vx, vy, vz;
    prv_spark_xyz_from_aev(&vx, &vy, &vz, azimuth, elevation, vel);
    p->vx = vx;
    p->vy = vy;
    p->vz = vz;
}

void prv_run_fireworks(void)
{
    m_sparks->config.move_particles(m_sparks);
    m_sparks->config.draw_particles(m_sparks);
    for (int i = 0; i < m_sparks->nparticles; i++) {
        struct particle *p = m_sparks->p + i;

        /* Slow down even minimum gravity. */
        if (0 != (wait % SPLASH_FIREWORKS_GRAVITY_DECAY_FACTOR)) {
            p->vy -= m_sparks->config.gravityy;
        }
        /* Decay speed. */
        if (0 != (wait % SPLASH_FIREWORKS_DECAY_SPEED_FACTOR)) {
            prv_slow_spark(p);
        }

        /* Decay color */
        for (int bits = 15; bits > 0; bits--) {
            int threshold = (1 << (bits - 1)) - 1;
            unsigned short color =
                PACKRGB(UNPACKR(SPLASH_FIREWORKS_COLOR) >> (5 - (bits * 5 / 15)),
                        UNPACKG(SPLASH_FIREWORKS_COLOR) >> (6 - (bits * 6 / 15)),
                        UNPACKB(SPLASH_FIREWORKS_COLOR) >> (5 - (bits * 5 / 15)));
            if (threshold <= p->life) {
                p->color = color;
                break;
            }
        }
    }
}

#if PREPRODUCTION_FIRMWARE
static void brand_preproduction_firmware(bool blink)
{
	if (!blink) {
            led_pwm_disable(BADGE_LED_RGB_RED);
            led_pwm_disable(BADGE_LED_RGB_GREEN);
            led_pwm_disable(BADGE_LED_RGB_BLUE);
            return;
        }

	FbColor(WHITE);
        FbBackgroundColor(RED);
	FbMove(13, 25);
	FbWriteString("ALPHA FIRMWARE");
	FbPushBuffer();

        led_pwm_enable(BADGE_LED_RGB_RED, 50 * 255 / 100);
        led_pwm_disable(BADGE_LED_RGB_GREEN);
        led_pwm_disable(BADGE_LED_RGB_BLUE);
}
#endif

static const struct audio_out_section *prv_intro_cb(const struct audio_out_section *prev)
{
    (void) prev;
    if (m_splash_state < SPLASH_STATE_RAVEN) {
        m_splash_state = SPLASH_STATE_RAVEN;
        return &RIB_THEME;
    } else if (m_splash_state == SPLASH_STATE_RAVEN) {
        m_splash_state = SPLASH_STATE_RVASEC;
        return &RIB_CRASHOUT;
    } else {
        m_splash_state = SPLASH_STATE_DONE;
        return NULL;
    }
}

static void prv_exit(void)
{
        FbBackgroundColor(BLACK);
        led_pwm_disable(BADGE_LED_RGB_RED);
        led_pwm_disable(BADGE_LED_RGB_GREEN);
        led_pwm_disable(BADGE_LED_RGB_BLUE);
        audio_out_stop(0);
        audio_out_music_stop();
        m_splash_state = SPLASH_STATE_NO_INIT;
        pop_app();
};

static unsigned prv_get_colormap_length(const struct asset2 *a)
{
    switch (a->type) {
        case PICTURE1BIT: return 1U << 1;
        case PICTURE2BIT: return 1U << 1;
        case PICTURE4BIT: return 1U << 4;
        case PICTURE8BIT: return 1U << 8;
        default: return 0;
    };
}

static unsigned prv_find_trans_idx(const struct asset2 *a, uint16_t color)
{
    unsigned len = prv_get_colormap_length(a);
    for (unsigned i = 0; i < len; i++) {
        if (a->colormap[i] == color) {
            return i;
        }
    }
    return 0;
};

void rvasec_splash_cb(__attribute__((unused)) struct badge_app *app)
{
    /* Allow the user to fast-forward at any point. */
    int down_latches = button_down_latches();
    if (down_latches & (1U << BADGE_BUTTON_FASTFORWARD)) {
        prv_exit();
        return;
    }

    switch (m_splash_state) {
        case SPLASH_STATE_NO_INIT: {
        loading_txt_idx = 0;
        wait = 0;
        m_sparks = NULL;
        display_rect(0, 0, LCD_XSIZE, LCD_YSIZE);
        display_color(0);
        FbSwapBuffers();

        led_pwm_disable(BADGE_LED_RGB_RED);
        led_pwm_disable(BADGE_LED_RGB_GREEN);
        led_pwm_disable(BADGE_LED_RGB_BLUE);
        const struct audio_out_spec spec = {
            .callback = NULL,
            .frequency_hz = NOTE_C3 / 2 ,
            .duration_ms = SPLASH_BOOT_AUDIO_MS,
            .envelope = 0,
            .phase = 0,
            .amplitude_dBFS = 3,
            .restart = false,
            .type = AUDIO_OUT_TYPE_SQUARE,
            .square.duty_cycle = UINT8_MAX / 2,
        };
#if TARGET_SIMULATOR
	if (!silent_startup)
#endif
        (void) audio_out_play(0, &spec);
        m_splash_state = SPLASH_STATE_LOADBAR;
    } break;

    case SPLASH_STATE_LOADBAR: {
        FbMove(24 + 16, SPLASH_SHIFT_DOWN - 13 - (SPLASH_LOADBAR_HEIGHT_PX / 2));
        FbColor(WHITE);
        FbWriteLine("Loading...");

        FbMove(SPLASH_LOADBAR_MARGIN_X,
               SPLASH_SHIFT_DOWN - SPLASH_LOADBAR_HEIGHT_PX / 2);
        FbColor(WHITE);
        char outer_x_sz = LCD_XSIZE - 1 - (SPLASH_LOADBAR_MARGIN_X * 2);
        FbRectangle(outer_x_sz, SPLASH_LOADBAR_HEIGHT_PX);

        FbMoveRelative(2 - outer_x_sz, 2 - SPLASH_LOADBAR_HEIGHT_PX);
        FbColor(GREEN);

        const unsigned int load_bar_frames 
            = (ARRAY_SIZE(splash_word_things) - 1) * SPLASH_WORD_THING_FRAMES;
        const unsigned char load_bar_size
            = LCD_XSIZE - ((SPLASH_LOADBAR_MARGIN_X + 1) * 2) - 1 
              - SPLASH_LOADBAR_FINAL_EMPTY_PX;
        const unsigned char load_bar_px 
            = MIN(load_bar_size * wait / load_bar_frames, load_bar_size);
        FbFilledRectangle(load_bar_px, 18);

        FbColor(WHITE);
        uint8_t load_bar_perc 
            = MIN(load_bar_px * 100 / load_bar_size, 99);
        char str[4];
        (void) snprintf(str, sizeof(str), "%02u%%", load_bar_perc);
        FbMove((LCD_XSIZE - (3 * 8)) / 2, 
                SPLASH_SHIFT_DOWN + ((SPLASH_LOADBAR_HEIGHT_PX - 8) / 2));
        FbWriteString(str);

        FbColor(WHITE);
        FbMove((LCD_XSIZE 
                - (strnlen(splash_word_things[loading_txt_idx], 160 / 8) * 8))
               / 2,
               SPLASH_SHIFT_DOWN + SPLASH_LOADBAR_HEIGHT_PX + 4);
        FbWriteLine(splash_word_things[loading_txt_idx]);
        FbSwapBuffers();

        if((loading_txt_idx < (ARRAY_SIZE(splash_word_things) - 1U))
           && (0 == (wait % SPLASH_WORD_THING_FRAMES))) {
            loading_txt_idx++;
        } else if (load_bar_frames + SPLASH_WAIT_POST_LOADBAR_FRAMES <= wait) {
            wait = 0;
            m_splash_state = SPLASH_STATE_WAIT_FOR_USER;
            led_pwm_enable(BADGE_LED_RGB_RED, 2 * 255/100);
            led_pwm_enable(BADGE_LED_RGB_GREEN, 20 * 255/100);
            led_pwm_disable(BADGE_LED_RGB_BLUE);
            break;
        }
        wait++;

    } break;

    case SPLASH_STATE_WAIT_FOR_USER: {
        FbBackgroundColor(BLACK);
        FbMove(16, (LCD_YSIZE - 16) / 2);
        FbColor(WHITE);
        FbWriteString("Press any button\nto continue...");
#if PREPRODUCTION_FIRMWARE
        brand_preproduction_firmware(!((wait / 5) & 0x01) 
                                     || (wait > SPLASH_WAIT_BLINK_FRAMES));
        wait++;
#endif
        FbSwapBuffers();
        if (down_latches) {
            wait = 0;
            m_splash_state = SPLASH_STATE_HACK;
            led_pwm_enable(BADGE_LED_RGB_RED, 50 * 255/100);
            led_pwm_enable(BADGE_LED_RGB_GREEN, 50 * 255/100);
            led_pwm_enable(BADGE_LED_RGB_BLUE, 50 * 255/100);
#if TARGET_SIMULATOR
            if (!silent_startup)
#endif
                (void) audio_out_music_play(&RIB_INTRO, prv_intro_cb);
        }
    } break;

    case SPLASH_STATE_HACK: {
        FbBackgroundColor(BLACK);
        FbMove((LCD_XSIZE - hack_logo.x) / 2,
               ((LCD_YSIZE - hack_logo.y) / 2));
        FbImage2(&hack_logo, 0);
        FbSwapBuffers();
        if ((SPLASH_WAIT_HACK_FRAMES < ++wait)) {
            wait = 0;
            m_splash_state = SPLASH_STATE_SPONSOR;
        }
    } break;

    case SPLASH_STATE_SPONSOR: {
        FbBackgroundColor(BLACK);
        FbColor(WHITE);
        FbMove((LCD_XSIZE - sponsor_logo.x) / 2,
               ((LCD_YSIZE - sponsor_logo.y) / 2));
        FbImage1bit2(&sponsor_logo, 0);
        FbSwapBuffers();
#if TARGET_SIMULATOR
        if (silent_startup && (SPLASH_WAIT_SPONSOR_FRAMES < ++wait)) {
            wait = 0;
            m_splash_state = SPLASH_STATE_RVASEC;
        }
#endif
    } break;

    case SPLASH_STATE_RAVEN: {
        FbBackgroundColor(BLACK);

        if (NULL == m_sparks) {
            prv_start_fireworks();
        } else {
            prv_run_fireworks();
        }

        FbColor(BLACK);
        FbMove(LCD_XSIZE - 120, LCD_YSIZE - 40);
        FbFilledRectangle(120, 40);

        FbTransparentIndex(prv_find_trans_idx(&rvasec15_raven, MAGENTA));
        FbMove((LCD_XSIZE - rvasec15_raven.x),
               ((LCD_YSIZE - rvasec15_raven.y)));
        FbImage2(&rvasec15_raven, 0);

        int rise = MIN(wait / BADGE_FRAME_RATE_FPS, GLASS_RISE_PX);
        FbTransparentIndex(prv_find_trans_idx(&rvasec15_glass, MAGENTA));
        FbMove(13,
               ((LCD_YSIZE - rvasec15_glass.y) + GLASS_RISE_PX - rise));
        FbImage2(&rvasec15_glass, 0);

        FbTransparentIndex(0);
        FbSwapBuffers();
        wait += 1;
    } break;

    case SPLASH_STATE_RVASEC: {
        FbBackgroundColor(BLACK);
        FbTransparentIndex(prv_find_trans_idx(&rvasec15_logo, MAGENTA));
        FbMove((LCD_XSIZE - rvasec15_logo.x) / 2,
               ((LCD_YSIZE - rvasec15_logo.y) / 2));
        FbImage2(&rvasec15_logo, 0);
        FbTransparentIndex(0);
        FbSwapBuffers();

        led_pwm_enable(BADGE_LED_RGB_RED, 15 * 255 / 100);
        led_pwm_enable(BADGE_LED_RGB_GREEN, 50 * 255 / 100);
        led_pwm_enable(BADGE_LED_RGB_BLUE, 10 * 255 / 100);
#if TARGET_SIMULATOR
	if (silent_startup && (SPLASH_RVASEC_SILENT_FRAMES < ++wait)) {
		/* We need this for --silent-start option to work, otherwise
		 * we get stuck in SPLASH_STATE_RVASEC because normally without
		 * --silent-start, it is the audio code that gets us to the
		 * done state.
		 */
		wait = 0;
		m_splash_state = SPLASH_STATE_DONE;
	}
#endif
    } break;

    case SPLASH_STATE_DONE: {
        prv_exit();
    } break;

    default:
        m_splash_state = SPLASH_STATE_NO_INIT;
        break;
    }
}


