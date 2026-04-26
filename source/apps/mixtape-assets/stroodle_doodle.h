#ifndef MIXTAPE_STROODLE_DOODLE_H__
#define MIXTAPE_STROODLE_DOODLE_H__

#include "music.h"
#include "audio.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

static const struct audio_out_note STROODLE_DOODLE_NOTES[] = {
	{
		.v = 0,
		.ms = 0,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 0,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds4,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 0,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As2,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 5,
		.ms = 0,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 7,
		.ms = 0,
		.spec = {
			.callback = NULL,
			.frequency_hz = 65,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 0,
		.ms = 125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F4,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 5,
		.ms = 125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C4,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F6,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 4,
		.ms = 250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C5,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 6,
		.ms = 250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 0,
		.ms = 375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As4,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 5,
		.ms = 375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F4,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 0,
		.ms = 500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F4,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F5,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C4,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 5,
		.ms = 500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C5,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 7,
		.ms = 500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As2,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F6,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 4,
		.ms = 750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C5,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 6,
		.ms = 750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 0,
		.ms = 1000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G4,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 1000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 1000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds4,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 5,
		.ms = 1000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds5,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 7,
		.ms = 1000,
		.spec = {
			.callback = NULL,
			.frequency_hz = 78,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 1250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 4,
		.ms = 1250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G4,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 6,
		.ms = 1250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds5,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 0,
		.ms = 1500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G4,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 1500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 1500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds4,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 5,
		.ms = 1500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds5,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 7,
		.ms = 1500,
		.spec = {
			.callback = NULL,
			.frequency_hz = 87,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 0,
		.ms = 1750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds4,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 1750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds5,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 1750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 1750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 4,
		.ms = 1750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G4,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 5,
		.ms = 1750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As4,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 6,
		.ms = 1750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds5,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 7,
		.ms = 2000,
		.spec = {
			.callback = NULL,
			.frequency_hz = 65,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 2250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G6,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 4,
		.ms = 2250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds5,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 6,
		.ms = 2250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As5,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 7,
		.ms = 2500,
		.spec = {
			.callback = NULL,
			.frequency_hz = 65,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 2750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G6,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 4,
		.ms = 2750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds5,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 6,
		.ms = 2750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As5,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 0,
		.ms = 3000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 3000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G4,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 3000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 5,
		.ms = 3000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds4,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 7,
		.ms = 3000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 0,
		.ms = 3250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 3250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As4,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 3250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 3250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 4,
		.ms = 3250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As4,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 5,
		.ms = 3250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G4,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 6,
		.ms = 3250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F5,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 0,
		.ms = 3500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 3500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G4,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 3500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 5,
		.ms = 3500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds4,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 7,
		.ms = 3500,
		.spec = {
			.callback = NULL,
			.frequency_hz = 87,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 0,
		.ms = 3750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 3750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds4,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 3750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 3750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As2,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 4,
		.ms = 3750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As4,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 5,
		.ms = 3750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 6,
		.ms = 3750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F5,
			.duration_ms = 25,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.ms = 3875,
		.spec = {
			.type = AUDIO_OUT_TYPE_NONE,
		}
	}
};

static const struct audio_out_section STROODLE_DOODLE = {
	.length = ARRAY_SIZE(STROODLE_DOODLE_NOTES),
	.notes = STROODLE_DOODLE_NOTES,
	.next = NULL,
};

#endif
