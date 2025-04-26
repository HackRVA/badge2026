/**
 *  @file   i2c_rp2040.h
 *  @author Peter Maxwell Warasila
 *  @date   April 2, 2024
 *
 *  @brief  RP2040 I2C Driver Tools
 *
 *------------------------------------------------------------------------------
 *
 */

#ifndef I2C_RP2040_H
#define I2C_RP2040_H

#include <stdint.h>

#include <pico.h>
#include <hardware/i2c.h>

enum i2c_error_code {
    I2C_ERROR_NONE         = 0,
    I2C_ERROR_NOADACK      = 0x01,
    I2C_ERROR_TIMEOUT      = 0x02,
    I2C_ERROR_DATALEN      = 0x04,
    I2C_ERROR_MEMRD_WRITE  = 0x10,
    I2C_ERROR_MEMRD_READ   = 0x20,
    I2C_ERROR_RGMOD_MEMRD  = 0x40,
    I2C_ERROR_RGMOD_WRITE  = 0x80,
};

struct i2c_ctx {
    i2c_inst_t *i2c;
    uint8_t addr;
    enum i2c_error_code error_code;
};


int i2c_memrd(struct i2c_ctx *ctx, uint8_t reg, void *dst, size_t dst_sz,
              uint32_t timeout_us);
int i2c_regmodify(struct i2c_ctx *ctx, uint8_t reg, uint8_t mask,
                  uint8_t value, uint32_t timeout_us);

#endif /* I2C_RP2020_H */
