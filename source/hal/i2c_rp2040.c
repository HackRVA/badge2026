/**
 *  @file   i2c_rp2040.c
 *  @author Peter Maxwell Warasila
 *  @date   April 2, 2024
 *
 *  @brief  RP2040 I2C Driver Tools
 *
 *------------------------------------------------------------------------------
 *
 */

#include <stdint.h>

#include <pico.h>
#include <hardware/i2c.h>
#include <i2c_rp2040.h>

static void set_i2c_error(struct i2c_ctx *ctx, int rc, int expected_length)
{
    enum i2c_error_code e = I2C_ERROR_NONE;
    if (PICO_ERROR_GENERIC == rc) {
        e = I2C_ERROR_NOADACK;
    } else if (PICO_ERROR_TIMEOUT == rc) {
        e = I2C_ERROR_TIMEOUT;
    } else if (rc != expected_length) {
        e = I2C_ERROR_DATALEN;
    }
    ctx->error_code |= e;
}

/*- API ----------------------------------------------------------------------*/
int i2c_memrd(struct i2c_ctx *ctx, uint8_t reg, void *dst, size_t dst_sz,
              uint32_t timeout_us)
{
    absolute_time_t t = make_timeout_time_us(timeout_us);
    int rc = i2c_write_blocking_until(ctx->i2c, ctx->addr,
                                      &reg, sizeof(reg),
                                      true, t);
    if ((rc < 0) || (rc != sizeof(reg))) {
        ctx->error_code |= I2C_ERROR_MEMRD_WRITE;
        set_i2c_error(ctx, rc, sizeof(reg));
        return rc;
    }

    rc = i2c_read_blocking_until(ctx->i2c, ctx->addr,
                                 dst, dst_sz,
                                 false, t);
    if (rc != (int) dst_sz) {
        ctx->error_code |= I2C_ERROR_MEMRD_READ;
        set_i2c_error(ctx, rc, dst_sz);
    }

    return rc;
}

int i2c_regmodify(struct i2c_ctx *ctx, uint8_t reg, uint8_t mask,
                  uint8_t value, uint32_t timeout_us)
{
    uint8_t data[2];
    data[0] = reg;

    absolute_time_t t = make_timeout_time_us(timeout_us);
    int rc = i2c_memrd(ctx, reg, &data[1], sizeof(data[1]), timeout_us);
    if (rc != sizeof(data[1])) {
        ctx->error_code |= I2C_ERROR_MEMRD_READ;
        set_i2c_error(ctx, rc, sizeof(data));
        return rc;
    }

    data[1] &= mask;
    data[1] |= value;

    rc = i2c_write_blocking_until(ctx->i2c, ctx->addr, 
                                  data, sizeof(data), false, t);
    if (rc != sizeof(data)) {
        ctx->error_code |= I2C_ERROR_RGMOD_WRITE;
        set_i2c_error(ctx, rc, sizeof(data));
        return rc;
    }

    return 1;
}

