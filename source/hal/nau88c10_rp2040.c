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

#include <stdint.h>
#include <stdio.h>

#include <pico.h>
#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/irq.h>
#include <hardware/pio.h>
#include <i2s/rp2040_i2s_example/i2s.h>

#include "utils.h"
#include "nau88c10_rp2040.h"

/* TODO: add logging system? -PMW */
extern int log_audio;
#ifndef LOG
#define LOG(...) do { if (log_audio) { printf("\r\n[nau88c10] " __VA_ARGS__); } } while (0)
#endif /* LOG */

/*- Private Macro ------------------------------------------------------------*/
#define NAU88C10_I2C_ADDR       (0x1AU)
#define NAU88C10_I2C_TIMEOUT_US (5000U)

#define NAU88C10_VOL_KNEE_VAL   (64)
#define NAU88C10_VOL_KNEE_GAIN  (NAU88C10_DACGAIN_NEG_DBFS(30))

/*- Private Types ------------------------------------------------------------*/
enum nau88c10_reg {
    NAU88C10_REG_SOFTWARE_RESET         = 0x00U,

    /* Power Management */
    NAU88C10_REG_POWER_MANAGEMENT_1     = 0x01U,
    NAU88C10_REG_POWER_MANAGEMENT_2     = 0x02U,
    NAU88C10_REG_POWER_MANAGEMENT_3     = 0x03U,

    /* Audio Control */
    NAU88C10_REG_AUDIO_INTERFACE        = 0x04U,
    NAU88C10_REG_COMPANDING             = 0x05U,
    NAU88C10_REG_CLOCK_CONTROL_1        = 0x06U,
    NAU88C10_REG_CLOCK_CONTROL_2        = 0x07U,
    NAU88C10_REG_DAC_CTRL               = 0x0AU,
    NAU88C10_REG_DAC_VOLUME             = 0x0BU,
    NAU88C10_REG_ADC_CTRL               = 0x0EU,
    NAU88C10_REG_ADC_VOLUME             = 0x0FU,

    /* Equaliser */
    NAU88C10_REG_EQ1_LOW_CUTOFF         = 0x12U,
    NAU88C10_REG_EQ2_PEAK_1             = 0x13U,
    NAU88C10_REG_EQ3_PEAK_2             = 0x14U,
    NAU88C10_REG_EQ4_PEAK_3             = 0x15U,
    NAU88C10_REG_EQ5_HIGH_CUTOFF        = 0x16U,

    /* Digital to Analog (DAC) Limiter */
    NAU88C10_REG_DAC_LIMITER_1          = 0x18U,
    NAU88C10_REG_DAC_LIMITER_2          = 0x19U,

    /* Notch Filter */
    NAU88C10_REG_NOTCH_FILTER_0_HIGH    = 0x1BU,
    NAU88C10_REG_NOTCH_FILTER_0_LOW     = 0x1CU,
    NAU88C10_REG_NOTCH_FILTER_1_HIGH    = 0x1DU,
    NAU88C10_REG_NOTCH_FILTER_1_LOW     = 0x1EU,

    /* ALC Control */
    NAU88C10_REG_ALC_CTRL_1             = 0x20U,
    NAU88C10_REG_ALC_CTRL_2             = 0x21U,
    NAU88C10_REG_ALC_CTRL_3             = 0x22U,
    NAU88C10_REG_NOISE_GATE             = 0x23U,

    /* PLL Control */
    NAU88C10_REG_PLL_N_CTRL             = 0x24U,
    NAU88C10_REG_PLL_K_1                = 0x25U,
    NAU88C10_REG_PLL_K_2                = 0x26U,
    NAU88C10_REG_PLL_K_3                = 0x27U,

    /* Input, Ouput & Mixer Control */
    NAU88C10_REG_ATTENUATION_CTRL       = 0x28U,
    NAU88C10_REG_INPUT_CTRL             = 0x2CU,
    NAU88C10_REG_PGA_GAIN               = 0x2DU,
    NAU88C10_REG_ADC_BOOST              = 0x2FU,
    NAU88C10_REG_OUTPUT_CTRL            = 0x31U,
    NAU88C10_REG_MIXER_CTRL             = 0x32U,
    NAU88C10_REG_SPKOUT_VOLUME          = 0x36U,
    NAU88C10_REG_MONO_MIXER_CONTROL     = 0x38U,

    /* Low Power Control */
    NAU88C10_REG_POWER_MANAGEMENT_4     = 0x3AU,

    /* PCM Time Slot & ADCOUT Impedance Option Control */
    NAU88C10_REG_TIME_SLOT              = 0x3BU,
    NAU88C10_REG_ADCOUT_DRIVE           = 0x3CU,

    /* Register ID */
    NAU88C10_REG_SILICON_REVISION       = 0x3EU,
    NAU88C10_REG_2_WIRE_ID              = 0x3FU,
    NAU88C10_REG_ADDITIONAL_ID          = 0x40U,

    /* Reserved */
    NAU88C10_REG_RESERVED               = 0x41U,

    /* Output Driver Control */
    NAU88C10_REG_HIGH_VOLTAGE_CTRL      = 0x45U,

    /* Automatic Level Control Enhancements */
    NAU88C10_REG_ALC_ENHANCEMENTS_1     = 0x46U,
    NAU88C10_REG_ALC_ENHANCEMENTS_2     = 0x47U,

    /* Misc */
    NAU88C10_REG_ADDITIONAL_IF_CTRL     = 0x49U,
    NAU88C10_REG_POWER_TIE_OFF_CTRL     = 0x4BU,
    NAU88C10_REG_AGC_P2P_DETECTOR       = 0x4CU,
    NAU88C10_REG_AGC_PEAK_DETECTOR      = 0x4DU,
    NAU88C10_REG_CONTROL_AND_STATUS     = 0x4EU,
    NAU88C10_REG_OUTPUT_TIE_OFF_CTRL    = 0x4FU,
};

/** VREF Impedance Selection. */
enum nau88c10_refimp {
    NAU88C10_REFIMP_DISABLE = 0U,
    NAU88C10_REFIMP_80K     = 1U,
    NAU88C10_REFIMP_300K    = 2U,
    NAU88C10_REFIMP_3K      = 3U,
};

#define NAU88C10_REFIMP_POS     (0)
#define NAU88C10_REFIMP_MASK    (0x3U << NAU88C10_REFIMP_POS)

/** Unused input/output tie off buffer enable. */
enum nau88c10_iobufen {
    NAU88C10_IOBUFEN_DISABLE    = 0U,
    NAU88C10_IOBUFEN_ENABLE     = 1U,
};

#define NAU88C10_IOBUFEN_POS    (2)
#define NAU88C10_IOBUFEN_MASK   (0x1U << NAU88C10_IOBUFEN_POS)

/** Analog amplifier bias control. */
enum nau88c10_abiasen {
    NAU88C10_ABIASEN_DISABLE    = 0U,
    NAU88C10_ABIASEN_ENABLE     = 1U,
};

#define NAU88C10_ABIASEN_POS    (3)
#define NAU88C10_ABIASEN_MASK   (0x1U << NAU88C10_ABIASEN_POS)

/** Microphone bias enable. */
enum nau88c10_micbiasen {
    NAU88C10_MICBIASEN_DISABLE  = 0U,
    NAU88C10_MICBIASEN_ENABLE   = 1U,
};

#define NAU88C10_MICBIASEN_POS   (4)
#define NAU88C10_MICBIASEN_MASK  (0x1U << NAU88C10_MICBIASEN_POS)

/** PLL enable. */
enum nau88c10_pllen {
    NAU88C10_PLLEN_DISABLE  = 0U,
    NAU88C10_PLLEN_ENABLE   = 1U,
};

#define NAU88C10_PLLEN_POS  (5)
#define NAU88C10_PLLEN_MASK (0x1U << NAU88C10_PLLEN_POS)

/* Buffer for DC level sifting (required for 1.5x gain). */
enum nau88c10_dcbufen {
    NAU88C10_DCBUFEN_DISABLE    = 0U,
    NAU88C10_DCBUFEN_ENABLE     = 1U,
};

#define NAU88C10_DCBUFEN_POS    (8)
#define NAU88C10_DCBUFEN_MASK   (0x1U << NAU88C10_DCBUFEN_POS)

/** ADC enable. */
enum nau88c10_adcen {
    NAU88C10_ADCEN_DISABLE  = 0U,
    NAU88C10_ADCEN_ENABLE   = 1U,
};

#define NAU88C10_ADCEN_POS  (0)
#define NAU88C10_ADCEN_MASK (0x1U << NAU88C10_ADCEN_POS)

/** MIC(+/-) PGA Enable */
enum nau88c10_pgaen {
    NAU88C10_PGAEN_DISABLE  = 0U,
    NAU88C10_PGAEN_ENABLE   = 1U,
};

#define NAU88C10_PGAEN_POS  (2)
#define NAU88C10_PGAEN_MASK (0x1U << NAU88C10_PGAEN_POS)

/** Input Boost Enable. */
enum nau88c10_bsten {
    NAU88C10_BSTEN_STAGE_DISABLE    = 0U,
    NAU88C10_BSTEN_STAGE_ENABLE     = 1U,
};

#define NAU88C10_BSTEN_POS  (4)
#define NAU88C10_BSTEN_MASK (0x1U << NAU88C10_BSTEN_POS)

/** DAC enable. */
enum nau88c10_dacen {
    NAU88C10_DACEN_DISABLE  = 0U,
    NAU88C10_DACEN_ENABLE   = 1U,
};

#define NAU88C10_DACEN_POS  (0)
#define NAU88C10_DACEN_MASK (0x1U << NAU88C10_DACEN_POS)

/** Speaker Mixer Enable. */
enum nau88c10_spkmxen {
    NAU88C10_SPKMXEN_DISABLE    = 0U,
    NAU88C10_SPKMXEN_ENABLE     = 1U,
};

#define NAU88C10_SPKMXEN_POS    (2)
#define NAU88C10_SPKMXEN_MASK   (0x1U << NAU88C10_SPKMXEN_POS)

/** Mono Mixer Enable. */
enum nau88c10_moutmxen {
    NAU88C10_MOUTMXEN_DISABLE   = 0U,
    NAU88C10_MOUTMXEN_ENABLE    = 1U,
};

#define NAU88C10_MOUTMXEN_POS   (3)
#define NAU88C10_MOUTMXEN_MASK  (0x1U << NAU88C10_MOUTMXEN_POS)

/** SPKOUT+ Enable. */
enum nau88c10_pspken {
    NAU88C10_PSPKEN_DISABLE = 0U,
    NAU88C10_PSPKEN_ENABLE  = 1U,
};

#define NAU88C10_PSPKEN_POS     (5)
#define NAU88C10_PSPKEN_MASK    (0x1U << NAU88C10_PSPKEN_POS)

/** SPKOUT- Enable. */
enum nau88c10_nspken {
    NAU88C10_NSPKEN_DISABLE = 0U,
    NAU88C10_NSPKEN_ENABLE  = 1U,
};

#define NAU88C10_NSPKEN_POS     (6)
#define NAU88C10_NSPKEN_MASK    (0x1U << NAU88C10_NSPKEN_POS)

/** MOUT Enable. */
enum nau88c10_mouten {
    NAU88C10_MOUTEN_DISABLE = 0U,
    NAU88C10_MOUTEN_ENABLE  = 1U,
};

#define NAU88C10_MOUTEN_POS     (7)
#define NAU88C10_MOUTEN_MASK    (0x1U << NAU88C10_MOUTEN_POS)

/** ADC data frame phase. */
enum nau88c10_adcphs {
    NAU88C10_ADCPHS_LEFT    = 0U, /**< ADC data appears in the 'left' phase of the frame. */
    NAU88C10_ADCPHS_RIGHT   = 1U, /**< ADC data appears in the 'right' phase of the frame. */
};

#define NAU88C10_ADCPHS_POS     (1)
#define NAU88C10_ADCPHS_MASK    (0x1U << NAU88C10_ADCPHS_POS)

/** DAC data frame phase. */
enum nau88c10_dacphs {
    NAU88C10_DACPHS_LEFT    = 0U, /**< DAC data appears in the 'left' phase of the frame. */
    NAU88C10_DACPHS_RIGHT   = 1U, /**< DAC data appears in the 'right' phase of the frame. */
};

#define NAU88C10_DACPHS_POS     (2)
#define NAU88C10_DACPHS_MASK    (0x1U << NAU88C10_DACPHS_POS)

/** Audio Data Format Select */
enum nau88c10_aifmt {
    NAU88C10_AIFMT_RIGHT_JUSTIFIED  = 0U,
    NAU88C10_AIFMT_LEFT_JUSTIFIED   = 1U,
    NAU88C10_AIFMT_I2S              = 2U,
    NAU88C10_AIFMT_PCM_A            = 3U,
};

#define NAU88C10_AIFMT_POS  (3)
#define NAU88C10_AIFMT_MASK (0x3U << NAU88C10_AIFMT_POS)

/** Word length selection. */
enum nau88c10_wlen {
    NAU88C10_WLEN_16    = 0U,
    NAU88C10_WLEN_20    = 1U,
    NAU88C10_WLEN_24    = 2U,
    NAU88C10_WLEN_32    = 3U,
};

#define NAU88C10_WLEN_POS   (5)
#define NAU88C10_WLEN_MASK  (0x3U << NAU88C10_WLEN_POS)

/** Frame clock polarity. */
enum nau88c10_fsp {
    NAU88C10_FSP_NORMAL     = 0U,
    NAU88C10_FSP_INVERTED   = 1U,
};

#define NAU88C10_FSP_POS    (7)
#define NAU88C10_FSP_MASK   (0x1U << NAU88C10_FSP_POS)

/** Bit clock polarity. */
enum nau88c10_bclkp {
    NAU88C10_BCLKP_NORMAL   = 0U,
    NAU88C10_BCLKP_INVERTED = 1U,
};

#define NAU88C10_BCLKP_POS  (8)
#define NAU88C10_BCLKP_MASK (0x1U << NAU88C10_BCLKP_POS)

/** ADC output data to DAC input data passthrough. */
enum nau88c10_addap {
    NAU88C10_ADDAP_DISABLE  = 0U,
    NAU88C10_ADDAP_ENABLE   = 1U,
};

#define NAU88C10_ADDAP_POS  (0)
#define NAU88C10_ADDAP_MASK (0x1U << NAU88C10_ADDAP_POS)

/** ADC companding selection. */
enum nau88c10_adccm {
    NAU88C10_ADCCM_DISABLED = 0U,
    NAU88C10_ADCCM_RESERVED = 1U,
    NAU88C10_ADCCM_U_LAW    = 2U,
    NAU88C10_ADCCM_A_LAW    = 3U,
};

#define NAU88C10_ADCCM_POS  (1)
#define NAU88C10_ADCCM_MASK (0x3U << NAU88C10_ADCCM_POS)

/** DAC companding selection. */
enum nau88c10_daccm {
    NAU88C10_DACCM_DISABLED = 0U,
    NAU88C10_DACCM_RESERVED = 1U,
    NAU88C10_DACCM_U_LAW    = 2U,
    NAU88C10_DACCM_A_LAW    = 3U,
};

#define NAU88C10_DACCM_POS  (3)
#define NAU88C10_DACCM_MASK (0x3U << NAU88C10_DACCM_POS)

/** Frame and BCLK direction. */
enum nau88c10_clkioen {
    NAU88C10_CLKIOEN_SLAVE  = 0U,
    NAU88C10_CLKIOEN_MASTER = 1U,
};

#define NAU88C10_CLKIOEN_POS    (0)
#define NAU88C10_CLKIOEN_MASK   (0x1U << NAU88C10_CLKIOEN_POS)

/** Bit clock divider selection. */
enum nau88c10_bclksel {
    NAU88C10_BCLKSEL_DIV_1  = 0U,
    NAU88C10_BCLKSEL_DIV_2  = 1U,
    NAU88C10_BCLKSEL_DIV_4  = 2U,
    NAU88C10_BCLKSEL_DIV_8  = 3U,
    NAU88C10_BCLKSEL_DIV_16 = 4U,
    NAU88C10_BCLKSEL_DIV_32 = 5U,
    NAU88C10_BCLKSEL_RES_0  = 6U,
    NAU88C10_BCLKSEL_RES_1  = 7U,
};

#define NAU88C10_BCLKSEL_POS    (2)
#define NAU88C10_BCLKSEL_MASK   (0x7U << NAU88C10_BCLKSEL_POS)

/** Master clock selection. */
enum nau88c10_mclksel {
    NAU88C10_MCLKSEL_DIV_1      = 0U,
    NAU88C10_MCLKSEL_DIV_1_5    = 1U,
    NAU88C10_MCLKSEL_DIV_2      = 2U,
    NAU88C10_MCLKSEL_DIV_3      = 3U,
    NAU88C10_MCLKSEL_DIV_4      = 4U,
    NAU88C10_MCLKSEL_DIV_6      = 5U,
    NAU88C10_MCLKSEL_DIV_8      = 6U,
    NAU88C10_MCLKSEL_DIV_12     = 7U,
};

#define NAU88C10_MCLKSEL_POS    (5)
#define NAU88C10_MCLKSEL_MASK   (0x7U << NAU88C10_MCLKSEL_POS)

/** Source of Internal Clock */
enum nau88c10_clkm {
    NAU88C10_CLKM_PLL_BYPASSED = 0U,
    NAU88C10_CLKM_PLL_OUTPUT   = 1U,
};

#define NAU88C10_CLKM_POS   (8)
#define NAU88C10_CLKM_MASK  (0x1U << NAU88C10_CLKM_POS)

/** Slow clock enable.
 *
 *  Used for zero-cross timeout.
 */
enum nau88c10_sclken {
    NAU88C10_SCLKEN_MCLK       = 0U, /**< Uses MCLK */
    NAU88C10_SCLKEN_PLL_OUTPUT = 1U, /**< Uses PLL output (Period of 2^21 * MCLK). */
};

#define NAU88C10_SCLKEN_POS     (0)
#define NAU88C10_SCLKEN_MASK    (0x1U << NAU88C10_SCLKEN_POS)

/** Sample rate selection.
 *
 *  @note   This does not set or configure the actual sample rate. It only
 *          configures the coefficients for the internal digital samples to
 *          match the actual sample rate.
 */
enum nau88c10_smplr {
    NAU88C10_SMPLR_48_KHZ       = 0U,
    NAU88C10_SMPLR_32_KHZ       = 1U,
    NAU88C10_SMPLR_24_KHZ       = 2U,
    NAU88C10_SMPLR_16_KHZ       = 3U,
    NAU88C10_SMPLR_12_KHZ       = 4U,
    NAU88C10_SMPLR_8_KHZ        = 5U,
    NAU88C10_SMPLR_RESERVED_0   = 6U,
    NAU88C10_SMPLR_RESERVED_1   = 7U,
};

#define NAU88C10_SMPLR_POS  (1)
#define NAU88C10_SMPLR_MASK (0x7U << NAU88C10_SMPLR_POS)

/** DAC output polarity. */
enum nau88c10_dacpl {
    NAU88C10_DACPL_NORMAL               = 0U,
    NAU88C10_DACPL_DAC_OUTPUT_INVERTED  = 1U,
};

#define NAU88C10_DACPL_POS  (0)
#define NAU88C10_DACPL_MASK (0x1U << NAU88C10_DACPL_POS)

/** DAC auto mute. */
enum nau88c10_automt {
    NAU88C10_AUTOMT_DISABLE = 0U,
    NAU88C10_AUTOMT_ENABLE  = 1U,
};

#define NAU88C10_AUTOMT_POS     (2)
#define NAU88C10_AUTOMT_MASK    (0x1U << NAU88C10_AUTOMT_POS)

/** DAC over sample rate. */
enum nau88c10_dacos {
    NAU88C10_DACOS_64X  = 0U, /**< 64X oversampling for lower power. */
    NAU88C10_DACOS_128X = 1U, /**< 128X oversampling for better SNR. */
};

#define NAU88C10_DACOS_POS  (3)
#define NAU88C10_DACOS_MASK (0x1U << NAU88C10_DACOS_POS)

/** DAC de-emphasis. */
enum nau88c10_deemp {
    NAU88C10_DEEMP_NONE     = 0U, /**< No de-emphasis. */
    NAU88C10_DEEMP_32_KHZ   = 1U, /**< 32 kHz sample rate. */
    NAU88C10_DEEMP_44_1_KHZ = 2U, /**< 44.1 kHz sample rate. */
    NAU88C10_DEEMP_48_KHZ   = 3U, /**< 48 kHz sample rate. */
};

#define NAU88C10_DEEMP_POS  (4)
#define NAU88C10_DEEMP_MASK (0x3U << NAU88C10_DEEMP_POS)

/** DAC soft mute. */
enum nau88c10_dacmt {
    NAU88C10_DACMT_DISABLE  = 0U,
    NAU88C10_DACMT_ENABLE   = 0U,
};

#define NAU88C10_DACMT_POS  (3)
#define NAU88C10_DACMT_MASK (0x1U << NAU88C10_DACMT_POS)

/** DAC Gain. */
enum nau88c10_dacgain {
    NAU88C10_DACGAIN_DIGITAL_MUTE   = 0x00U,
    NAU88C10_DACGAIN_LSB_PER_0_5_DB = 1U,
    NAU88C10_DACGAIN_0_DBFS         = 0xFFU,
};

#define NAU88C10_DACGAIN_POS    (0)
#define NAU88C10_DACGAIN_MASK   (0xFFU << NAU88C10_DACGAIN_POS)
#define NAU88C10_DACGAIN_NEG_DBFS(NDBFS) \
    (NAU88C10_DACGAIN_0_DBFS - (NAU88C10_DACGAIN_LSB_PER_0_5_DB * 2U * (NDBFS)))

/** ADC Polarity. */
enum nau88c10_adcpl {
    NAU88C10_ADCPL_NORMAL   = 0U,
    NAU88C10_ADCPL_INVERTED = 1U,
};

#define NAU88C10_ADCPL_POS  (1)
#define NAU88C10_ADCPL_MASK (0x1U << NAU88C10_ADCPL_POS)

/** ADC over sample rate */
enum nau88c10_adcos {
    NAU88C10_ADCOS_64X  = 0U, /**< 64X oversampling for lower power. */
    NAU88C10_ADCOS_128X = 1U, /**< 128X oversampling for better SNR. */
};

#define NAU88C10_ADCOS_POS  (4)
#define NAU88C10_ADCOS_MASK (0x1U << NAU88C10_ADCOS_POS)

/** ADC input high pass filter frequency.
 *
 *  Consult section 12.3.7 of the NAU88C10 datasheet for details.
 */
enum nau88c10_hpf {
    NAU88C10_HPF_FS_8K_082_HZ   = 0x0U,
    NAU88C10_HPF_FS_8K_102_HZ   = 0x1U,
    NAU88C10_HPF_FS_8K_131_HZ   = 0x2U,
    NAU88C10_HPF_FS_8K_163_HZ   = 0x3U,
    NAU88C10_HPF_FS_8K_204_HZ   = 0x4U,
    NAU88C10_HPF_FS_8K_261_HZ   = 0x5U,
    NAU88C10_HPF_FS_8K_327_HZ   = 0x6U,
    NAU88C10_HPF_FS_8K_408_HZ   = 0x7U,

    NAU88C10_HPF_FS_11_025K_113_HZ = 0x0U,
    NAU88C10_HPF_FS_11_025K_141_HZ = 0x1U,
    NAU88C10_HPF_FS_11_025K_180_HZ = 0x2U,
    NAU88C10_HPF_FS_11_025K_225_HZ = 0x3U,
    NAU88C10_HPF_FS_11_025K_281_HZ = 0x4U,
    NAU88C10_HPF_FS_11_025K_360_HZ = 0x5U,
    NAU88C10_HPF_FS_11_025K_450_HZ = 0x6U,
    NAU88C10_HPF_FS_11_025K_563_HZ = 0x7U,

    NAU88C10_HPF_FS_12K_112_HZ  = 0x0U,
    NAU88C10_HPF_FS_12K_153_HZ  = 0x1U,
    NAU88C10_HPF_FS_12K_156_HZ  = 0x2U,
    NAU88C10_HPF_FS_12K_245_HZ  = 0x3U,
    NAU88C10_HPF_FS_12K_306_HZ  = 0x4U,
    NAU88C10_HPF_FS_12K_392_HZ  = 0x5U,
    NAU88C10_HPF_FS_12K_490_HZ  = 0x6U,
    NAU88C10_HPF_FS_12K_612_HZ  = 0x7U,

    NAU88C10_HPF_FS_16K_082_HZ  = 0x0U,
    NAU88C10_HPF_FS_16K_102_HZ  = 0x1U,
    NAU88C10_HPF_FS_16K_131_HZ  = 0x2U,
    NAU88C10_HPF_FS_16K_163_HZ  = 0x3U,
    NAU88C10_HPF_FS_16K_204_HZ  = 0x4U,
    NAU88C10_HPF_FS_16K_261_HZ  = 0x5U,
    NAU88C10_HPF_FS_16K_327_HZ  = 0x6U,
    NAU88C10_HPF_FS_16K_408_HZ  = 0x7U,

    NAU88C10_HPF_FS_22_05K_113_HZ   = 0x0U,
    NAU88C10_HPF_FS_22_05K_141_HZ   = 0x1U,
    NAU88C10_HPF_FS_22_05K_180_HZ   = 0x2U,
    NAU88C10_HPF_FS_22_05K_225_HZ   = 0x3U,
    NAU88C10_HPF_FS_22_05K_281_HZ   = 0x4U,
    NAU88C10_HPF_FS_22_05K_360_HZ   = 0x5U,
    NAU88C10_HPF_FS_22_05K_450_HZ   = 0x6U,
    NAU88C10_HPF_FS_22_05K_563_HZ   = 0x7U,

    NAU88C10_HPF_FS_24K_112_HZ = 0x0U,
    NAU88C10_HPF_FS_24K_153_HZ = 0x1U,
    NAU88C10_HPF_FS_24K_156_HZ = 0x2U,
    NAU88C10_HPF_FS_24K_245_HZ = 0x3U,
    NAU88C10_HPF_FS_24K_306_HZ = 0x4U,
    NAU88C10_HPF_FS_24K_392_HZ = 0x5U,
    NAU88C10_HPF_FS_24K_490_HZ = 0x6U,
    NAU88C10_HPF_FS_24K_612_HZ = 0x7U,

    NAU88C10_HPF_FS_32K_082_HZ  = 0x0U,
    NAU88C10_HPF_FS_32K_102_HZ  = 0x1U,
    NAU88C10_HPF_FS_32K_131_HZ  = 0x2U,
    NAU88C10_HPF_FS_32K_163_HZ  = 0x3U,
    NAU88C10_HPF_FS_32K_204_HZ  = 0x4U,
    NAU88C10_HPF_FS_32K_261_HZ  = 0x5U,
    NAU88C10_HPF_FS_32K_327_HZ  = 0x6U,
    NAU88C10_HPF_FS_32K_408_HZ  = 0x7U,

    NAU88C10_HPF_FS_44_1K_113_HZ    = 0x0U,
    NAU88C10_HPF_FS_44_1K_141_HZ    = 0x1U,
    NAU88C10_HPF_FS_44_1K_180_HZ    = 0x2U,
    NAU88C10_HPF_FS_44_1K_225_HZ    = 0x3U,
    NAU88C10_HPF_FS_44_1K_281_HZ    = 0x4U,
    NAU88C10_HPF_FS_44_1K_360_HZ    = 0x5U,
    NAU88C10_HPF_FS_44_1K_450_HZ    = 0x6U,
    NAU88C10_HPF_FS_44_1K_563_HZ    = 0x7U,

    NAU88C10_HPF_FS_48K_112_HZ = 0x0U,
    NAU88C10_HPF_FS_48K_153_HZ = 0x1U,
    NAU88C10_HPF_FS_48K_156_HZ = 0x2U,
    NAU88C10_HPF_FS_48K_245_HZ = 0x3U,
    NAU88C10_HPF_FS_48K_306_HZ = 0x4U,
    NAU88C10_HPF_FS_48K_392_HZ = 0x5U,
    NAU88C10_HPF_FS_48K_490_HZ = 0x6U,
    NAU88C10_HPF_FS_48K_612_HZ = 0x7U,
};

#define NAU88C10_HPF_POS    (5)
#define NAU88C10_HPF_MASK   (0x7U << NAU88C10_HPF_POS)

/** HPF Audio or Application Mode. */
enum nau88c10_hpfam {
    NAU88C10_HPFAM_AUDIO        = 0U,   /**< Fixed first order HPF w/fc @ 3.7 kHz. */
    NAU88C10_HPFAM_APPLICATION  = 1U,   /**< Second order HPF w/fc selected by HFP. */
};

#define NAU88C10_HPFAM_POS  (7)
#define NAU88C10_HPFAM_MASK (0x1U << NAU88C10_HPFAM_POS)

enum nau88c10_hpfen {
    NAU88C10_HPFEN_DISABLED = 0U,
    NAU88C10_HPFEN_ENABLED  = 1U,
};

#define NAU88C10_HPFEN_POS  (8)
#define NAU88C10_HPFEN_MASK (0x1U << NAU88C10_HPFEN_POS)

/** ADC Gain. */
enum nau88c10_adcgain {
    NAU88C10_ADCGAIN_UNUSED         = 0x00U,
    NAU88C10_ADCGAIN_LSB_PER_0_5_DB = 1U,
    NAU88C10_ADCGAIN_0_DBFS         = 0xFFU,
};

#define NAU88C10_ADCGAIN_POS  (0)
#define NAU88C10_ADCGAIN_MASK (0xFFU << NAU88C10_ADCGAIN_POS)
#define NAU88C10_ADCGAIN_NEG_DBFS(NDBFS) \
    MIN(MAX((NAU88C10_ADCGAIN_LSB_PER_0_5_DB * 2U * (NDBFS)), \
            NAU88C10_ADCGAIN_UNUSED + 1U), \
        NAU88C10_ADC_GAIN_0_DBFS)

/** Equalizer Gain. 
 *
 * Valid for EQ1GC, EQ2GC, EQ3GC, EQ4GC, and EQ5GC.
 */
enum nau88c10_eqxgc {
    NAU88C10_EQXGC_PLUS_12_DB   = 0x00U,
    NAU88C10_EQXGC_PLUS_11_DB   = 0x01U,
    NAU88C10_EQXGC_PLUS_10_DB   = 0x02U,
    NAU88C10_EQXGC_PLUS_9_DB    = 0x03U,
    NAU88C10_EQXGC_PLUS_8_DB    = 0x04U,
    NAU88C10_EQXGC_PLUS_7_DB    = 0x05U,
    NAU88C10_EQXGC_PLUS_6_DB    = 0x06U,
    NAU88C10_EQXGC_PLUS_5_DB    = 0x07U,
    NAU88C10_EQXGC_PLUS_4_DB    = 0x08U,
    NAU88C10_EQXGC_PLUS_3_DB    = 0x09U,
    NAU88C10_EQXGC_PLUS_2_DB    = 0x0AU,
    NAU88C10_EQXGC_PLUS_1_DB    = 0x0BU,
    NAU88C10_EQXGC_0_DB         = 0x0CU,
    NAU88C10_EQXGC_MINUS_1_DB   = 0x0DU,
    NAU88C10_EQXGC_MINUS_2_DB   = 0x0EU,
    NAU88C10_EQXGC_MINUS_3_DB   = 0x0FU,
    NAU88C10_EQXGC_MINUS_4_DB   = 0x10U,
    NAU88C10_EQXGC_MINUS_5_DB   = 0x11U,
    NAU88C10_EQXGC_MINUS_6_DB   = 0x12U,
    NAU88C10_EQXGC_MINUS_7_DB   = 0x13U,
    NAU88C10_EQXGC_MINUS_8_DB   = 0x14U,
    NAU88C10_EQXGC_MINUS_9_DB   = 0x15U,
    NAU88C10_EQXGC_MINUS_10_DB  = 0x16U,
    NAU88C10_EQXGC_MINUS_11_DB  = 0x17U,
    NAU88C10_EQXGC_MINUS_12_DB  = 0x18U,
};

/** Equalizer center/cut-off frequency. 
 *
 *  Some of these are valid for only one of EQ1CF, EQ2CF, EQ3CF, EQ4CF, or 
 *  EQ5CF.
 */
enum nau88c10_eqxcf {
    NAU88C10_EQ1CF_80_HZ    = 0x0U,
    NAU88C10_EQ1CF_105_HZ   = 0x1U,
    NAU88C10_EQ1CF_135_HZ   = 0x2U,
    NAU88C10_EQ1CF_175_HZ   = 0x3U,
    NAU88C10_EQ2CF_230_HZ   = 0x0U,
    NAU88C10_EQ2CF_300_HZ   = 0x1U,
    NAU88C10_EQ2CF_385_HZ   = 0x2U,
    NAU88C10_EQ2CF_500_HZ   = 0x3U,
    NAU88C10_EQ3CF_650_HZ   = 0x0U,
    NAU88C10_EQ3CF_850_HZ   = 0x1U,
    NAU88C10_EQ3CF_1_1_KHZ  = 0x2U,
    NAU88C10_EQ3CF_1_4_KHZ  = 0x3U,
    NAU88C10_EQ4CF_1_8_KHZ  = 0x0U,
    NAU88C10_EQ4CF_2_4_KHZ  = 0x1U,
    NAU88C10_EQ4CF_3_2_KHZ  = 0x2U,
    NAU88C10_EQ4CF_4_1_KHZ  = 0x3U,
    NAU88C10_EQ5CF_5_3_KHZ  = 0x0U,
    NAU88C10_EQ5CF_6_9_KHZ  = 0x1U,
    NAU88C10_EQ5CF_9_0_KHZ  = 0x2U,
    NAU88C10_EQ5CF_11_7_KHZ = 0x3U,
};

/** Equalizer path. */
enum nau88c10_eqm {
    NAU88C10_EQM_ADC    = 0U, /**< Equalizer in the ADC path. */
    NAU88C10_EQM_DAC    = 1U, /**< Equalizer in the DAC path. */
};

/** Bandwitdh control.
 *
 *  Valid for EQ2BW, EQ3BW, and EQ4BW.
 */
enum nau88c10_eqxbw {
    NAU88C10_EQXBW_NARROW   = 0U,
    NAU88C10_EQXBW_WIDE     = 1U,
};

/** EQ configuration. */
struct nau88c10_eq_cfg {
    enum nau88c10_eqm eqm;
    enum nau88c10_eqxgc eq1gc;
    enum nau88c10_eqxcf eq1cf;
    enum nau88c10_eqxgc eq2gc;
    enum nau88c10_eqxcf eq2cf;
    enum nau88c10_eqxbw eq2bw;
    enum nau88c10_eqxgc eq3gc;
    enum nau88c10_eqxcf eq3cf;
    enum nau88c10_eqxbw eq3bw;
    enum nau88c10_eqxgc eq4gc;
    enum nau88c10_eqxcf eq4cf;
    enum nau88c10_eqxbw eq4bw;
    enum nau88c10_eqxgc eq5gc;
    enum nau88c10_eqxcf eq5cf;
};

/** DAC Limiter Attack Time.
 *
 *  Attack time per 6 dB gain change.
 *
 *  @note   All times nominal for 44.1 kHz sample rate.
 */
enum nau88c10_daclimatk {
    NAU88C10_DACLIMATK_68_US    = 0U,   /**< 68 microseconds. */
    NAU88C10_DACLIMATK_136_US   = 1U,   /**< 136 microseconds. */
    NAU88C10_DACLIMATK_272_US   = 2U,   /**< 272 microseconds. @note Default value. */
    NAU88C10_DACLIMATK_544_US   = 3U,   /**< 544 microseconds. */
    NAU88C10_DACLIMATK_1_1_MS   = 4U,   /**< 1.1 milliseconds. */
    NAU88C10_DACLIMATK_2_2_MS   = 5U,   /**< 2.2 milliseconds. */
    NAU88C10_DACLIMATK_4_4_MS   = 6U,   /**< 4.4 milliseconds. */
    NAU88C10_DACLIMATK_8_7_MS   = 7U,   /**< 8.7 milliseconds. */
    NAU88C10_DACLIMATK_17_4_MS  = 8U,   /**< 17.4 milliseconds. */
    NAU88C10_DACLIMATK_35_MS    = 9U,   /**< 35 milliseconds. */
    NAU88C10_DACLIMATK_69_6_MS  = 10U,  /**< 69.6 milliseconds. */
    NAU88C10_DACLIMATK_139_MS   = 11U,  /**< 139 microseconds. */
};

#define NAU88C10_DACLIMATK_POS  (0)
#define NAU88C10_DACLIMATK_MASK (0xFU << NAU88C10_DACLIMATK_POS)

/** DAC Limiter Decay Time.
 *
 *  Decay time per 6 dB gain change.
 *
 *  @note   All times nominal for 44.1 kHz sample rate.
 */
enum nau88c10_daclimdcy {
    NAU88C10_DACLIMDCY_544_US   = 0U,   /**< 544 microseconds. */
    NAU88C10_DACLIMDCY_1_1_MS   = 1U,   /**< 1.1 milliseconds. */
    NAU88C10_DACLIMDCY_2_2_MS   = 2U,   /**< 2.2 milliseconds. */
    NAU88C10_DACLIMDCY_4_4_MS   = 3U,   /**< 4.4 milliseconds. @note Default value. */
    NAU88C10_DACLIMDCY_8_7_MS   = 4U,   /**< 8.7 milliseconds. */
    NAU88C10_DACLIMDCY_17_4_MS  = 5U,   /**< 17.4 milliseconds. */
    NAU88C10_DACLIMDCY_35_MS    = 6U,   /**< 35 milliseconds. */
    NAU88C10_DACLIMDCY_69_6_MS  = 7U,   /**< 69.6 milliseconds. */
    NAU88C10_DACLIMDCY_139_MS   = 8U,   /**< 139 microseconds. */
    NAU88C10_DACLIMDCY_278_5_MS = 9U,   /**< 278.5 milliseconds. */
    NAU88C10_DACLIMDCY_557_MS   = 10U,  /**< 557 microseconds. */
    NAU88C10_DACLIMDCY_1_1_S    = 11U,  /**< 1.1 seconds. */
};

#define NAU88C10_DACLIMDCY_POS  (4)
#define NAU88C10_DACLIMDCY_MASK (0xFU << NAU88C10_DACLIMDCY_POS)

/** DAC Limiter Enable. */
enum nau88c10_daclimen {
    NAU88C10_DACLIMEN_DISABLED  = 0U,
    NAU88C10_DACLIMEN_ENABLED   = 1U,
};

#define NAU88C10_DACLIMEN_POS   (8)
#define NAU88C10_DACLIMEN_MASK  (0x1U << NAU88C10_DACLIMEN_POS)

/** DAC Limiter Boost. */
enum nau88c10_daclimbst {
    NAU88C10_DACLIMBST_0_DB         = 0,
    NAU88C10_DACLIMBST_PLUS_1_DB    = 1,
    NAU88C10_DACLIMBST_PLUS_2_DB    = 2,
    NAU88C10_DACLIMBST_PLUS_3_DB    = 3,
    NAU88C10_DACLIMBST_PLUS_4_DB    = 4,
    NAU88C10_DACLIMBST_PLUS_5_DB    = 5,
    NAU88C10_DACLIMBST_PLUS_6_DB    = 6,
    NAU88C10_DACLIMBST_PLUS_7_DB    = 7,
    NAU88C10_DACLIMBST_PLUS_8_DB    = 8,
    NAU88C10_DACLIMBST_PLUS_9_DB    = 9,
    NAU88C10_DACLIMBST_PLUS_10_DB   = 10,
    NAU88C10_DACLIMBST_PLUS_11_DB   = 11,
    NAU88C10_DACLIMBST_PLUS_12_DB   = 12,
    NAU88C10_DACLIMBST_MAX          = NAU88C10_DACLIMBST_PLUS_12_DB,
};

#define NAU88C10_DACLIMBST_POS  (0)
#define NAU88C10_DACLIMBST_MASK (0xFU << NAU88C10_DACLIMBST_POS)

/** DAC Limiter Threshold Level. */
enum nau88c10_daclimthl {
    NAU88C10_DACLIMTHL_MINUS_1_DB   = 0,
    NAU88C10_DACLIMTHL_MINUS_2_DB   = 1,
    NAU88C10_DACLIMTHL_MINUS_3_DB   = 2,
    NAU88C10_DACLIMTHL_MINUS_4_DB   = 3,
    NAU88C10_DACLIMTHL_MINUS_5_DB   = 4,
    NAU88C10_DACLIMTHL_MINUS_6_DB   = 5,
};

#define NAU88C10_DACLIMTHL_POS  (4)
#define NAU88C10_DACLIMTHL_MASK (0x7U << NAU88C10_DACLIMTHL_POS)

/** ADC Input Boost (from PGA) */
enum nau88c10_pgabst {
    NAU88C10_PGABST_0_DB    = 0U,   /**< +0 dB from PGA to ADC. */
    NAU88C10_PGABST_20_DB   = 1U,   /**< +20 dB from PGA to ADC. */
};

#define NAU88C10_PGABST_POS     (8)
#define NAU88C10_PGABST_MASK    (0x1U << NAU88C10_PGABST_POS)

/*- Private Variables --------------------------------------------------------*/
static const char NOTHING[] = "";
static const char FAILED_TO[] = "failed to ";
static const char ENABLED[] = "enabled";
static const char DISABLED[] = "disabled";
static const char LEFT[] = "left";
static const char RIGHT[] = "right";
static const char NORMAL[] = "normal";
static const char INVERTED[] = "inverted";

/*- Private Methods ----------------------------------------------------------*/
static int prv_nau_write_reg(struct nau88c10_ctx *ctx, uint8_t reg, uint16_t data)
{
    uint8_t tx[2];
    tx[0] = (reg << 1) | ((data & 0x100) >> 8);
    tx[1] = data & 0xff;
    int rc = i2c_write_timeout_us(ctx->cfg->i2c_inst, NAU88C10_I2C_ADDR, tx,
                                  sizeof(tx), false, NAU88C10_I2C_TIMEOUT_US);
    LOG("%s register (rc: %d, reg: 0x%02X, data: 0x%03X)",
        rc < 0 ? "failed to write" : "wrote", rc, reg, data);
    return rc;
}

static int prv_nau_send_reg(struct nau88c10_ctx *ctx, enum nau88c10_reg reg)
{
    return prv_nau_write_reg(ctx, reg, ctx->reg[reg]);
}

static int prv_nau_read_reg(struct nau88c10_ctx *ctx, uint8_t reg)
{
    /* Send the register address. */
    reg <<= 1;
    int rc = i2c_write_timeout_us(ctx->cfg->i2c_inst, NAU88C10_I2C_ADDR, &reg,
                                  sizeof(reg), true, NAU88C10_I2C_TIMEOUT_US);
    if (rc < (int) sizeof(reg)) {
        LOG("failed to write register address when reading (rc: %d)", rc);
        return rc;
    }

    /* Read the register. */
    uint8_t rx[2];
    rc = i2c_read_timeout_us(ctx->cfg->i2c_inst, NAU88C10_I2C_ADDR, 
                             (void *) &rx, sizeof(rx), false, 
                             NAU88C10_I2C_TIMEOUT_US);
    if (rc < (int) sizeof(rx)) {
        LOG("failed to read when reading (rc: %d)", rc);
        return rc;
    }
    
    uint16_t val = (rx[0] << 8) | rx[1];
    LOG("read register (reg: 0x%02X, val: 0x%03X)", reg, val);
    return (int) val;
}

static void prv_nau_set_reg_defaults(struct nau88c10_ctx *ctx)
{
    ctx->reg[NAU88C10_REG_SOFTWARE_RESET]        = 0x000U;

    /* Power Management */
    ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_1]    = 0x000U;
    ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_2]    = 0x000U;
    ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_3]    = 0x000U;

    /* Audio Control */
    ctx->reg[NAU88C10_REG_AUDIO_INTERFACE]       = 0x050U;
    ctx->reg[NAU88C10_REG_COMPANDING]            = 0x000U;
    ctx->reg[NAU88C10_REG_CLOCK_CONTROL_1]       = 0x140U;
    ctx->reg[NAU88C10_REG_CLOCK_CONTROL_2]       = 0x000U;
    ctx->reg[NAU88C10_REG_DAC_CTRL]              = 0x000U;
    ctx->reg[NAU88C10_REG_DAC_VOLUME]            = 0x0FFU;
    ctx->reg[NAU88C10_REG_ADC_CTRL]              = 0x100U;
    ctx->reg[NAU88C10_REG_ADC_VOLUME]            = 0x0FFU;
    
    /* Equaliser */
    ctx->reg[NAU88C10_REG_EQ1_LOW_CUTOFF]        = 0x12CU;
    ctx->reg[NAU88C10_REG_EQ2_PEAK_1]            = 0x02CU;
    ctx->reg[NAU88C10_REG_EQ3_PEAK_2]            = 0x02CU;
    ctx->reg[NAU88C10_REG_EQ4_PEAK_3]            = 0x02CU;
    ctx->reg[NAU88C10_REG_EQ5_HIGH_CUTOFF]       = 0x02CU;

    /* Digital to Analog (DAC) Limiter */
    ctx->reg[NAU88C10_REG_DAC_LIMITER_1]         = 0x032U;
    ctx->reg[NAU88C10_REG_DAC_LIMITER_2]         = 0x000U;

    /* Notch Filter */
    ctx->reg[NAU88C10_REG_NOTCH_FILTER_0_HIGH]   = 0x000U;
    ctx->reg[NAU88C10_REG_NOTCH_FILTER_0_LOW]    = 0x000U;
    ctx->reg[NAU88C10_REG_NOTCH_FILTER_1_HIGH]   = 0x000U;
    ctx->reg[NAU88C10_REG_NOTCH_FILTER_1_LOW]    = 0x000U;

    /* ALC Control */
    ctx->reg[NAU88C10_REG_ALC_CTRL_1]            = 0x038U;
    ctx->reg[NAU88C10_REG_ALC_CTRL_2]            = 0x00BU;
    ctx->reg[NAU88C10_REG_ALC_CTRL_3]            = 0x032U;
    ctx->reg[NAU88C10_REG_NOISE_GATE]            = 0x000U;

    /* PLL Control */
    ctx->reg[NAU88C10_REG_PLL_N_CTRL]            = 0x008U;
    ctx->reg[NAU88C10_REG_PLL_K_1]               = 0x00CU;
    ctx->reg[NAU88C10_REG_PLL_K_2]               = 0x093U;
    ctx->reg[NAU88C10_REG_PLL_K_3]               = 0x0E9U;

    /* Input, Output & Mixer Control */
    ctx->reg[NAU88C10_REG_ATTENUATION_CTRL]      = 0x000U;
    ctx->reg[NAU88C10_REG_INPUT_CTRL]            = 0x003U;
    ctx->reg[NAU88C10_REG_PGA_GAIN]              = 0x010U;
    ctx->reg[NAU88C10_REG_ADC_BOOST]             = 0x100U;
    ctx->reg[NAU88C10_REG_OUTPUT_CTRL]           = 0x002U;
    ctx->reg[NAU88C10_REG_MIXER_CTRL]            = 0x001U;
    ctx->reg[NAU88C10_REG_SPKOUT_VOLUME]         = 0x039U;
    ctx->reg[NAU88C10_REG_MONO_MIXER_CONTROL]    = 0x001U;

    /* Low Power Control */
    ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_4]    = 0x000U;
    
    /* PCM Time Slot & ADCOUT Impedance Option Control */
    ctx->reg[NAU88C10_REG_TIME_SLOT]             = 0x000U;
    ctx->reg[NAU88C10_REG_ADCOUT_DRIVE]          = 0x020U;
    
    /* Register ID */
    ctx->reg[NAU88C10_REG_SILICON_REVISION]      = 0x0EEU;
    ctx->reg[NAU88C10_REG_2_WIRE_ID]             = 0x01AU;
    ctx->reg[NAU88C10_REG_ADDITIONAL_ID]         = 0x0CAU;

    /* Reserved */
    ctx->reg[NAU88C10_REG_RESERVED]              = 0x124U;

    /* Output Driver Control */
    ctx->reg[NAU88C10_REG_HIGH_VOLTAGE_CTRL]     = 0x001U;

    /* Automatic Level Control Enhancements */
    ctx->reg[NAU88C10_REG_ALC_ENHANCEMENTS_1]    = 0x000U;
    ctx->reg[NAU88C10_REG_ALC_ENHANCEMENTS_2]    = 0x039U;

    /* Misc */
    ctx->reg[NAU88C10_REG_ADDITIONAL_IF_CTRL]    = 0x000U;
    ctx->reg[NAU88C10_REG_POWER_TIE_OFF_CTRL]    = 0x000U;
    ctx->reg[NAU88C10_REG_AGC_P2P_DETECTOR]      = 0x000U;
    ctx->reg[NAU88C10_REG_AGC_PEAK_DETECTOR]     = 0x000U;
    ctx->reg[NAU88C10_REG_CONTROL_AND_STATUS]    = 0x000U;
    ctx->reg[NAU88C10_REG_OUTPUT_TIE_OFF_CTRL]   = 0x000U;

    ctx->vol = UINT8_MAX;

    LOG("register defaults set");
}

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

    /* I2S pins are configured by PIO code. */
}

static int prv_nau_set_refimp(struct nau88c10_ctx *ctx,
                              enum nau88c10_refimp refimp)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_1];
    reg &= ~NAU88C10_REFIMP_MASK;
    reg |= refimp << NAU88C10_REFIMP_POS;
    ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_1] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_POWER_MANAGEMENT_1);
    LOG("%sset REFIMP %s", (0 > rc) ? FAILED_TO : NOTHING, 
        NAU88C10_REFIMP_DISABLE == refimp ? DISABLED :
        NAU88C10_REFIMP_3K == refimp ? "3K" :
        NAU88C10_REFIMP_80K == refimp ? "80K" :
        NAU88C10_REFIMP_300K == refimp ? "300K" : "?");
    return rc;
}

static int prv_nau_set_iobufen(struct nau88c10_ctx *ctx,
                               enum nau88c10_iobufen iobufen)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_1];
    reg &= ~NAU88C10_IOBUFEN_MASK;
    reg |= iobufen << NAU88C10_IOBUFEN_POS;
    ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_1] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_POWER_MANAGEMENT_1);
    LOG("%sset IOBUFEN %s", (0 > rc) ? FAILED_TO : NOTHING, 
        iobufen ? ENABLED : DISABLED);
    return rc;
}

static int prv_nau_set_abiasen(struct nau88c10_ctx *ctx,
                               enum nau88c10_abiasen abiasen)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_1];
    reg &= ~NAU88C10_ABIASEN_MASK;
    reg |= abiasen << NAU88C10_ABIASEN_POS;
    ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_1] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_POWER_MANAGEMENT_1);
    LOG("%sset ABIASEN %s", (0 > rc) ? FAILED_TO : NOTHING, 
        abiasen ? ENABLED : DISABLED);
    return rc;
}

static int prv_nau_set_micbiasen(struct nau88c10_ctx *ctx,
                                 enum nau88c10_micbiasen micbiasen)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_1];
    reg &= ~NAU88C10_MICBIASEN_MASK;
    reg |= micbiasen << NAU88C10_MICBIASEN_POS;
    ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_1] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_POWER_MANAGEMENT_1);
    LOG("%sset MICBIASEN %s", (0 > rc) ? FAILED_TO : NOTHING, 
        micbiasen ? ENABLED : DISABLED);
    return rc;
}

static int prv_nau_set_pllen(struct nau88c10_ctx *ctx,
                             enum nau88c10_pllen pllen)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_1];
    reg &= ~NAU88C10_PLLEN_MASK;
    reg |= pllen << NAU88C10_PLLEN_POS;
    ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_1] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_POWER_MANAGEMENT_1);
    LOG("%sset PLLEN %s", (0 > rc) ? FAILED_TO : NOTHING, 
        pllen ? ENABLED : DISABLED);
    return rc;
}

static int prv_nau_set_dcbufen(struct nau88c10_ctx *ctx,
                               enum nau88c10_dcbufen dcbufen)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_1];
    reg &= ~NAU88C10_DCBUFEN_MASK;
    reg |= dcbufen << NAU88C10_DCBUFEN_POS;
    ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_1] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_POWER_MANAGEMENT_1);
    LOG("%sset DCBUFEN %s", (0 > rc) ? FAILED_TO : NOTHING, 
        dcbufen ? ENABLED : DISABLED);
    return rc;
}

static int prv_nau_set_adcen(struct nau88c10_ctx *ctx,
                             enum nau88c10_adcen adcen)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_2];
    reg &= ~NAU88C10_ADCEN_MASK;
    reg |= adcen << NAU88C10_ADCEN_POS;
    ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_2] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_POWER_MANAGEMENT_2);
    LOG("%sset ADCEN %s", (0 > rc) ? FAILED_TO : NOTHING, 
        adcen ? ENABLED : DISABLED);
    return rc;
}

static int prv_nau_set_pgaen(struct nau88c10_ctx *ctx,
                             enum nau88c10_pgaen pgaen)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_2];
    reg &= ~NAU88C10_PGAEN_MASK;
    reg |= pgaen << NAU88C10_PGAEN_POS;
    ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_2] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_POWER_MANAGEMENT_2);
    LOG("%sset PGAEN %s", (0 > rc) ? FAILED_TO : NOTHING, 
        pgaen ? ENABLED : DISABLED);
    return rc;
}

static int prv_nau_set_bsten(struct nau88c10_ctx *ctx,
                             enum nau88c10_bsten bsten)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_2];
    reg &= ~NAU88C10_BSTEN_MASK;
    reg |= bsten << NAU88C10_BSTEN_POS;
    ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_2] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_POWER_MANAGEMENT_2);
    LOG("%sset BSTEN %s", (0 > rc) ? FAILED_TO : NOTHING, 
        bsten ? ENABLED : DISABLED);
    return rc;
}

static int prv_nau_set_dacen(struct nau88c10_ctx *ctx,
                             enum nau88c10_dacen dacen)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_3];
    reg &= ~NAU88C10_DACEN_MASK;
    reg |= dacen << NAU88C10_DACEN_POS;
    ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_3] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_POWER_MANAGEMENT_3);
    LOG("%sset DACEN %s", (0 > rc) ? FAILED_TO : NOTHING, 
        dacen ? ENABLED : DISABLED);
    return rc;
}

static int prv_nau_set_spkmxen(struct nau88c10_ctx *ctx,
                               enum nau88c10_spkmxen spkmxen)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_3];
    reg &= ~NAU88C10_SPKMXEN_MASK;
    reg |= spkmxen << NAU88C10_SPKMXEN_POS;
    ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_3] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_POWER_MANAGEMENT_3);
    LOG("%sset SPKMXEN %s", (0 > rc) ? FAILED_TO : NOTHING, 
        spkmxen ? ENABLED : DISABLED);
    return rc;
}

static int prv_nau_set_moutmxen(struct nau88c10_ctx *ctx,
                                enum nau88c10_moutmxen moutmxen)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_3];
    reg &= ~NAU88C10_MOUTMXEN_MASK;
    reg |= moutmxen << NAU88C10_MOUTMXEN_POS;
    ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_3] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_POWER_MANAGEMENT_3);
    LOG("%sset MOUTMXEN %s", (0 > rc) ? FAILED_TO : NOTHING, 
        moutmxen ? ENABLED : DISABLED);
    return rc;
}

static int prv_nau_set_pspken(struct nau88c10_ctx *ctx,
                              enum nau88c10_pspken pspken)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_3];
    reg &= ~NAU88C10_PSPKEN_MASK;
    reg |= pspken << NAU88C10_PSPKEN_POS;
    ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_3] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_POWER_MANAGEMENT_3);
    LOG("%sset PSPKEN %s", (0 > rc) ? FAILED_TO : NOTHING, 
        pspken ? ENABLED : DISABLED);
    return rc;
}

static int prv_nau_set_nspken(struct nau88c10_ctx *ctx,
                              enum nau88c10_nspken nspken)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_3];
    reg &= ~NAU88C10_NSPKEN_MASK;
    reg |= nspken << NAU88C10_NSPKEN_POS;
    ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_3] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_POWER_MANAGEMENT_3);
    LOG("%sset NSPKEN %s", (0 > rc) ? FAILED_TO : NOTHING, 
        nspken ? ENABLED : DISABLED);
    return rc;
}

static int prv_nau_set_mouten(struct nau88c10_ctx *ctx,
                              enum nau88c10_mouten mouten)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_3];
    reg &= ~NAU88C10_MOUTEN_MASK;
    reg |= mouten << NAU88C10_MOUTEN_POS;
    ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_3] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_POWER_MANAGEMENT_3);
    LOG("%sset MOUTEN %s", (0 > rc) ? FAILED_TO : NOTHING, 
        mouten ? ENABLED : DISABLED);
    return rc;
}

static int prv_nau_set_adcphs(struct nau88c10_ctx *ctx,
                              enum nau88c10_adcphs adcphs)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_AUDIO_INTERFACE];
    reg &= ~NAU88C10_ADCPHS_MASK;
    reg |= adcphs << NAU88C10_ADCPHS_POS;
    ctx->reg[NAU88C10_REG_AUDIO_INTERFACE] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_AUDIO_INTERFACE);
    LOG("%sset ADCPHS %s", (0 > rc) ? FAILED_TO : NOTHING,
        NAU88C10_ADCPHS_LEFT == adcphs ? LEFT : RIGHT);
    return rc;
}

static int prv_nau_set_dacphs(struct nau88c10_ctx *ctx,
                              enum nau88c10_dacphs dacphs)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_AUDIO_INTERFACE];
    reg &= ~NAU88C10_DACPHS_MASK;
    reg |= dacphs << NAU88C10_DACPHS_POS;
    ctx->reg[NAU88C10_REG_AUDIO_INTERFACE] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_AUDIO_INTERFACE);
    LOG("%sset DACPHS %s", (0 > rc) ? FAILED_TO : NOTHING,
        NAU88C10_DACPHS_LEFT == dacphs ? LEFT : RIGHT);
    return rc;
}

static int prv_nau_set_aifmt(struct nau88c10_ctx *ctx,
                             enum nau88c10_aifmt aifmt)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_AUDIO_INTERFACE];
    reg &= ~NAU88C10_AIFMT_MASK;
    reg |= aifmt << NAU88C10_AIFMT_POS;
    ctx->reg[NAU88C10_REG_AUDIO_INTERFACE] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_AUDIO_INTERFACE);
    LOG("%sset AIFMT %s", (0 > rc) ? FAILED_TO : NOTHING, 
        NAU88C10_AIFMT_RIGHT_JUSTIFIED == aifmt ? "right justified" :
        NAU88C10_AIFMT_LEFT_JUSTIFIED == aifmt ? "left justified" :
        NAU88C10_AIFMT_I2S == aifmt ? "I2S" :
        NAU88C10_AIFMT_PCM_A == aifmt ? "PCM A" : "?");
    return rc;
}

static int prv_nau_set_wlen(struct nau88c10_ctx *ctx,
                            enum nau88c10_wlen wlen)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_AUDIO_INTERFACE];
    reg &= ~NAU88C10_WLEN_MASK;
    reg |= wlen << NAU88C10_WLEN_POS;
    ctx->reg[NAU88C10_REG_AUDIO_INTERFACE] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_AUDIO_INTERFACE);
    LOG("%sset WLEN %d", (0 > rc) ? FAILED_TO : NOTHING, 
        NAU88C10_WLEN_16 == wlen ? 16 : 
        NAU88C10_WLEN_20 == wlen ? 20 : 
        NAU88C10_WLEN_24 == wlen ? 24 : 
        NAU88C10_WLEN_32 == wlen ? 32 : 
        -1);
    return rc;
}

static int prv_nau_set_fsp(struct nau88c10_ctx *ctx,
                           enum nau88c10_fsp fsp)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_AUDIO_INTERFACE];
    reg &= ~NAU88C10_FSP_MASK;
    reg |= fsp << NAU88C10_FSP_POS;
    ctx->reg[NAU88C10_REG_AUDIO_INTERFACE] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_AUDIO_INTERFACE);
    LOG("%sset FSP %s", (0 > rc) ? FAILED_TO : NOTHING,
        NAU88C10_FSP_NORMAL == fsp ? NORMAL : INVERTED);
    return rc;
}

static int prv_nau_set_bclkp(struct nau88c10_ctx *ctx,
                             enum nau88c10_bclkp bclkp)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_AUDIO_INTERFACE];
    reg &= ~NAU88C10_BCLKP_MASK;
    reg |= bclkp << NAU88C10_BCLKP_POS;
    ctx->reg[NAU88C10_REG_AUDIO_INTERFACE] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_AUDIO_INTERFACE);
    LOG("%sset BCLKP %s", (0 > rc) ? FAILED_TO : NOTHING,
        NAU88C10_BCLKP_NORMAL == bclkp ? NORMAL : INVERTED);
    return rc;
}

static int prv_nau_set_clkioen(struct nau88c10_ctx *ctx,
                               enum nau88c10_clkioen clkioen)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_CLOCK_CONTROL_1];
    reg &= ~NAU88C10_CLKIOEN_MASK;
    reg |= clkioen << NAU88C10_CLKIOEN_POS;
    ctx->reg[NAU88C10_REG_CLOCK_CONTROL_1] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_CLOCK_CONTROL_1);
    LOG("%sset CLKIOEN %s", (0 > rc) ? FAILED_TO : NOTHING,
        NAU88C10_CLKIOEN_MASTER == clkioen ? "master" : "slave");
    return rc;
}

static int prv_nau_set_bclksel(struct nau88c10_ctx *ctx,
                               enum nau88c10_bclksel bclksel)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_CLOCK_CONTROL_1];
    reg &= ~NAU88C10_BCLKSEL_MASK;
    reg |= bclksel << NAU88C10_BCLKSEL_POS;
    ctx->reg[NAU88C10_REG_CLOCK_CONTROL_1] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_CLOCK_CONTROL_1);
    LOG("%sset BCLKSEL %d", (0 > rc) ? FAILED_TO : NOTHING,
        1 << bclksel);
    return rc;
}

static int prv_nau_set_mclksel(struct nau88c10_ctx *ctx,
                               enum nau88c10_mclksel mclksel)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_CLOCK_CONTROL_1];
    reg &= ~NAU88C10_MCLKSEL_MASK;
    reg |= mclksel << NAU88C10_MCLKSEL_POS;
    ctx->reg[NAU88C10_REG_CLOCK_CONTROL_1] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_CLOCK_CONTROL_1);
    LOG("%sset MCLKSEL %f", (0 > rc) ? FAILED_TO : NOTHING,
        NAU88C10_MCLKSEL_DIV_1 == mclksel ? 1.f :
        NAU88C10_MCLKSEL_DIV_1_5 == mclksel ? 1.5f :
        NAU88C10_MCLKSEL_DIV_2 == mclksel ? 2.f :
        NAU88C10_MCLKSEL_DIV_3 == mclksel ? 3.f :
        NAU88C10_MCLKSEL_DIV_4 == mclksel ? 4.f :
        NAU88C10_MCLKSEL_DIV_6 == mclksel ? 6.f :
        NAU88C10_MCLKSEL_DIV_8 == mclksel ? 8.f :
        NAU88C10_MCLKSEL_DIV_12 == mclksel ? 12.f : -1);
    return rc;
}

static int prv_nau_set_clkm(struct nau88c10_ctx *ctx,
                            enum nau88c10_clkm clkm)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_CLOCK_CONTROL_1];
    reg &= ~NAU88C10_CLKM_MASK;
    reg |= clkm << NAU88C10_CLKM_POS;
    ctx->reg[NAU88C10_REG_CLOCK_CONTROL_1] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_CLOCK_CONTROL_1);
    LOG("%sset CLKM %s", (0 > rc) ? FAILED_TO : NOTHING,
        NAU88C10_CLKM_PLL_OUTPUT == clkm ? "PLL Output" : "PLL Bypassed");
    return rc;
}

static int prv_nau_set_sclken(struct nau88c10_ctx *ctx,
                              enum nau88c10_sclken sclken)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_CLOCK_CONTROL_2];
    reg &= ~NAU88C10_SCLKEN_MASK;
    reg |= sclken << NAU88C10_SCLKEN_POS;
    ctx->reg[NAU88C10_REG_CLOCK_CONTROL_2] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_CLOCK_CONTROL_2);
    LOG("%sset SCLKEN %s", (0 > rc) ? FAILED_TO : NOTHING,
        NAU88C10_SCLKEN_PLL_OUTPUT == sclken ? "PLL Output" : "MCLK");
    return rc;
}

static int prv_nau_set_smplr(struct nau88c10_ctx *ctx,
                             enum nau88c10_smplr smplr)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_CLOCK_CONTROL_2];
    reg &= ~NAU88C10_SMPLR_MASK;
    reg |= smplr << NAU88C10_SMPLR_POS;
    ctx->reg[NAU88C10_REG_CLOCK_CONTROL_2] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_CLOCK_CONTROL_2);
    LOG("%sset SMPLR %s", (0 > rc) ? FAILED_TO : NOTHING,
        NAU88C10_SMPLR_48_KHZ == smplr ? "48 kHz" : 
        NAU88C10_SMPLR_32_KHZ == smplr ? "32 kHz" : 
        NAU88C10_SMPLR_24_KHZ == smplr ? "24 kHz" : 
        NAU88C10_SMPLR_16_KHZ == smplr ? "16 kHz" : 
        NAU88C10_SMPLR_12_KHZ == smplr ? "12 kHz" : 
        NAU88C10_SMPLR_8_KHZ == smplr ? "8 kHz" : 
        "? kHz");
    return rc;
}

static int prv_nau_set_dacpl(struct nau88c10_ctx *ctx,
                             enum nau88c10_dacpl dacpl)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_DAC_CTRL];
    reg &= ~NAU88C10_DACPL_MASK;
    reg |= dacpl << NAU88C10_DACPL_POS;
    ctx->reg[NAU88C10_REG_DAC_CTRL] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_DAC_CTRL);
    LOG("%sset DACPL %s", (0 > rc) ? FAILED_TO : NOTHING,
        NAU88C10_DACPL_NORMAL == dacpl ? NORMAL : INVERTED);
    return rc;
}

static int prv_nau_set_automt(struct nau88c10_ctx *ctx,
                              enum nau88c10_automt automt)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_DAC_CTRL];
    reg &= ~NAU88C10_AUTOMT_MASK;
    reg |= automt << NAU88C10_AUTOMT_POS;
    ctx->reg[NAU88C10_REG_DAC_CTRL] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_DAC_CTRL);
    LOG("%sset AUTOMT %s", (0 > rc) ? FAILED_TO : NOTHING, 
        automt ? ENABLED : DISABLED);
    return rc;
}

static int prv_nau_set_dacos(struct nau88c10_ctx *ctx,
                             enum nau88c10_dacos dacos)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_DAC_CTRL];
    reg &= ~NAU88C10_DACOS_MASK;
    reg |= dacos << NAU88C10_DACOS_POS;
    ctx->reg[NAU88C10_REG_DAC_CTRL] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_DAC_CTRL);
    LOG("%sset DACOS %dX", (0 > rc) ? FAILED_TO : NOTHING,
        NAU88C10_DACOS_64X == dacos ? 64 : 128);
    return rc;
}

static int prv_nau_set_deemp(struct nau88c10_ctx *ctx,
                             enum nau88c10_deemp deemp)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_DAC_CTRL];
    reg &= ~NAU88C10_DEEMP_MASK;
    reg |= deemp << NAU88C10_DEEMP_POS;
    ctx->reg[NAU88C10_REG_DAC_CTRL] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_DAC_CTRL);
    LOG("%sset DEEMP %s", (0 > rc) ? FAILED_TO : NOTHING,
        NAU88C10_DEEMP_48_KHZ == deemp ? "48 kHz" : 
        NAU88C10_DEEMP_44_1_KHZ == deemp ? "44.1 kHz" : 
        NAU88C10_DEEMP_32_KHZ == deemp ? "32 kHz" : 
        NAU88C10_DEEMP_NONE == deemp ? "None" : 
        "?");
    return rc;
}

static int prv_nau_set_dacmt(struct nau88c10_ctx *ctx,
                             enum nau88c10_dacmt dacmt)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_DAC_CTRL];
    reg &= ~NAU88C10_DACMT_MASK;
    reg |= dacmt << NAU88C10_DACMT_POS;
    ctx->reg[NAU88C10_REG_DAC_CTRL] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_DAC_CTRL);
    LOG("%sset DAC %s", (0 > rc) ? FAILED_TO : NOTHING,
        dacmt ? "muted" : "unmuted");
    return rc;
}

static int prv_nau_set_dacgain(struct nau88c10_ctx *ctx,
                               enum nau88c10_dacgain dacgain)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_DAC_VOLUME];
    reg &= ~NAU88C10_DACGAIN_MASK;
    reg |= dacgain << NAU88C10_DACGAIN_POS;
    ctx->reg[NAU88C10_REG_DAC_VOLUME] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_DAC_VOLUME);
    LOG("%sset DACGAIN -%d.%d dB", (0 > rc) ? FAILED_TO : NOTHING,
        (NAU88C10_DACGAIN_0_DBFS - dacgain) / 2,
        ((NAU88C10_DACGAIN_0_DBFS - dacgain) % 2) * 5);
    return rc;
}

static int prv_nau_set_adcpl(struct nau88c10_ctx *ctx,
                             enum nau88c10_adcpl adcpl)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_ADC_CTRL];
    reg &= ~NAU88C10_ADCPL_MASK;
    reg |= adcpl << NAU88C10_ADCPL_POS;
    ctx->reg[NAU88C10_REG_ADC_CTRL] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_ADC_CTRL);
    LOG("%sset ADCPL %s", (0 > rc) ? FAILED_TO : NOTHING, 
        NAU88C10_ADCPL_NORMAL == adcpl ? NORMAL : INVERTED);
    return rc;
}

static int prv_nau_set_adcos(struct nau88c10_ctx *ctx,
                             enum nau88c10_adcos adcos)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_ADC_CTRL];
    reg &= ~NAU88C10_ADCOS_MASK;
    reg |= adcos << NAU88C10_ADCOS_POS;
    ctx->reg[NAU88C10_REG_ADC_CTRL] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_ADC_CTRL);
    LOG("%sset ADCOS %dX", (0 > rc) ? FAILED_TO : NOTHING,
        NAU88C10_ADCOS_64X == adcos ? 64 : 128);
    return rc;
}

static int prv_nau_set_hpf(struct nau88c10_ctx *ctx,
                             enum nau88c10_hpf hpf)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_ADC_CTRL];
    reg &= ~NAU88C10_HPF_MASK;
    reg |= hpf << NAU88C10_HPF_POS;
    ctx->reg[NAU88C10_REG_ADC_CTRL] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_ADC_CTRL);
    LOG("%sset HPF %d (@fs = 48 kHz)", (0 > rc) ? FAILED_TO : NOTHING,
        NAU88C10_HPF_FS_48K_112_HZ == hpf ? "112 Hz" : 
        NAU88C10_HPF_FS_48K_153_HZ == hpf ? "153 Hz" : 
        NAU88C10_HPF_FS_48K_156_HZ == hpf ? "156 Hz" : 
        NAU88C10_HPF_FS_48K_245_HZ == hpf ? "245 Hz" : 
        NAU88C10_HPF_FS_48K_306_HZ == hpf ? "306 Hz" : 
        NAU88C10_HPF_FS_48K_392_HZ == hpf ? "392 Hz" : 
        NAU88C10_HPF_FS_48K_490_HZ == hpf ? "490 Hz" : 
        NAU88C10_HPF_FS_48K_612_HZ == hpf ? "612 Hz" : 
        "?");
    return rc;
}

static int prv_nau_set_hpfam(struct nau88c10_ctx *ctx,
                             enum nau88c10_hpfam hpfam)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_ADC_CTRL];
    reg &= ~NAU88C10_HPFAM_MASK;
    reg |= hpfam << NAU88C10_HPFAM_POS;
    ctx->reg[NAU88C10_REG_ADC_CTRL] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_ADC_CTRL);
    LOG("%sset HPFAM %s", (0 > rc) ? FAILED_TO : NOTHING,
        NAU88C10_HPFAM_AUDIO == hpfam ? "AUDIO" : "APPLICATION");
    return rc;
}

static int prv_nau_set_hpfen(struct nau88c10_ctx *ctx,
                             enum nau88c10_hpfen hpfen)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_ADC_CTRL];
    reg &= ~NAU88C10_HPFEN_MASK;
    reg |= hpfen << NAU88C10_HPFEN_POS;
    ctx->reg[NAU88C10_REG_ADC_CTRL] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_ADC_CTRL);
    LOG("%sset HPFEN %s", (0 > rc) ? FAILED_TO : NOTHING, 
        hpfen ? ENABLED : DISABLED);
    return rc;
}

static int prv_nau_set_daclimatk(struct nau88c10_ctx *ctx,
                                 enum nau88c10_daclimatk daclimatk)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_DAC_LIMITER_1];
    reg &= ~NAU88C10_DACLIMATK_MASK;
    reg |= daclimatk << NAU88C10_DACLIMATK_POS;
    ctx->reg[NAU88C10_REG_DAC_LIMITER_1] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_DAC_LIMITER_1);
    LOG("%sset DACLIMATK %d", (0 > rc) ? FAILED_TO : NOTHING, daclimatk);
    return rc;
}

static int prv_nau_set_daclimdcy(struct nau88c10_ctx *ctx,
                                 enum nau88c10_daclimdcy daclimdcy)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_DAC_LIMITER_1];
    reg &= ~NAU88C10_DACLIMDCY_MASK;
    reg |= daclimdcy << NAU88C10_DACLIMDCY_POS;
    ctx->reg[NAU88C10_REG_DAC_LIMITER_1] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_DAC_LIMITER_1);
    LOG("%sset DACLIMDCY %d", (0 > rc) ? FAILED_TO : NOTHING, daclimdcy);
    return rc;
}

static int prv_nau_set_daclimen(struct nau88c10_ctx *ctx,
                                enum nau88c10_daclimen daclimen)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_DAC_LIMITER_1];
    reg &= ~NAU88C10_DACLIMEN_MASK;
    reg |= daclimen << NAU88C10_DACLIMEN_POS;
    ctx->reg[NAU88C10_REG_DAC_LIMITER_1] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_DAC_LIMITER_1);
    LOG("%sset DACLIMEN %s", (0 > rc) ? FAILED_TO : NOTHING, 
        daclimen ? ENABLED : DISABLED);
    return rc;
}

static int prv_nau_set_daclimbst(struct nau88c10_ctx *ctx,
                                 enum nau88c10_daclimbst daclimbst)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_DAC_LIMITER_2];
    reg &= ~NAU88C10_DACLIMBST_MASK;
    reg |= daclimbst << NAU88C10_DACLIMBST_POS;
    ctx->reg[NAU88C10_REG_DAC_LIMITER_2] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_DAC_LIMITER_2);
    LOG("%sset DACLIMBST +%d dB", (0 > rc) ? FAILED_TO : NOTHING, daclimbst);
    return rc;
}

static int prv_nau_set_daclimthl(struct nau88c10_ctx *ctx,
                                 enum nau88c10_daclimthl daclimthl)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_DAC_LIMITER_2];
    reg &= ~NAU88C10_DACLIMTHL_MASK;
    reg |= daclimthl << NAU88C10_DACLIMTHL_POS;
    ctx->reg[NAU88C10_REG_DAC_LIMITER_2] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_DAC_LIMITER_2);
    LOG("%sset DACLIMTHL -%d dB", (0 > rc) ? FAILED_TO : NOTHING, daclimthl);
    return rc;
}

static int prv_nau_set_pgabst(struct nau88c10_ctx *ctx,
                              enum nau88c10_pgabst pgabst)
{
    uint16_t reg = ctx->reg[NAU88C10_REG_ADC_BOOST];
    reg &= ~NAU88C10_PGABST_MASK;
    reg |= pgabst << NAU88C10_PGABST_POS;
    ctx->reg[NAU88C10_REG_ADC_BOOST] = reg;
    int rc = prv_nau_send_reg(ctx, NAU88C10_REG_ADC_BOOST);
    LOG("%sset PGABST +%d", (0 > rc) ? FAILED_TO : NOTHING, 
        pgabst ? 20 : 0);
    return rc;
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

void nau88c10_reset(struct nau88c10_ctx *ctx)
{
    int rc = prv_nau_write_reg(ctx, NAU88C10_REG_SOFTWARE_RESET, 0x1ff);
    if (rc < 0) {
        LOG("failed to reset");
        return;
    } else {
        LOG("reset");
    }

    prv_nau_set_reg_defaults(ctx);

    rc = prv_nau_read_reg(ctx, NAU88C10_REG_SILICON_REVISION);
    if (rc < 0) {
        LOG("failed to read silicon revision");
        return;
    } 
    LOG("silicon revision: 0x%03X", (uint16_t) rc);
}

static int wrap_busy_wait_ms(uint32_t ms)
{
    busy_wait_ms(ms);
    return 0;
}

void nau88c10_up(struct nau88c10_ctx *ctx)
{
    bool ok = true;
    if (/* Always start with the output muted. */
        (0 > prv_nau_set_dacmt(ctx, NAU88C10_DACMT_ENABLE))
        /* Set REFIMP lower for fast start up fill. */
        || (0 > prv_nau_set_refimp(ctx, NAU88C10_REFIMP_3K))
        /* Wait for 4.7 uF cap to fill. */
        || (0 > wrap_busy_wait_ms(30))
        /* Set REFIMP higher for better PSRR. */
        || (0 > prv_nau_set_refimp(ctx, NAU88C10_REFIMP_80K))
        || (0 > prv_nau_set_abiasen(ctx, NAU88C10_ABIASEN_ENABLE))
        || (0 > prv_nau_set_iobufen(ctx, NAU88C10_IOBUFEN_ENABLE))
        /* Configure clocks/bus. */
        || (0 > prv_nau_set_wlen(ctx, NAU88C10_WLEN_32)) // FIXME: verify this should be 32-bits. -PMW
        || (0 > prv_nau_set_clkioen(ctx, NAU88C10_CLKIOEN_SLAVE))
        || (0 > prv_nau_set_mclksel(ctx, NAU88C10_MCLKSEL_DIV_1))
        || (0 > prv_nau_set_clkm(ctx, NAU88C10_CLKM_PLL_BYPASSED))
        || (0 > prv_nau_set_sclken(ctx, NAU88C10_SCLKEN_PLL_OUTPUT))
        || (0 > prv_nau_set_smplr(ctx, NAU88C10_SMPLR_48_KHZ))
        /* Enable ADC/DAC. */
        || (0 > prv_nau_set_adcen(ctx, NAU88C10_ADCEN_DISABLE))
        || (0 > prv_nau_set_dacen(ctx, NAU88C10_DACEN_ENABLE))
        /* Enable analog input circuitry. */
        || (0 > prv_nau_set_pgaen(ctx, NAU88C10_PGAEN_DISABLE))
        || (0 > prv_nau_set_bsten(ctx, NAU88C10_BSTEN_STAGE_DISABLE))
        || (0 > prv_nau_set_micbiasen(ctx, NAU88C10_MICBIASEN_DISABLE))
        /* Enable analog output circuitry. */
        || (0 > prv_nau_set_spkmxen(ctx, NAU88C10_SPKMXEN_ENABLE))
        || (0 > prv_nau_set_moutmxen(ctx, NAU88C10_MOUTMXEN_ENABLE))
        || (0 > prv_nau_set_mouten(ctx, NAU88C10_MOUTEN_ENABLE))
        || (0 > prv_nau_set_nspken(ctx, NAU88C10_NSPKEN_ENABLE))
        || (0 > prv_nau_set_pspken(ctx, NAU88C10_PSPKEN_ENABLE))
        /* Configure the ADC. */
        || (0 > prv_nau_set_adcos(ctx, NAU88C10_ADCOS_64X))
        || (0 > prv_nau_set_hpf(ctx, NAU88C10_HPF_FS_48K_245_HZ))
        || (0 > prv_nau_set_hpfam(ctx, NAU88C10_HPFAM_APPLICATION))
        || (0 > prv_nau_set_hpfen(ctx, NAU88C10_HPFEN_ENABLED))
        /* Configure the input path. */
        || (0 > prv_nau_set_pgabst(ctx, NAU88C10_PGABST_0_DB))
        /* Configure the DAC. */
        || (0 > prv_nau_set_automt(ctx, NAU88C10_AUTOMT_ENABLE))
        || (0 > prv_nau_set_dacos(ctx, NAU88C10_DACOS_64X))
        || (0 > prv_nau_set_deemp(ctx, NAU88C10_DEEMP_48_KHZ))
        /* These limiter settings align with the -12 dBFS nominal mixer level. */
        || (0 > prv_nau_set_daclimatk(ctx, NAU88C10_DACLIMATK_68_US))
        || (0 > prv_nau_set_daclimdcy(ctx, NAU88C10_DACLIMDCY_4_4_MS))
        || (0 > prv_nau_set_daclimbst(ctx, NAU88C10_DACLIMBST_PLUS_12_DB))
        || (0 > prv_nau_set_daclimthl(ctx, NAU88C10_DACLIMTHL_MINUS_6_DB))
        || (0 > prv_nau_set_daclimen(ctx, NAU88C10_DACLIMEN_ENABLED))
        // TODO: configure eq -PMW
        // TODO: configure mixer (if needed?) -PMW
    ) {
        ok = false;
    }
    struct i2s_config i2s_cfg = {
        .fs = 48000,
        .sck_mult = 256,
        .bit_depth = 32,
        .sck_pin = ctx->cfg->i2s_mclk_pin,
        .dout_pin = ctx->cfg->i2s_dacin_pin,
        .din_pin = ctx->cfg->i2s_adcout_pin,
        .clock_pin_base = ctx->cfg->i2s_bclk_pin,
        .sck_enable = true,
    };
    i2s_program_start_synched(ctx->cfg->i2s_pio, &i2s_cfg, 
                              ctx->cfg->i2s_dma_handler, &ctx->pio_i2s);
    (void) prv_nau_set_dacmt(ctx, NAU88C10_DACMT_DISABLE);
    LOG("%s%s up", !ok ? FAILED_TO : NOTHING, !ok ? "bring" : "brought");
}

void nau88c10_set_output_muted(struct nau88c10_ctx *ctx, bool muted)
{
    enum nau88c10_dacmt dacmt;
    dacmt = muted ? NAU88C10_DACMT_ENABLE : NAU88C10_DACMT_DISABLE;
    (void) prv_nau_set_dacmt(ctx, dacmt);
}

uint8_t nau88c10_get_volume(struct nau88c10_ctx *ctx)
{
    return ctx->vol;
}

void nau88c10_set_volume(struct nau88c10_ctx *ctx, uint8_t volume)
{
    enum nau88c10_dacgain dacgain;
    if (0 == volume) {
        dacgain = NAU88C10_DACGAIN_DIGITAL_MUTE;
    } else if (UINT8_MAX == volume) {
        dacgain = NAU88C10_DACGAIN_0_DBFS;
    } else if (NAU88C10_VOL_KNEE_VAL < volume) {

        dacgain = 
            ((volume - NAU88C10_VOL_KNEE_VAL) 
             * (NAU88C10_DACGAIN_0_DBFS - NAU88C10_VOL_KNEE_GAIN)
             / (UINT8_MAX - NAU88C10_VOL_KNEE_VAL)) 
            + NAU88C10_VOL_KNEE_GAIN;
    } else {
        dacgain = volume * NAU88C10_VOL_KNEE_GAIN / NAU88C10_VOL_KNEE_VAL;
    }

    (void) prv_nau_set_dacgain(ctx, dacgain);
    ctx->vol = volume;
}

bool nau88c10_get_speaker_enabled(struct nau88c10_ctx *ctx)
{
    return NAU88C10_SPKMXEN_ENABLE 
        == ((ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_3] & NAU88C10_SPKMXEN_MASK)
            >> NAU88C10_SPKMXEN_POS);
}

void nau88c10_set_speaker_enabled(struct nau88c10_ctx *ctx, bool en)
{
    if (en && !nau88c10_get_speaker_enabled(ctx)) {
        (void) prv_nau_set_spkmxen(ctx, NAU88C10_SPKMXEN_ENABLE);
        (void) prv_nau_set_nspken(ctx, NAU88C10_NSPKEN_ENABLE);
        (void) prv_nau_set_pspken(ctx, NAU88C10_PSPKEN_ENABLE);
    } else if (!en && nau88c10_get_speaker_enabled(ctx)) {
        (void) prv_nau_set_pspken(ctx, NAU88C10_PSPKEN_DISABLE);
        (void) prv_nau_set_nspken(ctx, NAU88C10_NSPKEN_DISABLE);
        (void) prv_nau_set_spkmxen(ctx, NAU88C10_SPKMXEN_DISABLE);
    }
}

bool nau88c10_get_headphone_enabled(struct nau88c10_ctx *ctx)
{
    return NAU88C10_MOUTMXEN_ENABLE 
        == ((ctx->reg[NAU88C10_REG_POWER_MANAGEMENT_3] & NAU88C10_MOUTMXEN_MASK)
            >> NAU88C10_MOUTMXEN_POS);
}

void nau88c10_set_headphone_enabled(struct nau88c10_ctx *ctx, bool en)
{
    if (en && !nau88c10_get_headphone_enabled(ctx)) {
        (void) prv_nau_set_moutmxen(ctx, NAU88C10_MOUTMXEN_ENABLE);
        (void) prv_nau_set_mouten(ctx, NAU88C10_MOUTEN_ENABLE);
    } else if (!en && nau88c10_get_headphone_enabled(ctx)) {
        (void) prv_nau_set_mouten(ctx, NAU88C10_MOUTEN_DISABLE);
        (void) prv_nau_set_moutmxen(ctx, NAU88C10_MOUTMXEN_DISABLE);
    }
}


