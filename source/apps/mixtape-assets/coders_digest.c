
#include "music.h"
#include "audio.h"
#include "utils.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

static const struct audio_out_note CODERS_DIGEST_NOTES[] = {
	{
		.v = 0,
		.ms = 0,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -1,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x7FFF,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 0,
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
		.ms = 250,
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
		.ms = 375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As5,
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
		.ms = 500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G5,
			.duration_ms = 500,
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
		.ms = 500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As4,
			.duration_ms = 500,
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
		.ms = 1000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G6,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -1,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x7FFF,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 1000,
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
		.ms = 1000,
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
		.v = 3,
		.ms = 1125,
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
		.v = 1,
		.ms = 1250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As3,
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
		.ms = 1250,
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
		.ms = 1500,
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
		.ms = 1500,
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
		.ms = 1500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C5,
			.duration_ms = 500,
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
		.v = 0,
		.ms = 2000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -1,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x7FFF,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 2000,
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
		.ms = 2000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As5,
			.duration_ms = 500,
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
		.ms = 2000,
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
		.v = 3,
		.ms = 2125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds4,
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
		.ms = 2250,
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
		.ms = 2375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As3,
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
		.ms = 2500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F5,
			.duration_ms = 500,
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
			.frequency_hz = NOTE_G6,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -1,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x7FFF,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 2,
		.ms = 3000,
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
		.ms = 3000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As3,
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
		.v = 5,
		.ms = 3000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds4,
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
		.ms = 3057,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C4,
			.duration_ms = 42,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 5,
		.ms = 3057,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F4,
			.duration_ms = 42,
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
		.ms = 3125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C4,
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
		.v = 5,
		.ms = 3125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F4,
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
		.ms = 3182,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds4,
			.duration_ms = 42,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 5,
		.ms = 3182,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G4,
			.duration_ms = 42,
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
		.v = 3,
		.ms = 3250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds4,
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
		.v = 5,
		.ms = 3250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G4,
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
		.ms = 3307,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F4,
			.duration_ms = 42,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 5,
		.ms = 3307,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As4,
			.duration_ms = 42,
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
		.ms = 3375,
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
		.ms = 3375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_F4,
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
		.v = 5,
		.ms = 3375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As4,
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
		.ms = 3432,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G4,
			.duration_ms = 42,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 5,
		.ms = 3432,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C5,
			.duration_ms = 42,
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
		.v = 2,
		.ms = 4000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C5,
			.duration_ms = 500,
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
		.v = 0,
		.ms = 4000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -1,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x7FFF,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 1,
		.ms = 4250,
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
		.ms = 4500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As4,
			.duration_ms = 500,
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
		.ms = 5000,
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
		.ms = 5000,
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
		.v = 0,
		.ms = 5000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G6,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -1,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x7FFF,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 2,
		.ms = 5250,
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
		.ms = 5250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As3,
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
		.ms = 5375,
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
		.v = 2,
		.ms = 5500,
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
		.ms = 5500,
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
		.ms = 5750,
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
		.ms = 6000,
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
		.v = 0,
		.ms = 6000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_C3,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -1,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x7FFF,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 0,
		.ms = 7000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_G6,
			.duration_ms = 125,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -1,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x7FFF,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 2,
		.ms = 7000,
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
		.ms = 7006,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_As3,
			.duration_ms = 119,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_SQUARE,
			.square.duty_cycle = UINT8_MAX / 2,
		}
	},
	{
		.v = 5,
		.ms = 7006,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds4,
			.duration_ms = 119,
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
		.ms = 7250,
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
		.v = 3,
		.ms = 7250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Ds4,
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
		.v = 5,
		.ms = 7250,
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
		.ms = 7375,
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
		.ms = 7375,
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
		.v = 5,
		.ms = 7375,
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
		.v = 5,
		.ms = 7500,
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
		.ms = 7625,
		.spec = {
			.type = AUDIO_OUT_TYPE_NONE,
		}
	}
};

const struct audio_out_section CODERS_DIGEST = {
	.length = ARRAY_SIZE(CODERS_DIGEST_NOTES),
	.notes = CODERS_DIGEST_NOTES,
	.next = NULL,
};
