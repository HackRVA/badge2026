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
#include "errno.h"
#include "rtc.h"
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
static const struct {int8_t dB; uint32_t ratio;} DB_RATIO_TABLE[] = {
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

#define DB_TO_RATIO_I32_TABLE_ZERO_OFFSET   (-INT8_MIN)
static int32_t DB_TO_RATIO_I32_TABLE[256] = {
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             0,
             1,
             1,
             1,
             1,
             1,
             1,
             2,
             2,
             2,
             2,
             3,
             3,
             4,
             4,
             5,
             5,
             6,
             7,
             8,
             9,
            10,
            11,
            13,
            14,
            16,
            18,
            20,
            23,
            26,
            29,
            32,
            36,
            41,
            46,
            51,
            58,
            65,
            73,
            82,
            92,
           103,
           116,
           130,
           146,
           164,
           184,
           206,
           231,
           260,
           292,
           327,
           367,
           412,
           462,
           519,
           582,
           653,
           733,
           823,
           923,
          1036,
          1162,
          1304,
          1463,
          1642,
          1842,
          2067,
          2319,
          2602,
          2920,
          3276,
          3676,
          4125,
          4628,
          5193,
          5826,
          6537,
          7335,
          8230,
          9234,
         10361,
         11626,
         13044,
         14636,
         16422,
         18426,
         20674,
         23197,
         26027,
         29203,
         32767,
         36765,
         41251,
         46284,
         51932,
         58268,
         65378,
         73356,
         82306,
         92349,
        103618,
        116261,
        130447,
        146364,
        164224,
        184262,
        206745,
        231972,
        260277,
        292036,
        327670,
        367651,
        412512,
        462846,
        519321,
        582688,
        653787,
        733561,
        823069,
        923499,
       1036183,
       1162617,
       1304477,
       1463648,
       1642240,
       1842623,
       2067457,
       2319725,
       2602775,
       2920361,
       3276700,
       3676517,
       4125120,
       4628461,
       5193219,
       5826888,
       6537876,
       7335617,
       8230698,
       9234995,
      10361835,
      11626170,
      13044777,
      14636481,
      16422402,
      18426238,
      20674579,
      23197259,
      26027753,
      29203619,
      32767000,
      36765178,
      41251208,
      46284617,
      51932195,
      58268881,
      65378760,
      73356175,
      82306982,
      92349953,
     103618352,
     116261703,
     130447776,
     146364812,
     164224020,
     184262382,
     206745793,
     231972595,
     260277532,
     292036194,
     327670000,
     367651786,
     412512089,
     462846177,
     519321952,
     582688814,
     653787602,
     733561755,
     823069827,
     923499535,
    1036183520,
    1162617032,
    1304477765,
    1463648126,
    1642240208,
    1842623820,
    2067457930,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
    2147483647,
};

/*- Private Types ------------------------------------------------------------*/
/*----- Output ---------------------------------------------------------------*/
/** Output waveform voice context for square waveform. */
struct audio_out_voice_ctx_square {
    uint16_t period;
    uint16_t samples_high;
    uint16_t samples;
};

/** Output waveform voice context for triangle waveform. */
struct audio_out_voice_ctx_triangle {
    // TODO -PMW
    uint8_t dummy;
};

/** Output waveform voice context for sawtooth waveform. */
struct audio_out_voice_ctx_sawtooth {
    // TODO -PMW
    uint8_t dummy;
};

/** Output waveform voice context for NES LFSR noise. */
struct audio_out_voice_ctx_nes_noise {
    uint16_t period;
    uint16_t samples;
    uint16_t lfsr;
    uint16_t mode_tap;
};

/** Output waveform voice context for raw samples. */
struct audio_out_voice_ctx_samples {
    // TODO -PMW
    uint8_t dummy;
};

/** Audio output waveform voice context. */
struct audio_out_voice_ctx {
    /*----- Common fields. -----*/
    /** Note completion callback.
     *
     *  @param  voice   Voice index.
     *  @param  spec    Provided spec struct pointer.
     */
    void (*callback)(int voice, const struct audio_out_spec *spec);
    const struct audio_out_spec *spec;  /**< Audio output waveform spec. */
    uint32_t duration_samples;          /**< Duration in ms. */
    uint32_t elapsed_samples;           /**< Elapsed beep duration in ms. */
    int32_t amplitude;                  /**< Amplitude. */
    int16_t decay;                      /**< Linear decay to add every sample. */
    enum audio_out_type type;           /**< Audio output waveform type. */
    bool music;                         /**< Music is playing on this voice. */

    /*----- Type specific fields. -----*/
    union {
        struct audio_out_voice_ctx_square       square;
        struct audio_out_voice_ctx_triangle     triangle;
        struct audio_out_voice_ctx_sawtooth     sawtooth;
        struct audio_out_voice_ctx_nes_noise    nes_noise;
        struct audio_out_voice_ctx_samples      samples;
    };
};

/*--------- Music ------------------------------------------------------------*/
struct audio_out_music_ctx {
    /** Current note index. */
    uint32_t i;
    /** Milliseconds count when the section was started. */
    uint32_t start_ms;
    /** The ms_diff when the game was paused.*/
    uint32_t ms_paused;
    /** If the music is paused. */
    bool paused;
    /** Currently playing music section. */
    struct audio_out_section section;
    /** Section completion callback. */
    audio_out_section_callback_t callback;

};

/*- Private Variables --------------------------------------------------------*/
/*----- Input ----------------------------------------------------------------*/
static audio_input_callback_t m_audio_in_cb[AUDIO_INPUT_CALLBACKS_MAX];
static int m_audio_in_cb_count;

/*----- Output ---------------------------------------------------------------*/
static struct audio_out_voice_ctx m_audio_out_voices[AUDIO_OUT_VOICE_COUNT];

/*--------- Music ------------------------------------------------------------*/
static struct audio_out_music_ctx m_audio_out_music;

/*- Private Methods ----------------------------------------------------------*/
static bool prv_audio_out_music_playing(void);

static uint32_t log2u32(uint32_t x)
{
    uint32_t n = 0;
    while (x >>= 1) {
        n++;
    }
    return n;
}

/* This is a static inline so the compiler can optimize to the underlying
 * constant value within this compilation unit. -PMW
 */
static inline int32_t prv_audio_ratio(int8_t dB)
{
    return DB_TO_RATIO_I32_TABLE[dB - INT8_MIN];
}

/*----- Input ----------------------------------------------------------------*/
static void prv_audio_process_input(const audio_buffer_t *in)
{
    if (0 < m_audio_in_cb_count) {
        audio_sample_t samples[AUDIO_BUFFER_FRAMES];
        for (size_t i = 0; i < AUDIO_BUFFER_FRAMES; i++) {
            samples[i] = in[i * AUDIO_BUFFER_CHANS];
        }
        for (unsigned i = 0; i < ARRAY_SIZE(m_audio_in_cb); i++) {
            if (NULL != m_audio_in_cb[i]) {
                m_audio_in_cb[i](samples, AUDIO_BUFFER_FRAMES);
            }
        }
    }
}

/*----- Output ---------------------------------------------------------------*/
/*--------- Square Wave _|¯|_|¯ ----------------------------------------------*/
static int prv_audio_out_square_setup(struct audio_out_voice_ctx *voice,
                                      const struct audio_out_spec *spec)
{
    uint16_t period;
    if (0 == spec->frequency_hz) {
        period = UINT16_MAX;
    } else {
        period = AUDIO_FS / spec->frequency_hz;
        if (2 >= period) {
            /* Keep it below Nyquist. */
            period = 2;
        }
    }
    voice->square.period = period;
    voice->square.samples_high = period * spec->square.duty_cycle / UINT8_MAX;
    if (spec->restart) {
        voice->square.samples = 0;
    }
#ifdef TARGET_SIMULATOR
    LOG("playing square wave "
        "(voice: %d, freq: %u, dur_ms: %u, duty: %u,"
        " period: %u, samples_high: %u, duration_samples: %u)",
        (int) (voice - m_audio_out_voices), spec->frequency_hz,
        spec->duration_ms, spec->square.duty_cycle, period,
        voice->square.samples_high, voice->duration_samples);
#endif
    return 0;
}

static int32_t prv_audio_out_square_step(struct audio_out_voice_ctx *voice)
{
    int32_t sample;
    uint32_t samples = voice->square.samples;
    uint32_t period = voice->square.period;
    if (samples < voice->square.samples_high) {
        sample = voice->amplitude;
    } else {
        sample = -voice->amplitude;
    }
    if (++samples >= period) {
        samples = 0;
    }
    voice->square.samples = samples;
    return sample;
}

/*--------- NES LFSR noise ---------------------------------------------------*/
static int prv_audio_out_nes_noise_setup(struct audio_out_voice_ctx *voice,
                                         const struct audio_out_spec *spec)
{
    uint16_t period;
    if (0 == spec->frequency_hz) {
        period = UINT16_MAX;
    } else {
        period = AUDIO_FS / spec->frequency_hz;
        if (1 >= period) {
            period = 1;
        }
    }
    voice->nes_noise.period = period;
    voice->nes_noise.mode_tap = spec->nes_noise.mode_flag ? 1 << 6 : 1 << 1;
    if (spec->restart) {
        voice->nes_noise.samples = 0;
    }
    if (spec->nes_noise.lfsr_val != UINT16_MAX) {
        voice->nes_noise.lfsr = spec->nes_noise.lfsr_val & 0x7FFFU;
    } else if (0 == voice->nes_noise.lfsr) {
        /* The LFSR is loaded with 1 on NES power up. */
        voice->nes_noise.lfsr = 0x0001U;
    }
#ifdef TARGET_SIMULATOR
    LOG("playing nes_noise "
        "(voice: %d, freq: %u, dur_ms: %u, mode_flag: %u,"
        " lfsr: 0x%04X, period: %u, duration_samples: %u)",
        (int) (voice - m_audio_out_voices), spec->frequency_hz,
        spec->duration_ms, spec->nes_noise.mode_flag, voice->nes_noise.lfsr,
        period, voice->duration_samples);
#endif
    return 0;
}

static int32_t prv_audio_out_nes_noise_step(struct audio_out_voice_ctx *voice)
{
    uint32_t samples = voice->nes_noise.samples;
    uint32_t period = voice->nes_noise.period;
    if (++samples >= period) {
        samples = 0;
        uint16_t lfsr = voice->nes_noise.lfsr;
        uint16_t feedback = lfsr & 0x0001U;
        feedback ^= 0U != (lfsr & voice->nes_noise.mode_tap) ? 1U : 0U;
        feedback <<= 14;
        lfsr >>= 1;
        lfsr |= feedback;
        voice->nes_noise.lfsr = lfsr;
    }
    voice->nes_noise.samples = samples;
    int32_t sample = voice->nes_noise.lfsr;
    sample -= INT16_MAX / 2; /* Remove DC bias. */
    sample *= voice->amplitude;
    sample >>= 13; /* Correct for amplitude and DC bias removal above. */
    return sample;
}

static void prv_audio_out_complete(struct audio_out_voice_ctx *voice)
{
    int v = voice - m_audio_out_voices;
#ifdef TARGET_SIMULATOR
    LOG("finished playing (voice: %d)", v);
#endif
    voice->type = AUDIO_OUT_TYPE_NONE;
    if (NULL != voice->callback) {
        voice->callback(v, voice->spec);
    }
    /* If there's no note playing after the callback, reset state. */
    if (AUDIO_OUT_TYPE_NONE == voice->type) {
        memset(voice, 0, sizeof(*voice));
    }
}

static int prv_audio_out_play(int v, const struct audio_out_spec *spec, bool music)
{
    if ((0 > v) || ((int) ARRAY_SIZE(m_audio_out_voices) < v)) {
        LOG("Voice index out of range.");
        return -EINVAL;
    }

    if (AUDIO_OUT_VOICE_ANY != v) {
        if (music 
            && (AUDIO_OUT_TYPE_NONE != m_audio_out_voices[v].type) 
            && !m_audio_out_voices[v].music) {
            /* Music voices should not override the direct API voices. */
            return -ENOTEMPTY;
        }
    } else {
        for (int i = 0; i < (int) ARRAY_SIZE(m_audio_out_voices); i++) {
            if (AUDIO_OUT_TYPE_NONE == m_audio_out_voices[i].type) {
                v = i;
            }
        }
        if (AUDIO_OUT_VOICE_ANY == v) {
            LOG("No free output voice.");
            return -ENOMEM;
        }
    }

    // TODO: check for valid type -PMW

    audio_lock();
    struct audio_out_voice_ctx *voice = m_audio_out_voices + v;

    /* Do common initialization first. */
    voice->callback = spec->callback;
    voice->spec = spec;
    voice->duration_samples = spec->duration_ms * (AUDIO_FS / 1000U);
    voice->elapsed_samples = 0U;
    voice->amplitude = prv_audio_ratio(spec->amplitude_dBFS);
    voice->decay = spec->decay;
    voice->type = spec->type;
    voice->music = music;

    int rc = -1;
    switch (voice->type) {
        case AUDIO_OUT_TYPE_NONE:
        case AUDIO_OUT_TYPE_TRIANGE:
        case AUDIO_OUT_TYPE_SAWTOOTH:
        case AUDIO_OUT_TYPE_SAMPLES:
        default:
            /* Not implemented. */
            break;
        case AUDIO_OUT_TYPE_SQUARE:
            rc = prv_audio_out_square_setup(voice, spec);
            break;
        case AUDIO_OUT_TYPE_NES_NOISE:
            rc = prv_audio_out_nes_noise_setup(voice, spec);
            break;
    }

    if (0 != rc) {
        memset(voice, 0, sizeof(*voice));
    }
    audio_unlock();

    return v;
}

/*--------- Music ------------------------------------------------------------*/
static void prv_audio_out_music_section_finished(void) {
#ifdef TARGET_SIMULATOR
        LOG("finished section");
#endif /* TARGET_SIMULATOR */
        const struct audio_out_section *prev = &m_audio_out_music.section;
        const struct audio_out_section *next = prev->next;
        if (NULL != m_audio_out_music.callback) {
            next = m_audio_out_music.callback(prev);
        }
        if (NULL != next) {
            audio_out_music_play(next, m_audio_out_music.callback);
        } else {
            audio_out_music_stop();
        }
}

static void prv_audio_process_output(audio_buffer_t *out)
{
    /* Check music. */
    if (prv_audio_out_music_playing() && !m_audio_out_music.paused) {
        uint32_t ms_now = rtc_get_ms_since_boot();
        uint32_t ms_diff = ms_now - m_audio_out_music.start_ms;
        uint32_t i = m_audio_out_music.i;
#ifdef TARGET_SIMULATOR
#if 0
        LOG("playing music (ms_now: %u, start_ms: %u, ms_diff: %u, i: %u "
            "next_ms: %u)",
            ms_now, m_audio_out_music.start_ms, ms_diff, i,
            m_audio_out_music.section.notes[i].ms);
#endif
#endif /* TARGET_SIMULATOR */
        while ((i < m_audio_out_music.section.length)
               && (m_audio_out_music.section.notes[i].ms <= ms_diff)) {
            const struct audio_out_note *note = m_audio_out_music.section.notes + i;
            (void) prv_audio_out_play(note->v, &note->spec, true);
            i++;
        }
        m_audio_out_music.i = i;
        if (m_audio_out_music.section.length <= i) {
            /* Section complete! */
            prv_audio_out_music_section_finished();
        }
    }

    /* Process samples. */
    for (size_t o = 0; o < AUDIO_BUFFER_LEN; o += AUDIO_BUFFER_CHANS) {
        int32_t sample = 0;
        for (int v = 0; v < (int) ARRAY_SIZE(m_audio_out_voices); v++) {
            struct audio_out_voice_ctx *voice = m_audio_out_voices + v;
            switch (voice->type) {
                case AUDIO_OUT_TYPE_NONE:
                case AUDIO_OUT_TYPE_TRIANGE: // TODO: implement -PMW
                case AUDIO_OUT_TYPE_SAWTOOTH: // TODO: implement -PMW
                case AUDIO_OUT_TYPE_SAMPLES: // TODO: implement -PMW
                default:
                    /* No contribution to this sample. */
                    break;
                case AUDIO_OUT_TYPE_SQUARE:
                    sample += prv_audio_out_square_step(voice);
                    break;
                case AUDIO_OUT_TYPE_NES_NOISE:
                    sample += prv_audio_out_nes_noise_step(voice);
                    break;
            }
            switch (voice->type) {
                case AUDIO_OUT_TYPE_NONE:
                case AUDIO_OUT_TYPE_TRIANGE: // TODO: implement -PMW
                case AUDIO_OUT_TYPE_SAWTOOTH: // TODO: implement -PMW
                case AUDIO_OUT_TYPE_SAMPLES: // TODO: implement -PMW
                default:
                    /* Post step actions for these types. */
                    break;
                case AUDIO_OUT_TYPE_SQUARE:
                case AUDIO_OUT_TYPE_NES_NOISE:
                    if (++(voice->elapsed_samples) >= voice->duration_samples) {
                        prv_audio_out_complete(voice);
                    } else {
                        voice->amplitude += voice->decay;
                    }
                    break;
            }
        }
        /* Rescale to about -12 dBFS referenced to INT32_MAX.
         *
         * Because the dB table is referenced to INT16_MAX, multiplying them
         * twice gives the spec dB + -12 dB. However, there is a missing power
         * of two to get the full INT32_MAX (2^31 - 1) value... or there about.
         *
         * The actual "full-scale" value here is:
         *
         *      ref = 0x7FFF * 0x7FFF * 2 =>
         *      ref = 0x7FFE0002
         *
         * Close enough for badgernment work.
         */
        if (sample > (INT32_MAX / 2 / prv_audio_ratio(-12))) {
            sample = INT32_MAX;
        } else if ( sample < ((INT32_MIN + 1) / 2 / prv_audio_ratio(-12))) {
            sample = INT32_MIN;
        } else {
            sample *= prv_audio_ratio(-12) * 2;
        }
        out[o] = sample;
    }
}

/*- API ----------------------------------------------------------------------*/
/*----- Runtime --------------------------------------------------------------*/
void audio_process_buffer(const audio_buffer_t *in, audio_buffer_t *out)
{
    prv_audio_process_input(in);
    prv_audio_process_output(out);
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
int audio_out_play(int v, const struct audio_out_spec *spec)
{
    return prv_audio_out_play(v, spec, false);
}

int audio_out_stop(int v) {
    if ((0 > v) || ((int) ARRAY_SIZE(m_audio_out_voices) < v)) {
        LOG("voice index out of range");
        return -EINVAL;
    }
    audio_lock();
    if (v == AUDIO_OUT_VOICE_COUNT) {
        for (int i = 0; i < (int) ARRAY_SIZE(m_audio_out_voices); i++) {
            if (AUDIO_OUT_TYPE_NONE != m_audio_out_voices[i].type) {
                prv_audio_out_complete(m_audio_out_voices + i);
            }
        }
    } else if (AUDIO_OUT_TYPE_NONE != m_audio_out_voices[v].type) {
        prv_audio_out_complete(m_audio_out_voices + v);
    }
    audio_unlock();
#ifdef TARGET_SIMULATOR
    LOG("stopped playing (voice: %d)", v);
#endif /* TARGET_SIMULATOR */
    return v;
}

#define AUDIO_OUT_VOICE_BEEP            (AUDIO_OUT_VOICE_COUNT - 1)
#define AUDIO_OUT_BEEP_AMPLITUDE_DBFS   (-3)

static void (*m_audio_out_beep_callback)(void);

static void prv_audio_out_beep_cb(int v, const struct audio_out_spec *spec)
{
    (void) v; (void) *spec;
    if (NULL != m_audio_out_beep_callback) {
        m_audio_out_beep_callback();
    }
}

int audio_out_beep_with_cb(uint16_t freq_hz, uint16_t dur_ms, void (*cb)(void))
{
    static struct audio_out_spec spec;
    if ((freq_hz == 0) && (dur_ms == 0)) {
        /* Cancel the current beep. */
        int rc = audio_out_stop(AUDIO_OUT_VOICE_BEEP);
        return AUDIO_OUT_VOICE_BEEP == rc ? 0 : rc;
    } else if ((0 == freq_hz) && (NULL != cb)) {
        /* We're being asked to play a rest. */
        spec.amplitude_dBFS = INT8_MIN;
    } else if ((freq_hz < AUDIO_BEEP_FREQ_HZ_MIN)
               || (freq_hz > AUDIO_BEEP_FREQ_HZ_MAX)
               || (dur_ms < AUDIO_BEEP_DUR_MS_MIN)
               || (dur_ms > AUDIO_BEEP_DUR_MS_MAX))
    {
        return -1;
    } else {
        spec.amplitude_dBFS = AUDIO_OUT_BEEP_AMPLITUDE_DBFS;
    }

    spec.callback = prv_audio_out_beep_cb;
    spec.frequency_hz = freq_hz;
    spec.duration_ms = dur_ms;
    spec.decay = 0;
    spec.phase = 0;
    spec.restart = false;
    spec.type = AUDIO_OUT_TYPE_SQUARE;
    spec.square.duty_cycle = 128;
    int rc = audio_out_play(AUDIO_OUT_VOICE_BEEP, &spec);
    if (AUDIO_OUT_VOICE_BEEP == rc) {
        m_audio_out_beep_callback = cb;
        return 0;
    } else {
        LOG("failed to play beep (rc: %d)", rc);
        return -2;
    }
}

bool audio_is_playing(void) {
    for (int i = 0; i < (int) ARRAY_SIZE(m_audio_out_voices); i++) {
        if (AUDIO_OUT_TYPE_NONE == m_audio_out_voices[i].type) {
            return true;
        }
    }
    return false;
}

static bool prv_audio_out_music_playing(void)
{
    return 0U != m_audio_out_music.section.length;
}

/*--------- Music ------------------------------------------------------------*/
int audio_out_music_play(const struct audio_out_section *section,
                         audio_out_section_callback_t callback)
{
    if ((NULL == section) || (0 == section->length)
        || (NULL == section->notes)) {
        LOG("arguments to play music invalid");
        return -EINVAL;
    }

    audio_lock();
    m_audio_out_music.i = 0;
    m_audio_out_music.start_ms = rtc_get_ms_since_boot();
    m_audio_out_music.paused = false;
    m_audio_out_music.ms_paused = m_audio_out_music.start_ms;
    m_audio_out_music.section = *section;
    m_audio_out_music.callback = callback;
    audio_unlock();

    return 0;
}

bool audio_out_music_playing(void)
{
    return prv_audio_out_music_playing();    
}

int audio_out_music_pause(bool pause)
{
    audio_lock();
    if (prv_audio_out_music_playing()) {
        bool paused = m_audio_out_music.paused;
        if (paused != pause) {
            uint32_t ms_now = rtc_get_ms_since_boot();
            if (pause) {
                /* Pause. */
                m_audio_out_music.ms_paused =
                    ms_now - m_audio_out_music.start_ms;
            } else {
                /* Unpause. */
                m_audio_out_music.start_ms =
                    ms_now - m_audio_out_music.ms_paused;
            }
            m_audio_out_music.paused = pause;
        }
    }
    audio_unlock();
    return audio_out_music_paused();
}

int audio_out_music_paused(void)
{
    if (!prv_audio_out_music_playing()) {
        return -EINVAL;
    } else {
        return m_audio_out_music.paused ? 1 : 0;
    }
}

int audio_out_music_stop(void)
{
    int rc = 0;
    audio_lock();
    if (!prv_audio_out_music_playing()) {
        rc = -EINVAL;
    } else {
        if (NULL != m_audio_out_music.callback) {
            m_audio_out_music.callback(&m_audio_out_music.section);
        }
        for (int i = 0; i < (int) ARRAY_SIZE(m_audio_out_voices); i++) {
            if (m_audio_out_voices[i].music) {
                (void) audio_out_stop(i);
            }
        }
        memset(&m_audio_out_music, 0, sizeof(m_audio_out_music));
    }
    audio_unlock();
    return rc;
}

/*----- Utilities ------------------------------------------------------------*/
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

int32_t audio_ratio(int8_t dB)
{
    return prv_audio_ratio(dB);
}

/*! @} */ // BADGE_AUDIO

