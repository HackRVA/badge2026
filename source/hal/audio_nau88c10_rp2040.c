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

#include <pico/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <pico/time.h>
#include <hardware/adc.h>
#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/irq.h>
#include <hardware/pio.h>
#include <hardware/sync.h>

#include "pinout_rp2040.h"
#include "nau88c10_rp2040.h"
#include "badge.h"

#include "audio.h"

/* TODO: add logging system? -PMW */
#ifndef LOG
#define LOG(...) printf("\r\n[audio] " __VA_ARGS__)
#endif /* LOG */

/*! @addtogroup BADGE_AUDIO Audio Driver
 *  @{
 */

#define AUDIO_OUT_BEEP_AMPLITUDE    (INT32_MAX / 4)

static volatile enum audio_out_mode_ {
    AUDIO_OUT_MODE_OFF = 0,
    AUDIO_OUT_MODE_BEEP,
} m_audio_out_mode;

static struct audio_out_beep {
    uint16_t duration_ms;   /**< Duration in ms. */
    uint16_t elapsed_ms;    /**< Elapsed beep duration in ms. */
    uint32_t period;        /**< Period in samples. */
    uint32_t samples;       /**< Sample counter. */
    void (*cb)(void);       /**< Callback on beep completion. */
} m_audio_out_beep;

static struct nau88c10_ctx m_nau88c10_ctx;
static void prv_audio_i2s_dma_handler(void); /* Forward declaration. */
static const struct nau88c10_cfg NAU88C10_CFG = {
    .i2c_inst = BADGE_I2C_AUDIO_CODEC,
    .i2c_scl_pin = BADGE_GPIO_AUDIO_CODEC_SCL,
    .i2c_sda_pin = BADGE_GPIO_AUDIO_CODEC_SDA,
    .i2c_baud = BADGE_I2C_AUDIO_CODEC_BAUD,
    .i2s_mclk_pin = BADGE_GPIO_AUDIO_CODEC_MCLK,
    .i2s_bclk_pin = BADGE_GPIO_AUDIO_CODEC_BCLK,
    .i2s_fs_pin = BADGE_GPIO_AUDIO_CODEC_FS,
    .i2s_dacin_pin = BADGE_GPIO_AUDIO_CODEC_DACIN,
    .i2s_adcout_pin = BADGE_GPIO_AUDIO_CODEC_ADCOUT,
    .i2s_pio = BADGE_PIO_AUDIO_CODEC,
    .i2s_dma_handler = prv_audio_i2s_dma_handler,
};

static int32_t prv_audio_out_beep_get_next_sample(struct audio_out_beep *beep)
{
    if (UINT32_MAX == beep->period) {
        return 0;
    } else {
        int32_t sample;
        uint32_t samples = beep->samples;
        uint32_t period = beep->period;
        if (samples < (period / 2)) {
            sample = AUDIO_OUT_BEEP_AMPLITUDE;
        } else {
            sample = -AUDIO_OUT_BEEP_AMPLITUDE;
        }
        if (++samples == period) {
            samples = 0;
        }
        beep->samples = samples;
        return sample;
    }
}

static void prv_audio_out_beep_complete(struct audio_out_beep *beep)
{
    LOG("finished playing beep");
    m_audio_out_mode = AUDIO_OUT_MODE_OFF;
    audio_stby_ctl(true);
    if (NULL != beep->cb) {
        beep->cb();
    }
}

static void prv_audio_i2s_process(int32_t *in, int32_t *out, size_t n)
{
    // TODO: use input samples when mic is working. -PMW
    (void) in;
        
    /* Output samples. */
    if (AUDIO_OUT_MODE_BEEP == m_audio_out_mode) {
        struct audio_out_beep *beep = &m_audio_out_beep;
        if (beep->elapsed_ms < beep->duration_ms) {
            for (size_t i = 0; i < n; i += 2) {
                out[i] = prv_audio_out_beep_get_next_sample(beep);
            }
            if (++(beep->elapsed_ms) == beep->duration_ms) {
                prv_audio_out_beep_complete(beep);
            }
        }
    } else {
        /* Nothing is playing. */
        memset(out, 0x00, n * sizeof(*out));
    };
}

static void prv_audio_i2s_dma_handler(void)
{
    struct pio_i2s *p = &m_nau88c10_ctx.pio_i2s;
    size_t offset;
    if (*(int32_t**)dma_hw->ch[p->dma_ch_in_ctrl].read_addr == p->input_buffer) {
        // It is inputting to the second buffer so we can overwrite the first
        offset = 0;
    } else {
        // It is currently inputting the first buffer, so we write to the second
        offset = STEREO_BUFFER_SIZE;
    }
    prv_audio_i2s_process(p->input_buffer + offset, 
                          p->output_buffer + offset, 
                          STEREO_BUFFER_SIZE);
    dma_hw->ints0 = 1u << p->dma_ch_in_data;  // clear the IRQ
}

/*- Initialization -----------------------------------------------------------*/
void audio_init_gpio(void)
{
    nau88c10_set_cfg(&m_nau88c10_ctx, &NAU88C10_CFG);
    nau88c10_init(&m_nau88c10_ctx);
}

static void audio_out_init(void)
{
#if PREPRODUCTION_FIRMWARE
    /* Make sure logs can be seen. */
    busy_wait_until(1000 * 1000);
#endif
    nau88c10_reset(&m_nau88c10_ctx);
    nau88c10_up(&m_nau88c10_ctx);

    // TODO - simple wave table synth for beeps? -PMW
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
    else if (enable && (m_audio_out_mode == AUDIO_OUT_MODE_OFF))
    {
	// TODO - put the codec into sleep. -PMW
    }
}

/*- Output -------------------------------------------------------------------*/
int audio_out_beep_with_cb(uint16_t freq_hz, uint16_t dur_ms, void (*cb)(void))
{
    uint32_t period;
    if ((freq_hz == 0) && (dur_ms == 0)) {
        /* Cancel the current beep. */
        period = UINT16_MAX;
    } else if (freq_hz == 0 && cb != NULL) { 
        /* We're being asked to play a rest. */
        period = UINT32_MAX;
    } else if ((freq_hz < AUDIO_BEEP_FREQ_HZ_MIN)
               || (freq_hz > AUDIO_BEEP_FREQ_HZ_MAX)
               || (dur_ms < AUDIO_BEEP_DUR_MS_MIN)
               || (dur_ms > AUDIO_BEEP_DUR_MS_MAX))
    {
        return -1;
    } else {
       period = AUDIO_FS / freq_hz;
    }

    audio_stby_ctl(false);
    uint32_t irqs = save_and_disable_interrupts(); // FIXME: irq locking insufficient with multiple cores. -PMW
    m_audio_out_mode = AUDIO_OUT_MODE_BEEP;
    m_audio_out_beep.duration_ms = dur_ms;
    m_audio_out_beep.elapsed_ms = 0;
    m_audio_out_beep.period = period;
    m_audio_out_beep.samples = 0;
    m_audio_out_beep.cb = cb;
    restore_interrupts(irqs);
    LOG("playing beep (freq: %d, period: %u, duration: %u)", 
         freq_hz, period, dur_ms);
    return 0;
}

int audio_out_beep(uint16_t freqHz, uint16_t durMs)
{
	return audio_out_beep_with_cb(freqHz, durMs, NULL);
}

bool audio_is_playing(void) {
    return m_audio_out_mode == AUDIO_OUT_MODE_BEEP;
}

/*! @} */ // BADGE_AUDIO
