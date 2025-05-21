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
#include "rtc.h"
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

/*- Private Macro ------------------------------------------------------------*/
#if PREPRODUCTION_FIRMWARE
#define AUDIO_IRQ_STATS 1
#else
#define AUDIO_IRQ_STATS 0
#endif /* PREPRODUCTION_FIRMWARE */

/*- Private Variables --------------------------------------------------------*/
static uint64_t m_audio_irq_accum_us;
static uint64_t m_audio_last_irq_start_us;

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

/*- Private Methods ----------------------------------------------------------*/
static void prv_audio_i2s_dma_handler(void)
{
#if AUDIO_IRQ_STATS
    uint64_t entry_us = rtc_get_us_since_boot();
#endif /* AUDIO_IRQ_STATS */
    struct pio_i2s *p = &m_nau88c10_ctx.pio_i2s;
    size_t offset;
    if (*(int32_t**)dma_hw->ch[p->dma_ch_in_ctrl].read_addr == p->input_buffer) {
        // It is inputting to the second buffer so we can overwrite the first
        offset = 0;
    } else {
        // It is currently inputting the first buffer, so we write to the second
        offset = STEREO_BUFFER_SIZE;
    }
    audio_process_buffer(p->input_buffer + offset, p->output_buffer + offset);
    dma_hw->ints0 = 1U << p->dma_ch_in_data;  // clear the IRQ
#if AUDIO_IRQ_STATS
    uint64_t exit_us = rtc_get_us_since_boot();
    m_audio_irq_accum_us += (exit_us - entry_us);
    uint64_t since_last_log_us = entry_us - m_audio_last_irq_start_us;
    if (since_last_log_us > (1000 * 1000)) {
        LOG("irq cpu load %llu%%", 
            m_audio_irq_accum_us * 100 / MAX(1,since_last_log_us));
        m_audio_irq_accum_us = 0;
        m_audio_last_irq_start_us = entry_us;
    }
#endif /* AUDIO_IRQ_STATS */
}

int32_t m_audio_lock_irqs;

/*- API ----------------------------------------------------------------------*/
/*----- Initialization -------------------------------------------------------*/
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

/*----- Runtime --------------------------------------------------------------*/
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

void audio_lock(void)
{
    // TODO: irq locking insufficient with multiple cores. -PMW
    m_audio_lock_irqs = save_and_disable_interrupts();
}

void audio_unlock(void)
{
    restore_interrupts(m_audio_lock_irqs);
}

/*! @} */ // BADGE_AUDIO

