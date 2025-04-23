/*!
 *  @file   audio_nau88c10yg_rp2040.c
 *  @author Peter Maxwell Warasila
 *  @date   April 22, 2025
 *
 *  @brief  RVASec 2025 Badge Audio Implementation
 *
 *------------------------------------------------------------------------------
 *
 *  @note   Only basic output beep implemented!
 *
 */

#include <stdint.h>
#include <stdbool.h>

#include <pico/time.h>
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/dma.h>
#include <hardware/adc.h>
#include <hardware/clocks.h>
#include <hardware/i2c.h>
#include <hardware/pio.h>

#include "pinout_rp2040.h"
#include "badge.h"

#include "audio.h"

/*! @addtogroup BADGE_AUDIO Audio Driver
 *  @{
 */

static volatile enum audio_out_mode_ {
    AUDIO_OUT_MODE_OFF = 0,
    AUDIO_OUT_MODE_BEEP,
} audio_out_mode;

/*- Initialization -----------------------------------------------------------*/
void audio_init_gpio(void)
{
    /* Configure I2C controller. */
    i2c_init(BADGE_I2C_AUDIO_CODEC, BADGE_I2C_AUDIO_CODEC_BAUD);
    gpio_set_function(BADGE_GPIO_AUDIO_CODEC_SCL, GPIO_FUNC_I2C);
    gpio_set_function(BADGE_GPIO_AUDIO_CODEC_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(BADGE_GPIO_AUDIO_CODEC_SCL); /* Whoops I forgot I2C pull-ups! -PMW */
    gpio_pull_up(BADGE_GPIO_AUDIO_CODEC_SDA);

    /* Configure I2S pins. */
    gpio_init(BADGE_GPIO_AUDIO_CODEC_MCLK);
    gpio_set_input_enabled(BADGE_GPIO_AUDIO_CODEC_MCLK, false);
    gpio_set_slew_rate(BADGE_GPIO_AUDIO_CODEC_MCLK, GPIO_SLEW_RATE_FAST);
    gpio_set_drive_strength(BADGE_GPIO_AUDIO_CODEC_MCLK, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_dir(BADGE_GPIO_AUDIO_CODEC_MCLK, true);
    pio_gpio_init(BADGE_PIO_AUDIO_CODEC, BADGE_GPIO_AUDIO_CODEC_MCLK);
    gpio_disable_pulls(BADGE_GPIO_AUDIO_CODEC_MCLK);

    gpio_init(BADGE_GPIO_AUDIO_CODEC_BCLK);
    gpio_set_input_enabled(BADGE_GPIO_AUDIO_CODEC_BCLK, true);
    gpio_set_dir(BADGE_GPIO_AUDIO_CODEC_BCLK, false);
    pio_gpio_init(BADGE_PIO_AUDIO_CODEC, BADGE_GPIO_AUDIO_CODEC_BCLK);
    gpio_disable_pulls(BADGE_GPIO_AUDIO_CODEC_BCLK);

    gpio_init(BADGE_GPIO_AUDIO_CODEC_FS);
    gpio_set_input_enabled(BADGE_GPIO_AUDIO_CODEC_FS, true);
    gpio_set_dir(BADGE_GPIO_AUDIO_CODEC_FS, false);
    pio_gpio_init(BADGE_PIO_AUDIO_CODEC, BADGE_GPIO_AUDIO_CODEC_FS);
    gpio_disable_pulls(BADGE_GPIO_AUDIO_CODEC_FS);

    gpio_init(BADGE_GPIO_AUDIO_CODEC_DACIN);
    gpio_set_input_enabled(BADGE_GPIO_AUDIO_CODEC_DACIN, false);
    gpio_set_slew_rate(BADGE_GPIO_AUDIO_CODEC_DACIN, GPIO_SLEW_RATE_FAST);
    gpio_set_drive_strength(BADGE_GPIO_AUDIO_CODEC_DACIN, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_dir(BADGE_GPIO_AUDIO_CODEC_DACIN, true);
    pio_gpio_init(BADGE_PIO_AUDIO_CODEC, BADGE_GPIO_AUDIO_CODEC_DACIN);
    gpio_disable_pulls(BADGE_GPIO_AUDIO_CODEC_DACIN);

    gpio_init(BADGE_GPIO_AUDIO_CODEC_ADCOUT);
    gpio_set_input_enabled(BADGE_GPIO_AUDIO_CODEC_ADCOUT, true);
    gpio_set_dir(BADGE_GPIO_AUDIO_CODEC_ADCOUT, false);
    pio_gpio_init(BADGE_PIO_AUDIO_CODEC, BADGE_GPIO_AUDIO_CODEC_ADCOUT);
    gpio_disable_pulls(BADGE_GPIO_AUDIO_CODEC_ADCOUT);
}

static void audio_out_init(void)
{
    // TODO - simple wave table synth for beeps? -PMW

    /* Used for beep */
    alarm_pool_init_default();
}

void audio_init(void)
{
    audio_out_init();
}

/*- Standby Pin Control ------------------------------------------------------*/
void audio_stby_ctl(bool enable)
{
    /* Always take the opamp out of standby if requested */
    if (!enable && !badge_system_data()->mute)
    {
	// TODO - bring the codec out of sleep. -PMW
    }
    /* Only put the opamp into standby if nothing is using it */
    else if (enable && (audio_out_mode == AUDIO_OUT_MODE_OFF))
    {
	// TODO - put the codec into sleep. -PMW
    }
}

/*- Output -------------------------------------------------------------------*/
static int64_t audio_out_beep_alarm(__attribute__((unused)) alarm_id_t id,
                                    void* user_data)
{
    void (*beep_finished)(void) = user_data;

    if (audio_out_mode != AUDIO_OUT_MODE_BEEP)
    {
	if (beep_finished)
		beep_finished();
        return 0;
    }

    // TODO - stop playing beep -PMW
    audio_out_mode = AUDIO_OUT_MODE_OFF;
    audio_stby_ctl(true);
    if (beep_finished)
        beep_finished();
    return 0;
}

int audio_out_beep_with_cb(uint16_t freqHz, uint16_t durMs, void (*beep_finished)(void))
{
    static alarm_id_t prev_alarm;
    if (freqHz == 0 && beep_finished != NULL) { /* we're being asked to play a rest?  Ok. */
	// TODO - stop playing beep -PMW
        audio_out_mode = AUDIO_OUT_MODE_OFF;
        audio_stby_ctl(true);
        prev_alarm = alarm_pool_add_alarm_in_ms(alarm_pool_get_default(),
                                            durMs,
                                            audio_out_beep_alarm,
                                            beep_finished,
                                            true);
        return 0;
    }

    if ((freqHz < AUDIO_BEEP_FREQ_HZ_MIN)
        || (freqHz > AUDIO_BEEP_FREQ_HZ_MAX)
        || (durMs < AUDIO_BEEP_DUR_MS_MIN)
        || (durMs > AUDIO_BEEP_DUR_MS_MAX))
    {
        return -1;
    }

    if (AUDIO_OUT_MODE_BEEP == audio_out_mode)
    {
        alarm_pool_cancel_alarm(alarm_pool_get_default(), prev_alarm);
    }
    // TODO - Start playing beep. -PMW
    audio_out_mode = AUDIO_OUT_MODE_BEEP;
    audio_stby_ctl(false);
    prev_alarm = alarm_pool_add_alarm_in_ms(alarm_pool_get_default(),
                                            durMs,
                                            audio_out_beep_alarm,
                                            beep_finished,
                                            true);
    return 0;
}

int audio_out_beep(uint16_t freqHz, uint16_t durMs)
{
	return audio_out_beep_with_cb(freqHz, durMs, NULL);
}

bool audio_is_playing(void) {
    return audio_out_mode == AUDIO_OUT_MODE_BEEP;
}

/*! @} */ // BADGE_AUDIO
