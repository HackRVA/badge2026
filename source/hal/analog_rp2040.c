/*!
 *  @file   analog_rp2040.c
 *  @author Peter Maxwell Warasila
 *  @date   April 2, 2024
 *
 *  @brief  RVASec Badge Analog Measurements Driver RP2040 Implemenation
 *
 *------------------------------------------------------------------------------
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include <hardware/adc.h>

#include "analog.h"
#include "hardware/gpio.h"
#include "pinout_rp2040.h"

/*- Private Methods ----------------------------------------------------------*/
static uint16_t analog_get_adc_count(enum analog_channel channel)
{
    adc_select_input(channel);
    return adc_read();
}

/*- API ----------------------------------------------------------------------*/
void analog_init(void)
{
    adc_init();
    adc_set_temp_sensor_enabled(true);
}

void analog_init_gpio(void)
{
    adc_gpio_init(BADGE_GPIO_ADC_VOLUME);
}

uint32_t analog_get_chan_mV(enum analog_channel channel)
{
    uint32_t count = analog_get_adc_count(channel);
    count *= 3300;
    count /= 4096;
    return count;
}

int8_t analog_calc_mcu_temp_C(uint32_t mV)
{
    float raw = mV;
    raw /= 1e3f;
    return 27 - ((raw - 0.706f) / 0.001721f);
}

uint8_t analog_get_volume(void)
{
    uint16_t count = 4095U - analog_get_adc_count(ANALOG_CHAN_VOLUME);
    if (count > 4000) {
        return UINT8_MAX;
    } else if  (count < 100) {
        return 0;
    } else {
        return (((count - 100) * (UINT8_MAX - 1)) / (4000 - 100)) + 1;
    }
}

