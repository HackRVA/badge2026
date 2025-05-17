//
// Created by Samuel Jones on 2/21/22.
// Implemented by Stephen M. Cameron Sun 07 May 2023 06:06:22 PM EDT
//

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#ifdef SIMULATOR_AUDIO
#include <SDL_audio.h>
#include <SDL2/SDL.h>
#endif
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include "audio.h"
#include "badge.h"
#include "utils.h"

/* TODO: add logging system? -PMW */
#ifndef LOG
#define LOG(...) printf("\r\n[audio] " __VA_ARGS__)
#endif /* LOG */

#ifdef SIMULATOR_AUDIO
#define SAMPLE_RATE (48000)
/*
 * AUDIO_FRAMES_PER_CALLBACK
 * Audio buffer size in sample FRAMES
 * (total samples divided by channel count)
 * `baseaudiocontext.createScriptProcessor` in js (wasm)
 * wants a value from this set
 * [ 256, 512, 1024, 2048, 4096, 8192, 16384 ]
 * https://developer.mozilla.org/en-US/docs/Web/API/BaseAudioContext/createScriptProcessor
 */
#define AUDIO_FRAMES_PER_CALLBACK (256)

#define AUDIO_BUFFER_SIZE 48000
static float audio_buffer[AUDIO_BUFFER_SIZE] = {0};
static int audio_buffer_index = 0;
static int samples_left_to_play = 0;
static SDL_AudioDeviceID audio_device_id;
static SDL_mutex *audio_lock = NULL;
static void (*user_callback_fn)(void) = NULL;

static void mixer_loop(
	__attribute__((unused)) void *userdata, Uint8 *stream, int len)
{
	float *out = (float *)stream;
	int framesPerBuffer = len / sizeof(float);

	SDL_LockMutex(audio_lock);
	if (samples_left_to_play == 0 && user_callback_fn) {
		void (*temp_callback_fn)(void) = user_callback_fn;
		user_callback_fn = NULL;
		SDL_UnlockMutex(audio_lock);
		temp_callback_fn();
		SDL_LockMutex(audio_lock);
	}

	if (badge_system_data()->mute) {
		memset(out, 0, len);
	} else {
		for (int i = 0; i < framesPerBuffer; i++) {
			out[i] = audio_buffer[audio_buffer_index++];
			if (audio_buffer_index >= AUDIO_BUFFER_SIZE)
				audio_buffer_index = 0;
		}
	}
	samples_left_to_play -= framesPerBuffer;
	if (samples_left_to_play <= 0) {
		memset(audio_buffer, 0, sizeof(audio_buffer));
		samples_left_to_play = 0;
	}
	SDL_UnlockMutex(audio_lock);
}
#endif

void audio_init_gpio(void)
{
	return;
}

void audio_init(void)
{
#ifdef SIMULATOR_AUDIO
	printf("Initializing SDL audio...\n");
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "SDL audio init failed: %s\n", SDL_GetError());
		return;
	}

	SDL_AudioSpec desired, obtained;
	SDL_zero(desired);
	desired.freq = SAMPLE_RATE;
	desired.format = AUDIO_F32SYS;
	desired.channels = 1; /* mono output */
	desired.samples = AUDIO_FRAMES_PER_CALLBACK;
	desired.callback = mixer_loop;

	audio_lock = SDL_CreateMutex();
	if (!audio_lock) {
		fprintf(stderr, "SDL mutex creation failed: %s\n",
			SDL_GetError());
		return;
	}

	/* the NULL arg tells SDL to try to select the device automaticallly */
	audio_device_id = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
	if (audio_device_id == 0) {
		fprintf(stderr, "SDL_OpenAudioDevice failed: %s\n",
			SDL_GetError());
		return;
	}

	SDL_PauseAudioDevice(audio_device_id, 0);
#endif
}

/*- Input --------------------------------------------------------------------*/
// FIXME: move this to audio_common.c. -PMW
static audio_input_callback_t m_audio_in_cb[AUDIO_INPUT_CALLBACKS_MAX];
static int m_audio_in_cb_count;
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

/*- Output -------------------------------------------------------------------*/
int audio_out_beep_with_cb(
	uint16_t freq, uint16_t duration, void (*beep_finished)(void))
{
#ifdef SIMULATOR_AUDIO
	float value = -0.025;

	if ((freq == 0) || (duration == 0)) {
		/* Stop playing beep. */
		SDL_LockMutex(audio_lock);
		memset(audio_buffer, 0, sizeof(audio_buffer));
		user_callback_fn = NULL;
		audio_buffer_index = 0;
		samples_left_to_play = 0;
		SDL_UnlockMutex(audio_lock);
		return 0;
	}

	if (duration <= 0)
		return 0;

	if (freq == 0) {
		if (beep_finished == NULL) /* no callback provided?  Ok... */
			return 0;
		/* We're being asked to play a rest? Ok. */
		SDL_LockMutex(audio_lock);
		memset(audio_buffer, 0, sizeof(audio_buffer));
		user_callback_fn = beep_finished;
		audio_buffer_index = 0;
		samples_left_to_play = duration * 48;
		if (samples_left_to_play > AUDIO_BUFFER_SIZE)
			samples_left_to_play = AUDIO_BUFFER_SIZE;
		SDL_UnlockMutex(audio_lock);
		return 0;
	}

	int count = AUDIO_BUFFER_SIZE / freq / 2;
	SDL_LockMutex(audio_lock);
	for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
		audio_buffer[i] = value;
		if ((i % count) == 0)
			value = -value;
	}
	user_callback_fn = beep_finished;
	audio_buffer_index = 0;
	samples_left_to_play = duration * 48;
	if (samples_left_to_play > AUDIO_BUFFER_SIZE)
		samples_left_to_play = AUDIO_BUFFER_SIZE;
	SDL_UnlockMutex(audio_lock);
#endif
	return 0;
}

int audio_out_beep(uint16_t freq, uint16_t duration)
{
	return audio_out_beep_with_cb(freq, duration, NULL);
}

void audio_stby_ctl(__attribute__((__unused__)) bool enabled)
{
	return;
}
