/**
 *  @file   nau88c10_rp2040.h
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
#include <hardware/pio.h>

#include <i2c_rp2040.h>

struct nau88c10_cfg {
    /* I2C configuration. */
    i2c_inst_t *i2c_inst;
    unsigned int i2c_scl_pin;
    unsigned int i2c_sda_pin;
    unsigned int i2c_baud;
    
    /* I2S configuration. */
    unsigned int i2s_mclk_pin;
    unsigned int i2s_bclk_pin;
    unsigned int i2s_fs_pin;
    unsigned int i2s_dacin_pin;
    unsigned int i2s_adcout_pin;
    pio_hw_t *i2s_pio;
};

struct nau88c10_ctx {
    const struct nau88c10_cfg *cfg;
    struct i2c_ctx i2c_ctx;
};

void nau88c10_set_cfg(struct nau88c10_ctx *ctx,
                      const struct nau88c10_cfg *cfg);
void nau88c10_init(struct nau88c10_ctx *ctx);
void nau88c10_up(struct nau88c10_ctx *ctx);
