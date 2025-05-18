/*!
 *  @file   analog.h
 *  @author Peter Maxwell Warasila
 *  @date   April 2, 2024
 *
 *  @brief  RVASec Badge Analog Measurements Driver
 *
 *------------------------------------------------------------------------------
 *
 */

#ifndef BADGE_C_ANALOG_H
#define BADGE_C_ANALOG_H

#include <stdint.h>

/*! @defgroup   BADGE_ANALOG Analog Measurements Driver
 *  @{
 */

enum analog_channel {
    ANALOG_CHAN_0               = 0,
    ANALOG_CHAN_1               = 1,
    ANALOG_CHAN_2               = 2,
    ANALOG_CHAN_VOLUME          = 3,
    ANALOG_CHAN_MCU_TEMP        = 4,
};

void analog_init(void);

void analog_init_gpio(void);

uint32_t analog_get_chan_mV(enum analog_channel chan);

int8_t analog_calc_mcu_temp_C(uint32_t mV);

uint8_t analog_get_volume(void);

#if TARGET_SIMULATOR
/* For use by simulator, not badge apps */
struct analog_sim_values {
	int value[5]; /* indexed by enum analog_channel */
};

void analog_sensors_set_values(struct analog_sim_values values);

#endif
/*! @} */ // BADGE_ANALOG

#endif /* BADGE_C_ANALOG_H */

