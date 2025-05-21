/*
 *  @author Peter Maxwell Warasila
 *  @date   April 10, 2024
 *
 *  @brief  RVASec 2024 Badge Audio Common Functions
 *
 *------------------------------------------------------------------------------
 *
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fxp_sqrt.h>

#include "audio.h"
#include "utils.h"

/* TODO: add logging system? -PMW */
#ifndef LOG
#define LOG(...) printf("\r\n[audio] " __VA_ARGS__)
#endif /* LOG */

/*! @addtogroup BADGE_AUDIO Audio Driver
 *  @{
 */

/*- Private Constants --------------------------------------------------------*/
#define DB_RATIO_TABLE_ZERO_INDEX (86) /* Index of the entry for 0 dB. */
static const struct {int8_t dB; uint32_t ratio;} DB_RATIO_TABLE[] = 
{
    {INT8_MIN,     0},
    {-96,          1},
    {-90,          2},
    {-86,          3},
    {-84,          4},
    {-82,          5},
    {-80,          6},
    {-79,          7},
    {-78,          8},
    {-77,          9},
    {-76,         10},
    {-75,         11},
    {-74,         13},
    {-73,         14},
    {-72,         16},
    {-71,         18},
    {-70,         20},
    {-69,         23},
    {-68,         26},
    {-67,         29},
    {-66,         32},
    {-65,         36},
    {-64,         41},
    {-63,         46},
    {-62,         52},
    {-61,         58},
    {-60,         65},
    {-59,         73},
    {-58,         82},
    {-57,         92},
    {-56,        103},
    {-55,        116},
    {-54,        130},
    {-53,        146},
    {-52,        164},
    {-51,        184},
    {-50,        207},
    {-49,        232},
    {-48,        260},
    {-47,        292},
    {-46,        328},
    {-45,        368},
    {-44,        413},
    {-43,        463},
    {-42,        520},
    {-41,        584},
    {-40,        655},
    {-39,        735},
    {-38,        825},
    {-37,        925},
    {-36,       1038},
    {-35,       1165},
    {-34,       1307},
    {-33,       1467},
    {-32,       1646},
    {-31,       1847},
    {-30,       2072},
    {-29,       2325},
    {-28,       2609},
    {-27,       2927},
    {-26,       3284},
    {-25,       3685},
    {-24,       4135},
    {-23,       4639},
    {-22,       5205},
    {-21,       5840},
    {-20,       6553},
    {-19,       7353},
    {-18,       8250},
    {-17,       9257},
    {-16,      10386},
    {-15,      11654},
    {-14,      13076},
    {-13,      14671},
    {-12,      16461},
    {-11,      18470},
    {-10,      20724},
    { -9,      23253},
    { -8,      26090},
    { -7,      29273},
    { -6,      32845},
    { -5,      36853},
    { -4,      41350},
    { -3,      46395},
    { -2,      52057},
    { -1,      58409},
    {  0,      65536},
    {  1,      73532},
    {  2,      82504},
    {  3,      92572},
    {  4,     103867},
    {  5,     116541},
    {  6,     130761},
    {  7,     146716},
    {  8,     164618},
    {  9,     184705},
    { 10,     207243},
    { 11,     232530},
    { 12,     260903},
    { 13,     292738},
    { 14,     328458},
    { 15,     368536},
    { 16,     413504},
    { 17,     463959},
    { 18,     520570},
    { 19,     584090},
    { 20,     655360},
    { 21,     735326},
    { 22,     825049},
    { 23,     925720},
    { 24,    1038675},
    { 25,    1165413},
    { 26,    1307615},
    { 27,    1467168},
    { 28,    1646189},
    { 29,    1847055},
    { 30,    2072430},
    { 31,    2325305},
    { 32,    2609035},
    { 33,    2927385},
    { 34,    3284580},
    { 35,    3685360},
    { 36,    4135042},
    { 37,    4639593},
    { 38,    5205709},
    { 39,    5840902},
    { 40,    6553600},
    { 41,    7353260},
    { 42,    8250493},
    { 43,    9257206},
    { 44,   10386756},
    { 45,   11654131},
    { 46,   13076151},
    { 47,   14671682},
    { 48,   16461898},
    { 49,   18470554},
    { 50,   20724302},
    { 51,   23253050},
    { 52,   26090351},
    { 53,   29273855},
    { 54,   32845806},
    { 55,   36853601},
    { 56,   41350420},
    { 57,   46395934},
    { 58,   52057095},
    { 59,   58409021},
    { 60,   65536000},
    { 61,   73532601},
    { 62,   82504935},
    { 63,   92572060},
    { 64,  103867560},
    { 65,  116541319},
    { 66,  130761511},
    { 67,  146716828},
    { 68,  164618989},
    { 69,  184705543},
    { 70,  207243028},
    { 71,  232530502},
    { 72,  260903515},
    { 73,  292738558},
    { 74,  328458065},
    { 75,  368536010},
    { 76,  413504205},
    { 77,  463959349},
    { 78,  520570951},
    { 79,  584090214},
    { 80,  655360000},
    { 81,  735326014},
    { 82,  825049357},
    { 83,  925720605},
    { 84, 1038675602},
    { 85, 1165413194},
    { 86, 1307615110},
    { 87, 1467168285},
    { 88, 1646189891},
    { 89, 1847055437},
    { 90, 2072430287},
    { 91, 2325305027},
    { 92, 2609035152},
    { 93, 2927385589},
    { 94, 3284580654},
    { 95, 3685360108},
    { 96, 4135042052},
    {INT8_MAX, UINT32_MAX}, /* Guarantees UINT32_MAX in search domain. */
};

#define AUDIO_OUT_BEEP_AMPLITUDE    (INT32_MAX)

/*- Private Variables --------------------------------------------------------*/
/*----- Input ----------------------------------------------------------------*/
static audio_input_callback_t m_audio_in_cb[AUDIO_INPUT_CALLBACKS_MAX];
static int m_audio_in_cb_count;

/*----- Output ---------------------------------------------------------------*/
static volatile enum audio_out_mode_ {
    AUDIO_OUT_MODE_OFF = 0,
    AUDIO_OUT_MODE_BEEP,
} m_audio_out_mode;

static struct audio_out_beep {
    uint32_t duration_samples;  /**< Duration in ms. */
    uint32_t elapsed_samples;   /**< Elapsed beep duration in ms. */
    uint16_t period;            /**< Period in samples. */
    uint16_t samples_high;      /**< Samples high. */
    uint16_t samples;           /**< Sample counter. */
    void (*cb)(void);           /**< Callback on beep completion. */
} m_audio_out_beep;

/*- Private Methods ----------------------------------------------------------*/
static uint32_t log2u32(uint32_t x)
{
    uint32_t n = 0;
    while (x >>= 1) {
        n++;
    }
    return n;
}

/*----- Output ---------------------------------------------------------------*/
static void prv_audio_out_beep_complete(struct audio_out_beep *beep)
{
    LOG("finished playing beep");
    m_audio_out_mode = AUDIO_OUT_MODE_OFF;
    if (NULL != beep->cb) {
        beep->cb();
    }
}

static int32_t prv_audio_out_beep_get_next_sample(struct audio_out_beep *beep)
{
    int32_t sample;
    uint32_t samples = beep->samples;
    uint32_t period = beep->period;
    if (samples < beep->samples_high) {
        sample = AUDIO_OUT_BEEP_AMPLITUDE;
    } else {
        sample = -AUDIO_OUT_BEEP_AMPLITUDE;
    }
    if (++samples >= period) {
        samples = 0;
    }
    beep->samples = samples;
    return sample;
}

/*- API ----------------------------------------------------------------------*/
audio_sample_t audio_rms(const audio_sample_t *samples, size_t len)
{
    if (len == 0) {
        return 0;
    }

    uint32_t accum = 0;
    for (size_t i = 0; i < len; i++) {
        int32_t sample = samples[i];
        int32_t square = sample * sample;
        int32_t contribution = square / len;
        accum += contribution;
    }
    return sqrtu32(accum);
}

audio_sample_t audio_peak(const audio_sample_t *samples, size_t len)
{
    if (len == 0) {
        return 0;
    }

    audio_sample_t peak = 0;
    for (size_t i = 0; i < len; i++) { 
        audio_sample_t sample = abs(samples[i]);
        peak = peak < sample ? sample : peak;
    } 
    return peak;
}

int8_t audio_dB(audio_sample_t _ref, audio_sample_t _raw)
{
    /* Check some easy cases. */
    if (_ref == 0) {
        return INT8_MAX;
    } else if (_raw == 0) {
        return INT8_MIN;
    }

    /* Calculate the ratio as a 16.16 fixed precision integer. */
    uint32_t ref = _ref;
    uint32_t raw = _raw << 16;
    uint32_t ratio = raw / ref;

    /* Search the table. */
    size_t index;
    /* Start with a quick bisection. */
    if (ratio < (1 << 16)) {
        /* negative dB */
        index = 0;
    } else {
        /* positive dB */
        index = DB_RATIO_TABLE_ZERO_INDEX;
    }
    /* Scan upward through the table to find a ratio that is less or equal. */
    for (; DB_RATIO_TABLE[index].ratio < ratio; index++);
    return DB_RATIO_TABLE[index].dB;
}

void audio_process_buffer(const audio_buffer_t *in, audio_buffer_t *out)
{
    /* Input samples. */
    if (0 < m_audio_in_cb_count) {
        audio_sample_t samples[AUDIO_BUFFER_FRAMES];
        for (size_t i = 0; i < AUDIO_BUFFER_FRAMES; i++) {
            samples[i] = in[1 + i * 2U];
        }
        for (unsigned i = 0; i < ARRAY_SIZE(m_audio_in_cb); i++) {
            if (NULL != m_audio_in_cb[i]) {
                m_audio_in_cb[i](samples, AUDIO_BUFFER_FRAMES);
            }
        }
    }

    /* Output samples. */
    if (AUDIO_OUT_MODE_BEEP == m_audio_out_mode) {
        struct audio_out_beep *beep = &m_audio_out_beep;
        if (beep->elapsed_samples < beep->duration_samples) {
            unsigned period = beep->period;
            if (UINT16_MAX != period) {
                /* Play the note. */
                for (size_t i = 0; i < AUDIO_BUFFER_LEN; i += AUDIO_BUFFER_CHANS) {
                    out[i] = prv_audio_out_beep_get_next_sample(beep);
                    /* Check if beep is finished. */
                    if (++(beep->elapsed_samples) == beep->duration_samples) {
                        prv_audio_out_beep_complete(beep);
                    }
                }
            } else {
                /* This is a rest. */
                for (size_t i = 0; i < AUDIO_BUFFER_LEN; i += AUDIO_BUFFER_CHANS) {
                    out[i] = 0U;
                    if (++(beep->elapsed_samples) == beep->duration_samples) {
                        prv_audio_out_beep_complete(beep);
                    }
                }
            }
        } else {
            for (size_t i = 0; i < AUDIO_BUFFER_LEN; i += AUDIO_BUFFER_CHANS) {
                out[i] = 0U;
            }
        }
    } else {
        /* Nothing is playing. */
        for (size_t i = 0; i < AUDIO_BUFFER_LEN; i += AUDIO_BUFFER_CHANS) {
            out[i] = 0U;
        }
    };
}

/*----- Input ----------------------------------------------------------------*/
int audio_in_add_cb(audio_input_callback_t cb)
{
    if (NULL == cb) {
        LOG("Cannot add NULL input callback.");
        return -EINVAL;
    } else if (ARRAY_SIZE(m_audio_in_cb) <= (unsigned) m_audio_in_cb_count) {
        LOG("No space to add input callback.");
        return -ENOMEM;
    } else {
        for (int i = 0; i < (int) ARRAY_SIZE(m_audio_in_cb); i++) {
            if (NULL == m_audio_in_cb[i]) {
                m_audio_in_cb_count++;
                m_audio_in_cb[i] = cb;
                LOG("Added input callback. (i: %d, count: %d)",
                    i, m_audio_in_cb_count);
                return i;
            }
        }
        LOG("Did not find space to add input callback.");
        return -ENOMEM;
    }
}

int audio_in_remove_cb(int i)
{
    if (((int) ARRAY_SIZE(m_audio_in_cb) <= i) || (i < 0)) {
        LOG("Input entry index out of range.");
        return -EINVAL;
    } else if (NULL == m_audio_in_cb[i]) {
        LOG("Input entry already empty.");
        return -ENOENT;
    } else {
        m_audio_in_cb[i] = NULL;
        m_audio_in_cb_count--;
        LOG("Removed input callback. (i: %d, count: %d)", i, m_audio_in_cb_count);
        return 0;
    }
}

int audio_in_cb_count()
{
    return m_audio_in_cb_count;
}

/*----- Output ---------------------------------------------------------------*/
int audio_out_beep_with_cb(uint16_t freq_hz, uint16_t dur_ms, void (*cb)(void))
{
    uint32_t period;
    enum audio_out_mode_ out_mode = AUDIO_OUT_MODE_OFF;
    if ((freq_hz == 0) && (dur_ms == 0)) {
        /* Cancel the current beep. */
        period = UINT16_MAX;
    } else if (freq_hz == 0 && cb != NULL) { 
        /* We're being asked to play a rest. */
        out_mode = AUDIO_OUT_MODE_BEEP;
        period = UINT16_MAX;
    } else if ((freq_hz < AUDIO_BEEP_FREQ_HZ_MIN)
               || (freq_hz > AUDIO_BEEP_FREQ_HZ_MAX)
               || (dur_ms < AUDIO_BEEP_DUR_MS_MIN)
               || (dur_ms > AUDIO_BEEP_DUR_MS_MAX))
    {
        return -1;
    } else {
        out_mode = AUDIO_OUT_MODE_BEEP;
        period = AUDIO_FS / freq_hz;
    }

    audio_lock();
    m_audio_out_mode = out_mode;
    m_audio_out_beep.duration_samples = dur_ms * (AUDIO_FS / 1000);
    m_audio_out_beep.elapsed_samples = 0;
    if (m_audio_out_beep.period != period) {
        m_audio_out_beep.period = period;
        m_audio_out_beep.samples_high = period / 2;
        m_audio_out_beep.samples = 0;
    }
    m_audio_out_beep.cb = cb;
    audio_unlock();
    LOG("playing beep (freq: %d, period: %u, duration: %u)", 
         freq_hz, period, dur_ms);
    return 0;
}

int audio_out_beep(uint16_t freqHz, uint16_t durMs)
{
    return audio_out_beep_with_cb(freqHz, durMs, NULL);
}

bool audio_is_playing(void) {
    return m_audio_out_mode == AUDIO_OUT_MODE_BEEP;
}

/*! @} */ // BADGE_AUDIO

