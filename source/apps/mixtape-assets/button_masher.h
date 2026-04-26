#ifndef MIXTAPE_BUTTON_MASHER_H__
#define MIXTAPE_BUTTON_MASHER_H__

#include "music.h"
#include "audio.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif


static const struct audio_out_note BUTTON_MASHER_NOTES[] = {
	{
		.v = 4,
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
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0xFFFF,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 7,
		.ms = 6,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 30,
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
		.ms = 6,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 30,
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
		.ms = 131,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
			.duration_ms = 30,
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
		.ms = 131,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As5,
			.duration_ms = 30,
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
		.ms = 250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
			.duration_ms = 250,
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
		.ms = 256,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F6,
			.duration_ms = 30,
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
		.ms = 256,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 30,
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
		.ms = 381,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G6,
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
		.ms = 381,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
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
		.ms = 381,
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
		.v = 4,
		.ms = 500,
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
		.v = 0,
		.ms = 500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x1234,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As5,
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
		.ms = 625,
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
		.ms = 750,
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
		.ms = 875,
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
		.v = 4,
		.ms = 1000,
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
		.v = 0,
		.ms = 1000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0xFFFF,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 1000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
			.duration_ms = 250,
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
		.ms = 1000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 250,
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
			.frequency_hz = NOTE_G5,
			.duration_ms = 250,
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
			.frequency_hz = NOTE_G4,
			.duration_ms = 250,
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
		.ms = 1000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C5,
			.duration_ms = 250,
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
		.v = 5,
		.ms = 1250,
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
		.v = 4,
		.ms = 1500,
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
		.v = 0,
		.ms = 1500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x1234,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 1500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G6,
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
		.v = 5,
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
		.v = 0,
		.ms = 2000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0xFFFF,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 4,
		.ms = 2000,
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
		.v = 1,
		.ms = 2000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
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
		.ms = 2250,
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
		.v = 7,
		.ms = 2496,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F6,
			.duration_ms = 30,
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
		.ms = 2496,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 30,
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
		.ms = 2500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x1234,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 4,
		.ms = 2500,
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
		.v = 1,
		.ms = 2500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F5,
			.duration_ms = 30,
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
		.ms = 2500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C5,
			.duration_ms = 30,
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
		.ms = 2554,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G6,
			.duration_ms = 30,
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
		.ms = 2554,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
			.duration_ms = 30,
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
		.ms = 2558,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 30,
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
		.ms = 2558,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds5,
			.duration_ms = 30,
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
		.ms = 2621,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G6,
			.duration_ms = 30,
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
		.ms = 2621,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
			.duration_ms = 30,
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
		.ms = 2625,
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
		.ms = 2625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 30,
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
		.ms = 2625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds5,
			.duration_ms = 30,
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
		.ms = 2679,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As6,
			.duration_ms = 30,
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
		.ms = 2679,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F6,
			.duration_ms = 30,
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
		.ms = 2683,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As5,
			.duration_ms = 30,
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
		.ms = 2683,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F5,
			.duration_ms = 30,
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
		.ms = 2746,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As6,
			.duration_ms = 30,
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
		.ms = 2746,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F6,
			.duration_ms = 30,
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
		.ms = 2750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As5,
			.duration_ms = 30,
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
			.frequency_hz = NOTE_F5,
			.duration_ms = 30,
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
		.ms = 2804,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As6,
			.duration_ms = 30,
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
		.ms = 2804,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G6,
			.duration_ms = 30,
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
		.ms = 2808,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 30,
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
		.ms = 2808,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 30,
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
		.ms = 2875,
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
		.ms = 2875,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
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
		.ms = 2875,
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
		.v = 5,
		.ms = 2875,
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
		.v = 0,
		.ms = 3000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0xFFFF,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 4,
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
		.v = 1,
		.ms = 3000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 250,
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
		.ms = 3000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F5,
			.duration_ms = 250,
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
			.frequency_hz = NOTE_C5,
			.duration_ms = 250,
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
			.frequency_hz = NOTE_C4,
			.duration_ms = 250,
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
		.ms = 3000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F4,
			.duration_ms = 250,
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
		.v = 0,
		.ms = 3500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x1234,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 4,
		.ms = 3500,
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
		.ms = 3500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
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
		.v = 4,
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
		.v = 5,
		.ms = 3750,
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
		.v = 4,
		.ms = 4000,
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
		.v = 0,
		.ms = 4000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0xFFFF,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 4000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
			.duration_ms = 250,
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
		.ms = 4014,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 30,
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
		.ms = 4014,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 30,
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
		.ms = 4139,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
			.duration_ms = 30,
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
		.ms = 4139,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As5,
			.duration_ms = 30,
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
		.ms = 4250,
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
		.ms = 4250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As5,
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
		.ms = 4250,
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
		.v = 7,
		.ms = 4264,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F6,
			.duration_ms = 30,
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
		.ms = 4264,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 30,
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
		.ms = 4389,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G6,
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
		.ms = 4389,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
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
		.ms = 4500,
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
		.v = 0,
		.ms = 4500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x1234,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 2,
		.ms = 4500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As5,
			.duration_ms = 115,
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
		.ms = 4625,
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
		.ms = 4750,
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
		.ms = 4750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
			.duration_ms = 250,
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
		.ms = 4750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As5,
			.duration_ms = 250,
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
		.ms = 4750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C5,
			.duration_ms = 250,
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
		.ms = 4750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C4,
			.duration_ms = 250,
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
		.ms = 4750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F4,
			.duration_ms = 250,
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
		.ms = 4875,
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
		.v = 4,
		.ms = 5000,
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
		.v = 0,
		.ms = 5000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0xFFFF,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 4,
		.ms = 5250,
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
		.ms = 5250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G6,
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
		.ms = 5250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
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
		.ms = 5500,
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
		.v = 0,
		.ms = 5500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x1234,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 4,
		.ms = 5750,
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
		.ms = 5750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
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
		.ms = 5750,
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
		.v = 0,
		.ms = 6000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0xFFFF,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 4,
		.ms = 6000,
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
		.v = 4,
		.ms = 6250,
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
		.v = 0,
		.ms = 6500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x1234,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 4,
		.ms = 6500,
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
		.v = 4,
		.ms = 6625,
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
		.ms = 6750,
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
		.ms = 6750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 250,
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
		.ms = 6750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 250,
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
		.ms = 6750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds5,
			.duration_ms = 250,
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
		.ms = 6750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds4,
			.duration_ms = 250,
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
		.ms = 6750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G4,
			.duration_ms = 250,
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
		.ms = 6875,
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
		.v = 0,
		.ms = 7000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds3,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0xFFFF,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 4,
		.ms = 7000,
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
		.v = 4,
		.ms = 7250,
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
		.ms = 7250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
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
		.ms = 7250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As5,
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
		.ms = 7250,
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
		.v = 0,
		.ms = 7500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 125,
			.decay = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x1234,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 4,
		.ms = 7500,
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
		.ms = 7500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 30,
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
		.ms = 7500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 30,
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
		.ms = 7500,
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
		.ms = 7625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
			.duration_ms = 30,
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
		.ms = 7625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As5,
			.duration_ms = 30,
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
		.ms = 7750,
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
		.ms = 7750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F6,
			.duration_ms = 30,
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
		.ms = 7750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 30,
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
		.ms = 7750,
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
		.ms = 7875,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G6,
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
		.ms = 7875,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
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
		.ms = 7875,
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
		.ms = 8000,
		.spec = {
			.type = AUDIO_OUT_TYPE_NONE,
		}
	}
};

static const struct audio_out_section BUTTON_MASHER = {
	.length = ARRAY_SIZE(BUTTON_MASHER_NOTES),
	.notes = BUTTON_MASHER_NOTES,
	.next = NULL,
};
#endif
