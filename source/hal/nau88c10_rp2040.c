/**
 *  @file   nau88c10_rp2040.c
 *  @author Peter Maxwell Warasila
 *  @date   April 26, 2025
 *
 *  @brief  NAU88C10 codec driver
 *
 *------------------------------------------------------------------------------
 *
 */

#include <pico.h>
#include <hardware/i2c.h>
#include <hardware/gpio.h>
#include <hardware/pio.h>

#include "i2c_rp2040.h"
#include "nau88c10_rp2040.h"

static void prv_nau_init_hw(struct nau88c10_ctx *ctx)
{
    const struct nau88c10_cfg *cfg = ctx->cfg;

    /* Configure I2C controller. */
    i2c_init(cfg->i2c_inst, cfg->i2c_baud);
    gpio_set_function(cfg->i2c_sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(cfg->i2c_scl_pin, GPIO_FUNC_I2C);

    /* Whoops I forgot I2C pull-ups! -PMW */
    gpio_pull_up(cfg->i2c_scl_pin);
    gpio_pull_up(cfg->i2c_sda_pin);

    /* Configure I2S pins. */
    gpio_init(cfg->i2s_mclk_pin);
    gpio_set_input_enabled(cfg->i2s_mclk_pin, false);
    gpio_set_slew_rate(cfg->i2s_mclk_pin, GPIO_SLEW_RATE_FAST);
    gpio_set_drive_strength(cfg->i2s_mclk_pin, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_dir(cfg->i2s_mclk_pin, true);
    pio_gpio_init(cfg->i2s_pio, cfg->i2s_mclk_pin);
    gpio_disable_pulls(cfg->i2s_mclk_pin);

    gpio_init(cfg->i2s_bclk_pin);
    gpio_set_input_enabled(cfg->i2s_bclk_pin, true);
    gpio_set_dir(cfg->i2s_bclk_pin, false);
    pio_gpio_init(cfg->i2s_pio, cfg->i2s_bclk_pin);
    gpio_disable_pulls(cfg->i2s_bclk_pin);

    gpio_init(cfg->i2s_fs_pin);
    gpio_set_input_enabled(cfg->i2s_fs_pin, true);
    gpio_set_dir(cfg->i2s_fs_pin, false);
    pio_gpio_init(cfg->i2s_pio, cfg->i2s_fs_pin);
    gpio_disable_pulls(cfg->i2s_fs_pin);

    gpio_init(cfg->i2s_dacin_pin);
    gpio_set_input_enabled(cfg->i2s_dacin_pin, false);
    gpio_set_slew_rate(cfg->i2s_dacin_pin, GPIO_SLEW_RATE_FAST);
    gpio_set_drive_strength(cfg->i2s_dacin_pin, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_dir(cfg->i2s_dacin_pin, true);
    pio_gpio_init(cfg->i2s_pio, cfg->i2s_dacin_pin);
    gpio_disable_pulls(cfg->i2s_dacin_pin);

    gpio_init(cfg->i2s_adcout_pin);
    gpio_set_input_enabled(cfg->i2s_adcout_pin, true);
    gpio_set_dir(cfg->i2s_adcout_pin, false);
    pio_gpio_init(cfg->i2s_pio, cfg->i2s_adcout_pin);
    gpio_disable_pulls(cfg->i2s_adcout_pin);
}

/*- API ----------------------------------------------------------------------*/
void nau88c10_set_cfg(struct nau88c10_ctx *ctx, const struct nau88c10_cfg *cfg)
{
    ctx->cfg = cfg;
    ctx->i2c_ctx.i2c = cfg->i2c_inst;
}

void nau88c10_init(struct nau88c10_ctx *ctx)
{
    prv_nau_init_hw(ctx);
}

