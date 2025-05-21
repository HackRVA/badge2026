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

#define AUDIO_BEEP_FREQ_HZ_MIN  (120)
#define AUDIO_BEEP_FREQ_HZ_MAX  (10000)
#define AUDIO_BEEP_DUR_MS_MIN   (1)
#define AUDIO_BEEP_DUR_MS_MAX   (30000)

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


#define AUDIO_INPUT_CALLBACKS_MAX (4) /*!< Maximum number of audio input callbacks simultaneously active. */

/*- Public Types -------------------------------------------------------------*/
/** Integer type used for sample processing. */
typedef int16_t audio_sample_t;

/** Integer type used for samples in the audio buffer. */
typedef int32_t audio_buffer_t;

/** Audio input callback.
 *
 *  @param  samples Input samples to be processed.
 *  @param  len     Number of input samples to be processed.
 */
typedef void (*audio_input_callback_t)(const audio_sample_t *samples, size_t len);

/*- API ----------------------------------------------------------------------*/
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

/** Update audio engine with any per-frame tasks (like volume). */
void audio_poll(void);

/** Process audio buffer.
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
int audio_out_beep(uint16_t freq, uint16_t duration);

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
 *  @brief  Request the opamp standby pin take a certain state.
 *
 *  @param  enable  Request the standby mode be enabled
 */
void audio_stby_ctl(bool enable);

/*!
 * @brief Tell us Signal if the audio is on or not.
 */
bool audio_is_playing(void);

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

/*! @} */ // BADGE_AUDIO

#endif /* BADGE_C_AUDIO_H */
