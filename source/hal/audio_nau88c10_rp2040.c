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
#include <stdlib.h>

#include <pico/time.h>
#include <hardware/adc.h>
#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/irq.h>
#include <hardware/pio.h>
#include <hardware/sync.h>

#include "analog.h"
#include "pinout_rp2040.h"
#include "nau88c10_rp2040.h"
#include "badge.h"

#include "audio.h"
#include "utils.h"

/* TODO: add logging system? -PMW */
#ifndef LOG
#define LOG(...) printf("\r\n[audio] " __VA_ARGS__)
#endif /* LOG */

/*! @addtogroup BADGE_AUDIO Audio Driver
 *  @{
 */

#define AUDIO_OUT_BEEP_AMPLITUDE    (INT32_MAX)

static volatile enum audio_out_mode_ {
    AUDIO_OUT_MODE_OFF = 0,
    AUDIO_OUT_MODE_BEEP,
} m_audio_out_mode;

// FIXME: move this to audio_common.c. -PMW
static audio_input_callback_t m_audio_in_cb[AUDIO_INPUT_CALLBACKS_MAX];
static int m_audio_in_cb_count;

static struct audio_out_beep {
    uint16_t duration_ms;   /**< Duration in ms. */
    uint16_t period;        /**< Period in samples. */
    uint16_t samples_high;  /**< Samples high. */
    uint16_t elapsed_ms;    /**< Elapsed beep duration in ms. */
    uint16_t samples;       /**< Sample counter. */
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

static void prv_audio_out_beep_complete(struct audio_out_beep *beep)
{
    LOG("finished playing beep");
    m_audio_out_mode = AUDIO_OUT_MODE_OFF;
    audio_stby_ctl(true);
    if (NULL != beep->cb) {
        beep->cb();
    }
}

static int32_t prv_audio_out_beep_get_next_sample(struct audio_out_beep *beep)
{
    int32_t sample;
    uint32_t samples = beep->samples;
    uint32_t period = beep->period;
    if (samples < beep->samples_high) {
        sample = AUDIO_OUT_BEEP_AMPLITUDE;
    } else {
        sample = -AUDIO_OUT_BEEP_AMPLITUDE;
    }
    if (++samples >= period) {
        samples = 0;
    }
    beep->samples = samples;
    return sample;
}

static void prv_audio_i2s_process(int32_t *in, int32_t *out)
{
    /* Input samples. */
    if (0 < m_audio_in_cb_count) {
        audio_sample_t samples[AUDIO_BUFFER_FRAMES];
        for (size_t i = 0; i < AUDIO_BUFFER_FRAMES; i++) {
            samples[i] = in[1 + i * 2U];
        }
        for (unsigned i = 0; i < ARRAY_SIZE(m_audio_in_cb); i++) {
            if (NULL != m_audio_in_cb[i]) {
                m_audio_in_cb[i](samples, AUDIO_BUFFER_FRAMES);
            }
        }
    }
        
    /* Output samples. */
    if (AUDIO_OUT_MODE_BEEP == m_audio_out_mode) {
        struct audio_out_beep *beep = &m_audio_out_beep;
        if (beep->elapsed_ms < beep->duration_ms) {
            unsigned period = beep->period;
            if (UINT16_MAX != period) {
                /* Play the note. */
                for (size_t i = 0; i < STEREO_BUFFER_SIZE; i += 2) {
                    out[i] = prv_audio_out_beep_get_next_sample(beep);
                }
            } else {
                /* This is a rest. */
                for (size_t i = 0; i < STEREO_BUFFER_SIZE; i += 2) {
                    out[i] = 0U;
                }
            }

            /* Check if beep is finished. */
            if (++(beep->elapsed_ms) == beep->duration_ms) {
                prv_audio_out_beep_complete(beep);
            }
        } else {
            for (size_t i = 0; i < STEREO_BUFFER_SIZE; i += 2) {
                out[i] = 0U;
            }
        }
    } else {
        /* Nothing is playing. */
        for (size_t i = 0; i < STEREO_BUFFER_SIZE; i += 2) {
            out[i] = 0U;
        }
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
    prv_audio_i2s_process(p->input_buffer + offset, p->output_buffer + offset);
    dma_hw->ints0 = 1U << p->dma_ch_in_data;  // clear the IRQ
}

/*- Initialization -----------------------------------------------------------*/
void audio_init_gpio(void)
{
    nau88c10_set_cfg(&m_nau88c10_ctx, &NAU88C10_CFG);
    nau88c10_init(&m_nau88c10_ctx);
}

static void audio_out_init(void)
{
#if PREPRODUCTION_FIRMWARE && 0
    /* Make sure logs can be seen. */
    busy_wait_until(2000 * 1000);
#endif
    nau88c10_reset(&m_nau88c10_ctx);
    nau88c10_up(&m_nau88c10_ctx);
}

void audio_init(void)
{
    audio_out_init();
}

void audio_poll(void)
{
    /* Only update volume if the voume has changed by more than 2 percentage 
     * points. This helps filter noise on the ADC input. */
    uint8_t vol = analog_get_volume();
    if (abs((int) vol - (int) nau88c10_get_volume(&m_nau88c10_ctx)) > 1) {
        nau88c10_set_volume(&m_nau88c10_ctx, vol);
    }

    /* Check for new audio output configuration. */
    uint8_t aoc = badge_system_data()->audio_out_cfg;
    nau88c10_set_speaker_enabled(&m_nau88c10_ctx, (aoc & 0x1) == 0);
    nau88c10_set_headphone_enabled(&m_nau88c10_ctx, (aoc & 0x2) == 0);
}

/*- Standby Pin Control ------------------------------------------------------*/
void audio_stby_ctl(bool enable)
{
    /* Always take the opamp out of standby if requested */
    if (!enable && !badge_system_data()->audio_out_cfg)
    {
	// TODO - bring the codec out of sleep. -PMW
    }
    /* Only put the opamp into standby if nothing is using it */
    else if (enable && (m_audio_out_mode == AUDIO_OUT_MODE_OFF))
    {
	// TODO - put the codec into sleep. -PMW
    }
}

/*- Input --------------------------------------------------------------------*/
// FIXME: move this to audio_common.c. -PMW
int audio_in_add_cb(audio_input_callback_t cb)
{
    if (NULL == cb) {
        LOG("Cannot add NULL input callback.");
        return -EINVAL;
    } else if (ARRAY_SIZE(m_audio_in_cb) <= (unsigned) m_audio_in_cb_count) {
        LOG("No space to add input callback.");
        return -ENOMEM;
    } else {
        for (int i = 0; i < (int) ARRAY_SIZE(m_audio_in_cb); i++) {
            if (NULL == m_audio_in_cb[i]) {
                m_audio_in_cb_count++;
                m_audio_in_cb[i] = cb;
                LOG("Added input callback. (i: %d, count: %d)",
                    i, m_audio_in_cb_count);
                return i;
            }
        }
        LOG("Did not find space to add input callback.");
        return -ENOMEM;
    }
}

int audio_in_remove_cb(int i)
{
    if (((int) ARRAY_SIZE(m_audio_in_cb) <= i) || (i < 0)) {
        LOG("Input entry index out of range.");
        return -EINVAL;
    } else if (NULL == m_audio_in_cb[i]) {
        LOG("Input entry already empty.");
        return -ENOENT;
    } else {
        m_audio_in_cb[i] = NULL;
        m_audio_in_cb_count--;
        LOG("Removed input callback. (i: %d, count: %d)", i, m_audio_in_cb_count);
        return 0;
    }
}

int audio_in_cb_count()
{
    return m_audio_in_cb_count;
}

/*- Output -------------------------------------------------------------------*/
int audio_out_beep_with_cb(uint16_t freq_hz, uint16_t dur_ms, void (*cb)(void))
{
    uint32_t period;
    enum audio_out_mode_ out_mode = AUDIO_OUT_MODE_OFF;
    if ((freq_hz == 0) && (dur_ms == 0)) {
        /* Cancel the current beep. */
        period = UINT16_MAX;
    } else if (freq_hz == 0 && cb != NULL) { 
        /* We're being asked to play a rest. */
        out_mode = AUDIO_OUT_MODE_BEEP;
        period = UINT16_MAX;
    } else if ((freq_hz < AUDIO_BEEP_FREQ_HZ_MIN)
               || (freq_hz > AUDIO_BEEP_FREQ_HZ_MAX)
               || (dur_ms < AUDIO_BEEP_DUR_MS_MIN)
               || (dur_ms > AUDIO_BEEP_DUR_MS_MAX))
    {
        return -1;
    } else {
        out_mode = AUDIO_OUT_MODE_BEEP;
        period = AUDIO_FS / freq_hz;
    }

    audio_stby_ctl(false);
    uint32_t irqs = save_and_disable_interrupts(); // FIXME: irq locking insufficient with multiple cores. -PMW
    m_audio_out_mode = out_mode;
    m_audio_out_beep.duration_ms = dur_ms;
    m_audio_out_beep.elapsed_ms = 0;
    if (m_audio_out_beep.period != period) {
        m_audio_out_beep.period = period;
        m_audio_out_beep.samples_high = period / 2;
        m_audio_out_beep.samples = 0;
    }
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
