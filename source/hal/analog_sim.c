
/*!
 *  @file   analog_sim.c
 *  @author Peter Maxwell Warasila
 *  @date   April 8, 2024
 *
 *  @brief  RVASec Badge Analog Measurements Driver Simulator Implemenation
 *
 *------------------------------------------------------------------------------
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "analog.h"

/*- Private Variables --------------------------------------------------------*/
static struct analog_sim_values analog_values = { {
	0, /* Unused */
	0, /* Unused */
	0, /* Unused */
	1650, /* Volume, mV */
	706,  /* ~ 27 C, MCU temp */
} };

/*- API ----------------------------------------------------------------------*/
void analog_init(void)
{
    // nothing
}

void analog_init_gpio(void)
{
    // nothing
}

uint32_t analog_get_chan_mV(enum analog_channel channel)
{
    /* FIXME - implement channels. */
    switch (channel)
    {
        case ANALOG_CHAN_VOLUME:
        case ANALOG_CHAN_MCU_TEMP:
		return analog_values.value[channel];
        default:
            return 0;
    }
}

int8_t analog_calc_mcu_temp_C(uint32_t mV)
{
    float raw = mV;
    raw /= 1e3f;
    return 27 - ((raw - 0.706f) / 0.001721f);
}

uint8_t analog_get_volume_perc(void)
{
    return analog_values.value[ANALOG_CHAN_VOLUME] * 100U / 3300U;
}

void analog_sensors_set_values(struct analog_sim_values values)
{
    analog_values = values;
}

