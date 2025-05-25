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
#define LOG(...) do { if (log_audio) { printf("\r\n[audio] " __VA_ARGS__); } } while (0)
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
#if AUDIO_BUFFER_FRAMES != AUDIO_FRAMES_PER_CALLBACK
	#error "audio.h configuration for SDL simulator does not match audio_sim.c"
#endif

static SDL_AudioDeviceID audio_device_id;
static SDL_mutex *audio_mutex = NULL;

static void mixer_loop(__attribute__((unused)) void *userdata,
		       Uint8 *stream, __attribute__((unused)) int len)
{
	audio_buffer_t *out = (int32_t *) stream;

	SDL_LockMutex(audio_mutex);

	/* Pass out buffer as "fake" input buffer for now. -PMW */
	audio_process_buffer(out, out);

	SDL_UnlockMutex(audio_mutex);
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
	desired.format = AUDIO_S32SYS;
	desired.channels = AUDIO_BUFFER_CHANS; /* mono output */
	desired.samples = AUDIO_FRAMES_PER_CALLBACK;
	desired.callback = mixer_loop;

	audio_mutex = SDL_CreateMutex();
	if (!audio_mutex) {
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

	if ((desired.freq != obtained.freq)
	    || (desired.format != obtained.format)
	    || (desired.channels != obtained.channels)
	    || (desired.samples != obtained.samples)) {
		fprintf(stderr, "Obtained audio spec differs from desired.\n");
		return;
	}
	SDL_PauseAudioDevice(audio_device_id, 0);
#endif /* SIMULATOR_AUDIO */
}

void audio_poll(void)
{
	return;
}

void audio_lock(void)
{
#ifdef SIMULATOR_AUDIO
	SDL_LockMutex(audio_mutex);
#endif /* SIMULATOR_AUDIO */
}

void audio_unlock(void)
{
#ifdef SIMULATOR_AUDIO
	SDL_UnlockMutex(audio_mutex);
#endif /* SIMULATOR_AUDIO */
}

