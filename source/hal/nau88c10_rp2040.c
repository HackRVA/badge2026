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

#include <stdio.h>

#include <pico.h>
#include <hardware/i2c.h>
#include <hardware/gpio.h>
#include <hardware/pio.h>

#include "i2c_rp2040.h"
#include "nau88c10_rp2040.h"

#define NAU88C10_I2C_ADDR       (0x1AU)
#define NAU88C10_I2C_TIMEOUT_US (5000U)

#define NAU88C10_REG_RESET      (0x00U)
#define NAU88C10_REG_DEV_REV    (0x3EU)
#define NAU88C10_REG_TWID       (0x3FU)
#define NAU88C10_REG_ADD_ID     (0x40U)
#define NAU88C10_REG_RESERVED   (0x41U)

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

static int prv_nau_write_reg(struct nau88c10_ctx *ctx, uint8_t reg, uint16_t data)
{
    uint8_t tx[2];
    tx[0] = (reg << 1) | ((data & 0x100) >> 8);
    tx[1] = data & 0xff;
    return i2c_write_timeout_us(ctx->cfg->i2c_inst, NAU88C10_I2C_ADDR, tx,
                                sizeof(tx), false, NAU88C10_I2C_TIMEOUT_US);
}

static int prv_nau_read_reg(struct nau88c10_ctx *ctx, uint8_t reg)
{
    /* Send the register address. */
    reg <<= 1;
    int rc = i2c_write_timeout_us(ctx->cfg->i2c_inst, NAU88C10_I2C_ADDR, &reg,
                                  sizeof(reg), true, NAU88C10_I2C_TIMEOUT_US);
    if (rc < (int) sizeof(reg)) {
        return rc;
    }

    /* Read the register. */
    uint8_t rx[2];
    rc = i2c_read_timeout_us(ctx->cfg->i2c_inst, NAU88C10_I2C_ADDR, 
                             (void *) &rx, sizeof(rx), false, 
                             NAU88C10_I2C_TIMEOUT_US);
    if (rc < (int) sizeof(rx)) {
        return rc;
    }
    
    return (int) (rx[0] << 8) | rx[1];
}

/*- API ----------------------------------------------------------------------*/
void nau88c10_set_cfg(struct nau88c10_ctx *ctx, const struct nau88c10_cfg *cfg)
{
    ctx->cfg = cfg;
}

void nau88c10_init(struct nau88c10_ctx *ctx)
{
    prv_nau_init_hw(ctx);
}

void nau88c10_up(struct nau88c10_ctx *ctx)
{
    int rc;

    rc = prv_nau_write_reg(ctx, NAU88C10_REG_RESET, 0x1ff);
    if (rc < 0) {
        printf("[nau88c10] failed to write reset register: %d\r\n", rc);
        return;
    }
    printf("[nau88c10] wrote to reset register\r\n");

    rc = prv_nau_read_reg(ctx, NAU88C10_REG_DEV_REV);
    if (rc < 0) {
        printf("[nau88c10] failed to device revision: %d\r\n", rc);
        return;
    } 
    printf("[nau88c10] device revision: 0x%x\r\n", rc);
    
    rc = prv_nau_read_reg(ctx, NAU88C10_REG_TWID);
    if (rc < 0) {
        printf("[nau88c10] failed to two-wire id: %d\r\n", rc);
        return;
    } 
    printf("[nau88c10] twi id: 0x%x\r\n", rc);

    rc = prv_nau_read_reg(ctx, NAU88C10_REG_ADD_ID);
    if (rc < 0) {
        printf("[nau88c10] failed to additional id: %d\r\n", rc);
        return;
    } 
    printf("[nau88c10] additional id: 0x%x\r\n", rc);

    rc = prv_nau_read_reg(ctx, NAU88C10_REG_RESERVED);
    if (rc < 0) {
        printf("[nau88c10] failed to reserved register: %d\r\n", rc);
        return;
    } 
    printf("[nau88c10] reserved: 0x%x\r\n", rc);
}

