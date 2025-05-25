/*!
 *  @file   audio.h
 *  @author Peter Maxwell Warasila
 *  @date   May 28, 2022
 *
 *  @brief  RVASec Badge Audio Driver
 *
 *------------------------------------------------------------------------------
 *
 */


#ifndef BADGE_C_AUDIO_H
#define BADGE_C_AUDIO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>

/*! @defgroup   BADGE_AUDIO Audio Driver
 *  @{
 */

/*- Public Macro ------------------------------------------------------------*/
#define AUDIO_SAMPLE_MAX        (INT16_MAX)     //!< Audio driver maximum sample value
#define AUDIO_SAMPLE_MIN        (INT16_MIN)     //!< Audio driver maximum sample value

#define AUDIO_FS    (48000) /**< Audio driver sample rate. */

#ifdef TARGET_SIMULATOR
  #define AUDIO_BUFFER_FRAMES   (256)
  #define AUDIO_BUFFER_CHANS    (1)
#else
  #ifdef TARGET_PICO
    #ifdef AUDIO_BUFFER_FRAMES /* This might already be defined for i2s on pio. */
      #if AUDIO_BUFFER_FRAMES != 48
        #error "Incompatbile value for I2S AUDIO_BUFFER_FRAMES!"
      #elif STEREO_BUFFER_SIZE != 96
        #error "Unexpected channel count in I2S STEREO_BUFFER_SIZE!"
      #endif
    #else
      #define AUDIO_BUFFER_FRAMES   (48)
      #define AUDIO_BUFFER_CHANS    (2)
    #endif /* AUDIO_BUFFER_FRAMES */
  #else
    #error "Target audio not configured!"
  #endif /* TARGET_PICO */
#endif /* TARGET_SIMULATOR */
#define AUDIO_BUFFER_LEN    (AUDIO_BUFFER_FRAMES * AUDIO_BUFFER_CHANS)

/*----- Input ----------------------------------------------------------------*/
#define AUDIO_INPUT_CALLBACKS_MAX (4) /*!< Maximum number of audio input callbacks simultaneously active. */

/*----- Output ---------------------------------------------------------------*/
#define AUDIO_OUT_VOICE_COUNT   (8)                     /**< Number of output voice slots. */
#define AUDIO_OUT_VOICE_ANY     (AUDIO_OUT_VOICE_COUNT) /**< Output using any available voice slot. */

#define AUDIO_OUT_SPEC_NES_SQUARE_DUTY_0    (UINT8_MAX / 8)
#define AUDIO_OUT_SPEC_NES_SQUARE_DUTY_1    (UINT8_MAX / 4)
#define AUDIO_OUT_SPEC_NES_SQUARE_DUTY_2    (UINT8_MAX / 2)
#define AUDIO_OUT_SPEC_NES_SQUARE_DUTY_3    (UINT8_MAX - (UINT8_MAX / 4)

#define AUDIO_OUT_SPEC_NES_NOISE_FREQ_0x0   (447443)
#define AUDIO_OUT_SPEC_NES_NOISE_FREQ_0x1   (223722)
#define AUDIO_OUT_SPEC_NES_NOISE_FREQ_0x2   (111861)
#define AUDIO_OUT_SPEC_NES_NOISE_FREQ_0x3   (55930)
#define AUDIO_OUT_SPEC_NES_NOISE_FREQ_0x4   (27965)
#define AUDIO_OUT_SPEC_NES_NOISE_FREQ_0x5   (18644)
#define AUDIO_OUT_SPEC_NES_NOISE_FREQ_0x6   (13983)
#define AUDIO_OUT_SPEC_NES_NOISE_FREQ_0x7   (11186)
#define AUDIO_OUT_SPEC_NES_NOISE_FREQ_0x8   (8860)
#define AUDIO_OUT_SPEC_NES_NOISE_FREQ_0x9   (7046)
#define AUDIO_OUT_SPEC_NES_NOISE_FREQ_0xA   (4710)
#define AUDIO_OUT_SPEC_NES_NOISE_FREQ_0xB   (3523)
#define AUDIO_OUT_SPEC_NES_NOISE_FREQ_0xC   (2349)
#define AUDIO_OUT_SPEC_NES_NOISE_FREQ_0xD   (1762)
#define AUDIO_OUT_SPEC_NES_NOISE_FREQ_0xE   (880)
#define AUDIO_OUT_SPEC_NES_NOISE_FREQ_0xF   (440)

#define AUDIO_BEEP_FREQ_HZ_MIN  (120)
#define AUDIO_BEEP_FREQ_HZ_MAX  (10000)
#define AUDIO_BEEP_DUR_MS_MIN   (1)
#define AUDIO_BEEP_DUR_MS_MAX   (30000)

/*- Public Types -------------------------------------------------------------*/
/** Integer type used for sample processing. */
typedef int16_t audio_sample_t;

/** Integer type used for samples in the audio buffer. */
typedef int32_t audio_buffer_t;

/*----- Input ----------------------------------------------------------------*/
/** Audio input callback.
 *
 *  @param  samples Input samples to be processed.
 *  @param  len     Number of input samples to be processed.
 */
typedef void (*audio_input_callback_t)(const audio_sample_t *samples, size_t len);

/*----- Output ---------------------------------------------------------------*/
/** Audio output waveform type. */
enum audio_out_type {
    AUDIO_OUT_TYPE_NONE = 0,

    AUDIO_OUT_TYPE_SQUARE,      /** Square wave. _|¯|_|¯ */
    AUDIO_OUT_TYPE_TRIANGE,     /** Triangle wave. /\/\ */
    AUDIO_OUT_TYPE_SAWTOOTH,    /** Sawtooth wave. |\_|\_ */
    AUDIO_OUT_TYPE_NES_NOISE,   /** NES LFSR noise. */
    AUDIO_OUT_TYPE_SAMPLES,     /** Raw samples. */

    AUDIO_OUT_TYPE_COUNT,
};

/** Output spec for square waveform. */
struct audio_out_spec_square {
    /** Duty cycle of the square wave. */
    uint8_t duty_cycle;
};

/** Output spec for triangle waveform. */
struct audio_out_spec_triangle {
    // TODO -PMW
    uint8_t dummy;
};

/** Output spec for sawtooth waveform. */
struct audio_out_spec_sawtooth {
    // TODO -PMW
    uint8_t dummy;
};

/** Output spec for NES LFSR noise. */
struct audio_out_spec_nes_noise {
    uint16_t lfsr_val;  /**< LFSR value to force. UINT16_MAX to not force load. */
    bool mode_flag;     /**< LFSR XOR operand bit 6 not bit 1. */
};

/** Bit depth of raw samples for output. */
enum audio_out_spec_samples_bit_depth {
    AUDIO_OUT_SPEC_SAMPLES_BIT_DEPTH_32 = 1,
    AUDIO_OUT_SPEC_SAMPLES_BIT_DEPTH_24 = (1 << 8),
    AUDIO_OUT_SPEC_SAMPLES_BIT_DEPTH_16 = (1 << 16),
    AUDIO_OUT_SPEC_SAMPLES_BIT_DEPTH_8  = (1 << 24),
};

/** Output spec for raw samples. */
struct audio_out_spec_samples {
    enum audio_out_spec_samples_bit_depth bit_depth; /**< Sample bit depth. */

    /** Ratio between provided and output sample rate.
     *
     *  Sample rates less than the target sample rate may be interpolated
     *  linearly between the provided samples.
     *
     *  @note   This must be an integer in the inclusive range [0, 4].
     */
    uint8_t rate_ratio;

    void *samples;      /**< Pointer to the array of samples. */
    uint32_t n_samples; /**< Count of samples (array length NOT array size). */
};

struct audio_out_spec {
    /*----- Common fields. -----*/
    /** Note completion callback.
     *
     *  @param  voice   Voice index.
     *  @param  spec    Provided spec struct pointer.
     */
    void (*callback)(int voice, const struct audio_out_spec *spec);
    uint16_t frequency_hz;      /**< Frequency of the "note" in Hz. */
    uint16_t duration_ms;       /**< Duration of the "note" in ms. */
    int16_t decay;              /**< Linear decay to add to the amplitude every sample. @note This may change -PMW */
    int16_t phase;              /**< Phase adjustment in samples. */
    int8_t amplitude_dBFS;      /**< Starting amplitude of the waveform. */
    bool restart;               /**< If the "note" should be restarted or continued with new parameters. */
    enum audio_out_type type;   /**< Type of output. */

    /*----- Type specific fields. -----*/
    union {
        struct audio_out_spec_square    square;     /**< Spec for square waveform. */
        struct audio_out_spec_triangle  triangle;   /**< Spec for triangle waveform. */
        struct audio_out_spec_sawtooth  sawtooth;   /**< Spec for sawtooth waveform. */
        struct audio_out_spec_nes_noise nes_noise;  /**< Spec for NES LFSR noise. */
        struct audio_out_spec_samples   samples;    /**< Spec for raw samples. */
    };
};

/*--------- Music ------------------------------------------------------------*/
/** Audio output music section. */
struct audio_out_note {
    /** Output spec for note. */
    struct audio_out_spec spec;
    /** Milliseconds since the start of the section to start the note at. */
    uint32_t ms;
    /** Voice to use to play the note. */
    uint8_t v;
};

/** Audio output music section. */
struct audio_out_section {
    /** Number of notes. */
    uint32_t length;

    /** Array of notes.
     * 
     *  @note   These should be in order by the `ms` field.
     */
    const struct audio_out_note *notes;

    /** Pointer to the next section to play. */
    const struct audio_out_section *next;
};

/** Audio output music section completition callback.
 *
 *  @param  prev    The section that just finished playing.
 *
 *  @return Pointer to the next section to play.
 *  @retval NULL    Do not play a new section.
 */
typedef const struct audio_out_section *
(*audio_out_section_callback_t)(const struct audio_out_section *prev);

/*- API ----------------------------------------------------------------------*/
/*----- Initialization -------------------------------------------------------*/
/*!
 *  @brief  Initialize and configure audio gpio
 *
 *  @subsection Input
 *  The input is configured
 */
void audio_init_gpio(void);

/*!
 *  @brief  Intialize audio driver
 */
void audio_init(void);

/*----- Runtime --------------------------------------------------------------*/
/** Update audio engine with any per-frame tasks (like volume). */
void audio_poll(void);

/** Process audio buffer.
 *
 *  @note   Should always be called with audio locked.
 *
 *  @param  in  Pointer to audio input buffer.
 *  @param  out Pointer to audio output buffer.
 */
void audio_process_buffer(const audio_buffer_t *in, audio_buffer_t *out);

/** Lock shared audio context. 
 *
 *  @note   If this wraps a mutex, it must be a recursive mutex.
 */
void audio_lock(void);

/** Unlock shared audio context.
 *
 *  @note   If this wraps a mutex, it must be a recursive mutex.
 */
void audio_unlock(void);

/*----- Input ----------------------------------------------------------------*/
/** Add audio input callback.
 *
 *  @param  cb  Callback to add.
 *
 *  @retval 0       The callback was added successfully.
 *  @retval -EINVAL The callback pointer was NULL.
 *  @retval -ENOMEM There is no space left in the table.
 */
int audio_in_add_cb(audio_input_callback_t cb);

/** Remove audio input callback.
 *
 *  @param  i   Index provided by audio_in_add_cb().
 *
 *  @retval 0       The callback was removed successfully.
 *  @retval -EINVAL The index is invalid.
 *  @retval -ENOENT The provided index is empty.
 */
int audio_in_remove_cb(int i);

/** Number of audio input callbacks registered.
 *
 *  @return Number of audio input callbacks registered.
 */
int audio_in_cb_count(void);

/*----- Output ---------------------------------------------------------------*/
/** Start playing an output waveform.
 *
 *  @param  v       Index of voice to use in range [0, AUDIO_OUT_VOICE_COUNT].
 *                  Use AUDIO_OUT_VOICE_ANY to automatically select.
 *  @param  spec    Pointer to output spec.
 *
 *  @return Negative error on failure or positive voice index of playing voice.
 */
int audio_out_play(int v, const struct audio_out_spec *spec);

/** Stop a playing output waveform.
 *
 *  @note   This function will return as successful if there is no currently
 *          playing output waveform in the specified voice index.
 *
 *  @param  v   Index of voice to stop in range [0, AUDIO_OUT_VOICE_COUNT]. Use
 *              AUDIO_OUT_VOICE_COUNT to stop all playing output waveforms.
 *
 *  @return Negative error on failure or positive voice index of stopped voice.
 *  @retval -EINVAL Voice index was out of range.
 */
int audio_out_stop(int v);

/*!
 *  @brief  Play an old fashioned beep on the speaker.
 *
 *  @note   To play a rest, provide a callback and a frequency of zero with a
 *          valid duration.
 *  @note   To stop playing beeps, provide a duration and frequency of zero.
 *
 *  @param  frequency      Frequency in Hertz
 *  @param  duration       Duration in milliseconds
 *  @param  beep_finished  function to call when beep is finished playing.
 *
 */
int audio_out_beep_with_cb(uint16_t freq, uint16_t duration, void (*beep_finished)(void));

/*!
 *  @brief  Play an old fashioned beep on the speaker.
 *
 *  @note   To play a rest, provide a callback and a frequency of zero with a
 *          valid duration.
 *  @note   To stop playing beeps, provide a duration and frequency of zero.
 *
 *  @param  frequency   Frequency in Hertz
 *  @param  duration    Duration in milliseconds
 */
static inline int audio_out_beep(uint16_t freq, uint16_t duration)
{
    return audio_out_beep_with_cb(freq, duration, NULL);
}

/** If the audio is on or not.
 *
 *  @retval true    Audio is playing.
 *  @retval false   Audio is not playing.
 */
bool audio_is_playing(void);

/*--------- Music ------------------------------------------------------------*/
/** Start playing a music section. 
 *
 *  @note   Playing music always takes lower priority for a given voice than
 *          sounds played using the direct API. In other words, the music is
 *          alwyas "in the background".
 *
 *  @param  section     Pointer to the section of music to play.
 *  @param  callback    Callback to run when the section finishes.
 *
 *  @retval 0   The section is now playing.
 */
int audio_out_music_play(const struct audio_out_section *section,
                         audio_out_section_callback_t callback);

/** If music is currently playing.
 *
 *  @retval true    Music is playing.
 *  @retval false   Music is not currently playing.
 */
bool audio_out_music_playing(void);

/** Pause the currently playing music section.
 *
 *  @param  pause   If true, pause the music otherwise unpause.
 *
 *  @retval 0       Music is playing.
 *  @retval 1       Music is paused.
 *  @retval -EINVAL There is no music currently playing.
 */
int audio_out_music_pause(bool pause);

/** If the music is currently paused.
 *
 *  @retval 0       Music is playing.
 *  @retval 1       Music is paused.
 *  @retval -EINVAL There is no music currently playing.
 */
int audio_out_music_paused(void);

/** Stop the currently playing music section.
 *
 *  @retval 0       The section was stopped successfully.
 *  @retval -EINVAL There was no music section playing.
 */
int audio_out_music_stop(void);

/*----- Utilities ------------------------------------------------------------*/
/** Get the RMS level.
 *
 *  @param  samples Pointer to buffer of samples.
 *  @param  len     Length of buffer of samples.
 *
 *  @return RMS level of samples.
 */
audio_sample_t audio_rms(const audio_sample_t *samples, size_t len);

/** Get the peak level.
 *
 *  @param  samples Pointer to buffer of samples.
 *  @param  len     Length of buffer of samples.
 *
 *  @return Peak level of samples.
 */
audio_sample_t audio_peak(const audio_sample_t *samples, size_t len);

/** Get ratio in dB.
 *
 *  @param  ref Reference level.
 *  @param  raw Raw level to compare.
 *
 *  @return Ratio in dB (20 log).
 */
int8_t audio_dB(audio_sample_t ref, audio_sample_t raw);

/** Get dBFS.
 *
 *  @param  raw Raw level to compare.
 *
 *  @return dBFS (20 log).
 */
static inline int8_t audio_dBFS(audio_sample_t raw)
{
    return audio_dB(AUDIO_SAMPLE_MAX, raw);
}

/** Get ratio from dB.
 *
 *  @param  dB  Ratio in dB.
 *
 *  @return Ratio referenced to INT16_MAX as 0 dB.
 */
int32_t audio_ratio(int8_t dB);

/*! @} */ // BADGE_AUDIO

#endif /* BADGE_C_AUDIO_H */
