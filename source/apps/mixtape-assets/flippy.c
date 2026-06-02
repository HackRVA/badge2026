
#include "music.h"
#include "audio.h"
#include "utils.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

static const struct audio_out_note FLIPPY_NOTES[] = {
	{
		.v = 0,
		.ms = 0,
		.spec = {
			.callback = NULL,
			.frequency_hz = 65,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = 0,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x4001,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 0,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 124,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 0,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C5,
			.duration_ms = 125,
			.envelope = 0,
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
			.frequency_hz = NOTE_F3,
			.duration_ms = 124,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds5,
			.duration_ms = 125,
			.envelope = 0,
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
			.frequency_hz = 97,
			.duration_ms = 124,
			.envelope = 0,
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
			.frequency_hz = NOTE_F5,
			.duration_ms = 125,
			.envelope = 0,
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
			.frequency_hz = NOTE_C3,
			.duration_ms = 124,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 125,
			.envelope = 0,
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
			.frequency_hz = NOTE_C4,
			.duration_ms = 124,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 0,
		.ms = 502,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 129,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = 0,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x0101,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F4,
			.duration_ms = 124,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G3,
			.duration_ms = 124,
			.envelope = 0,
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
			.frequency_hz = NOTE_Ds5,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 875,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C4,
			.duration_ms = 124,
			.envelope = 0,
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
			.frequency_hz = 65,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = 0,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x4001,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 1000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 124,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 1125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F3,
			.duration_ms = 124,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 1250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds3,
			.duration_ms = 124,
			.envelope = 0,
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
			.frequency_hz = NOTE_As4,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 1375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 124,
			.envelope = 0,
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
			.frequency_hz = NOTE_C3,
			.duration_ms = 124,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 1500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C5,
			.duration_ms = 30,
			.envelope = 0,
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
			.frequency_hz = NOTE_F5,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 0,
		.ms = 1502,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 129,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = 0,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x0101,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 2,
		.ms = 1535,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds5,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 1535,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 1579,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F5,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 1579,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As5,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 1625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F3,
			.duration_ms = 124,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 1625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds5,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 1625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 1660,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F5,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 1660,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As5,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 1704,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 1704,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 30,
			.envelope = 0,
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
			.frequency_hz = NOTE_G3,
			.duration_ms = 124,
			.envelope = 0,
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
			.frequency_hz = NOTE_F5,
			.duration_ms = 30,
			.envelope = 0,
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
			.frequency_hz = NOTE_As5,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 1785,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 1785,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 1829,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As5,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 1829,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 1875,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C4,
			.duration_ms = 124,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 1875,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 1875,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 1910,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As5,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 1910,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 1954,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 1954,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F6,
			.duration_ms = 30,
			.envelope = 0,
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
			.frequency_hz = 65,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = 0,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x4001,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 2000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 124,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 2000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C5,
			.duration_ms = 57,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 2125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F3,
			.duration_ms = 124,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 2125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds5,
			.duration_ms = 57,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 2250,
		.spec = {
			.callback = NULL,
			.frequency_hz = 97,
			.duration_ms = 124,
			.envelope = 0,
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
			.frequency_hz = NOTE_F5,
			.duration_ms = 57,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 2375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 124,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 2375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 57,
			.envelope = 0,
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
			.frequency_hz = NOTE_C4,
			.duration_ms = 124,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 0,
		.ms = 2502,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 129,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = 0,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x0101,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 2625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F4,
			.duration_ms = 124,
			.envelope = 0,
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
			.frequency_hz = NOTE_F5,
			.duration_ms = 125,
			.envelope = 0,
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
			.frequency_hz = NOTE_G3,
			.duration_ms = 124,
			.envelope = 0,
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
			.frequency_hz = NOTE_C4,
			.duration_ms = 124,
			.envelope = 0,
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
			.frequency_hz = 65,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = 0,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x4001,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 3000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 124,
			.envelope = 0,
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
			.frequency_hz = NOTE_As4,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 3125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F3,
			.duration_ms = 124,
			.envelope = 0,
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
			.frequency_hz = NOTE_Ds3,
			.duration_ms = 124,
			.envelope = 0,
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
			.frequency_hz = NOTE_As4,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 3375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 124,
			.envelope = 0,
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
			.frequency_hz = NOTE_C3,
			.duration_ms = 124,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 3500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C5,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 0,
		.ms = 3502,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 129,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = 0,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x0101,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 3625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F3,
			.duration_ms = 124,
			.envelope = 0,
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
			.frequency_hz = NOTE_G3,
			.duration_ms = 124,
			.envelope = 0,
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
			.frequency_hz = 65,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = 0,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x4001,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 4000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 4000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G4,
			.duration_ms = 64,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 4125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F3,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 4125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As4,
			.duration_ms = 64,
			.envelope = 0,
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
			.frequency_hz = 97,
			.duration_ms = 125,
			.envelope = 0,
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
			.frequency_hz = NOTE_C5,
			.duration_ms = 64,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 4375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 4375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds5,
			.duration_ms = 64,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 4500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C4,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 0,
		.ms = 4502,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 129,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = 0,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x0101,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 4625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F4,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 4636,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C5,
			.duration_ms = 64,
			.envelope = 0,
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
			.frequency_hz = NOTE_G3,
			.duration_ms = 125,
			.envelope = 0,
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
			.frequency_hz = NOTE_As4,
			.duration_ms = 64,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 4875,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C4,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 4875,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C5,
			.duration_ms = 64,
			.envelope = 0,
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
			.frequency_hz = 65,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = 0,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x4001,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 5000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 5000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds5,
			.duration_ms = 64,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 5125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F3,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 5125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F5,
			.duration_ms = 64,
			.envelope = 0,
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
			.frequency_hz = NOTE_Ds3,
			.duration_ms = 125,
			.envelope = 0,
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
			.frequency_hz = NOTE_F4,
			.duration_ms = 131,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 5375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 5500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 5500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F5,
			.duration_ms = 64,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 0,
		.ms = 5502,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 129,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = 0,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x0101,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 2,
		.ms = 5570,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 43,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 5625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F3,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 5625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 64,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 5695,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As5,
			.duration_ms = 43,
			.envelope = 0,
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
			.frequency_hz = NOTE_G3,
			.duration_ms = 125,
			.envelope = 0,
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
			.frequency_hz = NOTE_As5,
			.duration_ms = 64,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 5820,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 43,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 5875,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C4,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 5875,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 64,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 5945,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
			.duration_ms = 43,
			.envelope = 0,
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
			.frequency_hz = 65,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = 0,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x4001,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 6000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 6000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As5,
			.duration_ms = 64,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 6070,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 43,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 6125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F3,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 6125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 64,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 6195,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
			.duration_ms = 43,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 6250,
		.spec = {
			.callback = NULL,
			.frequency_hz = 97,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 6250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds6,
			.duration_ms = 64,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 6320,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F6,
			.duration_ms = 43,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 6375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 6375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F6,
			.duration_ms = 64,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 6445,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G6,
			.duration_ms = 43,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 6500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C4,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 6500,
		.spec = {
			.callback = NULL,
			.frequency_hz = 2093,
			.duration_ms = 250,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 6500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G6,
			.duration_ms = 250,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 0,
		.ms = 6502,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 129,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = 0,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x0101,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 6625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F4,
			.duration_ms = 125,
			.envelope = 0,
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
			.frequency_hz = NOTE_G3,
			.duration_ms = 125,
			.envelope = 0,
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
			.frequency_hz = NOTE_F6,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 6875,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C4,
			.duration_ms = 125,
			.envelope = 0,
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
			.frequency_hz = 65,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = 0,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x4001,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 7000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 2,
		.ms = 7000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 375,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 7000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 375,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 7125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F3,
			.duration_ms = 125,
			.envelope = 0,
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
			.frequency_hz = NOTE_Ds3,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 1,
		.ms = 7375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 125,
			.envelope = 0,
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
			.frequency_hz = NOTE_C3,
			.duration_ms = 125,
			.envelope = 0,
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
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 7500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds5,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 0,
		.ms = 7502,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C6,
			.duration_ms = 129,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = 0,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x0101,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 7625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F3,
			.duration_ms = 125,
			.envelope = 0,
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
			.frequency_hz = NOTE_F5,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 7625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C5,
			.duration_ms = 125,
			.envelope = 0,
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
			.frequency_hz = NOTE_G3,
			.duration_ms = 125,
			.envelope = 0,
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
			.frequency_hz = NOTE_C5,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 7750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G4,
			.duration_ms = 125,
			.envelope = 0,
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
			.frequency_hz = NOTE_C4,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 3,
		.ms = 7875,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G3,
			.duration_ms = 125,
			.envelope = 0,
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

const struct audio_out_section FLIPPY = {
	.length = ARRAY_SIZE(FLIPPY_NOTES),
	.notes = FLIPPY_NOTES,
	.next = NULL,
};
