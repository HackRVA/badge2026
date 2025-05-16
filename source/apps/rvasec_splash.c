#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "audio.h"
#include "button.h"
#include "colors.h"
#include "display.h"
#include "framebuffer.h"
#include "led_pwm.h"
#include "menu.h"
#include "music.h"
#include "rvasec_splash.h"
#include "hack_logo_2.h"
#include "utils.h"
#include "badge.h"

#define SPLASH_SHIFT_DOWN 80
#define SPLASH_LOADBAR_MARGIN_X (4)
#define SPLASH_LOADBAR_HEIGHT_PX (20)
#define SPLASH_LOADBAR_FINAL_EMPTY_PX (5)

#define SPLASH_WAIT_HACK_FRAMES (2 * BADGE_FRAME_RATE_FPS)
#define SPLASH_WAIT_POST_LOADBAR_FRAMES (2 * BADGE_FRAME_RATE_FPS)
#define SPLASH_WORD_THING_FRAMES (3)
#define SPLASH_WAIT_BLINK_FRAMES (2 * BADGE_FRAME_RATE_FPS)

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

static const char splash_words_btn1[] = "Press any button";
static const char splash_words_btn2[] = "to continue!";

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

void rvasec_splash_cb(__attribute__((unused)) struct badge_app *app)
{
    extern const struct asset2 RVAsec_14;
    static unsigned int wait = 0;
    static unsigned char loading_txt_idx = 0;
    static enum splash_state {
        SPLASH_STATE_NO_INIT,
        SPLASH_STATE_HACK,
        SPLASH_STATE_LOADBAR,
        SPLASH_STATE_DONE,
    } m_splash_state = SPLASH_STATE_NO_INIT;

    switch (m_splash_state) {
    case SPLASH_STATE_NO_INIT:
        loading_txt_idx = 0;
        wait = 0;
        display_rect(0, 0, LCD_XSIZE, LCD_YSIZE);
        display_color(0);
        FbSwapBuffers();
        led_pwm_enable(BADGE_LED_RGB_RED, 50 * 255/100);
        led_pwm_enable(BADGE_LED_RGB_GREEN, 50 * 255/100);
        led_pwm_enable(BADGE_LED_RGB_BLUE, 50 * 255/100);
        //if(buzzer)
        audio_out_beep(NOTE_C3, 50);
        m_splash_state = SPLASH_STATE_HACK;
        break;

    case SPLASH_STATE_HACK:
        FbMove((LCD_XSIZE - hack_logo.x) / 2, 
               ((LCD_YSIZE - hack_logo.y) / 2));	
        FbImage2(&hack_logo, 0);
        FbSwapBuffers();
        if (SPLASH_WAIT_HACK_FRAMES < ++wait) {
            wait = 0;
            m_splash_state = SPLASH_STATE_LOADBAR;
        }
        break;

    case SPLASH_STATE_LOADBAR:
	// FbBackgroundColor(0x21c5);
        FbBackgroundColor(G_Fb.transIndex);
        FbMove(0, 0);
        FbImage2(&RVAsec_14, 0);

        FbMove(24 + 16, SPLASH_SHIFT_DOWN - 13);
        FbColor(WHITE);
        FbWriteLine("Loading...");

        FbMove(SPLASH_LOADBAR_MARGIN_X, SPLASH_SHIFT_DOWN);
        FbColor(WHITE);
        FbRectangle(LCD_XSIZE - 1 - (SPLASH_LOADBAR_MARGIN_X * 2), 
                    SPLASH_LOADBAR_HEIGHT_PX);

        FbMove(SPLASH_LOADBAR_MARGIN_X + 1, SPLASH_SHIFT_DOWN+1);
        FbColor(GREEN);

        const unsigned int load_bar_frames 
            = (ARRAY_SIZE(splash_word_things) - 1) * SPLASH_WORD_THING_FRAMES;
        const unsigned char load_bar_size
            = LCD_XSIZE - ((SPLASH_LOADBAR_MARGIN_X + 1) * 2) - 1 
              - SPLASH_LOADBAR_FINAL_EMPTY_PX;
        const unsigned char load_bar_px 
            = MIN(load_bar_size * wait / load_bar_frames, load_bar_size);
        FbFilledRectangle(load_bar_px, 18);

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
        if((loading_txt_idx < (ARRAY_SIZE(splash_word_things) - 1U))
           && (0 == (wait % SPLASH_WORD_THING_FRAMES))) {
            loading_txt_idx++;
        } else if (load_bar_frames + SPLASH_WAIT_POST_LOADBAR_FRAMES <= wait) {
            wait = 0;
            m_splash_state = SPLASH_STATE_DONE;
        }

	FbBackgroundColor(BLACK);
        FbSwapBuffers();

        led_pwm_enable(BADGE_LED_RGB_RED, 15 * 255 / 100);
        led_pwm_enable(BADGE_LED_RGB_GREEN, 50 * 255 / 100);
	led_pwm_enable(BADGE_LED_RGB_BLUE, 10 * 255 / 100);

        wait++;
        break;

    case SPLASH_STATE_DONE:
        FbMove(0, 0);
        FbImage2(&RVAsec_14, 0);

        FbColor(WHITE);
        // FbBackgroundColor(0x21c5);
    	FbBackgroundColor(G_Fb.transIndex);
        FbMove((LCD_XSIZE - ((ARRAY_SIZE(splash_words_btn1) - 1) * 8)) / 2, 
               SPLASH_SHIFT_DOWN);
        FbWriteLine(splash_words_btn1);
        FbMove((LCD_XSIZE - ((ARRAY_SIZE(splash_words_btn2) - 1) * 8)) / 2,
               SPLASH_SHIFT_DOWN + 8);
        FbWriteLine(splash_words_btn2);

#if PREPRODUCTION_FIRMWARE
        brand_preproduction_firmware(!((wait / 5) & 0x01) 
                                     || (wait > SPLASH_WAIT_BLINK_FRAMES));
#endif

        FbSwapBuffers();

        int down_latches = button_down_latches();
        if (0 != down_latches) {
            FbBackgroundColor(BLACK);
            led_pwm_disable(BADGE_LED_RGB_RED);
            m_splash_state = SPLASH_STATE_NO_INIT;
            pop_app();
        }

        wait++;
        break;

    default:
        m_splash_state = SPLASH_STATE_NO_INIT;
        break;
    }
}


