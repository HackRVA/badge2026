
#include "music.h"
#include "audio.h"
#include "utils.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

static const struct audio_out_note DR_BAD_GUY_NOTES[] = {
	{
		.v = 0,
		.ms = 0,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
			.duration_ms = 30,
			.envelope = 0,
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
		.ms = 0,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
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
		.v = 1,
		.ms = 375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
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
		.ms = 500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs6,
			.duration_ms = 30,
			.envelope = 0,
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
			.frequency_hz = NOTE_Fs3,
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
		.ms = 625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
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
		.ms = 750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
			.duration_ms = 30,
			.envelope = 0,
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
		.ms = 750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_A3,
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
		.ms = 875,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x1234,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 0,
		.ms = 1000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
			.duration_ms = 30,
			.envelope = 0,
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
		.ms = 1000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_E3,
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
		.v = 4,
		.ms = 1000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs5,
			.duration_ms = 1000,
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
			.frequency_hz = NOTE_E3,
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
			.frequency_hz = NOTE_E3,
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
		.ms = 1500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs6,
			.duration_ms = 30,
			.envelope = 0,
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
			.frequency_hz = NOTE_E3,
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
		.ms = 1750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_A3,
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
		.ms = 2000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
			.duration_ms = 30,
			.envelope = 0,
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
		.ms = 2000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
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
		.v = 4,
		.ms = 2000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs5,
			.duration_ms = 1000,
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
			.frequency_hz = NOTE_Fs3,
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
		.ms = 2500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs6,
			.duration_ms = 30,
			.envelope = 0,
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
		.ms = 2500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
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
			.frequency_hz = NOTE_Fs5,
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
		.ms = 2500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs5,
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
		.ms = 2625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
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
		.ms = 2750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
			.duration_ms = 30,
			.envelope = 0,
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
		.ms = 2750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_A3,
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
		.ms = 2875,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x1234,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 0,
		.ms = 3000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
			.duration_ms = 30,
			.envelope = 0,
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
		.ms = 3000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_E3,
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
		.v = 4,
		.ms = 3000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_A5,
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
		.v = 1,
		.ms = 3125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_E3,
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
			.frequency_hz = NOTE_E3,
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
		.ms = 3500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs6,
			.duration_ms = 30,
			.envelope = 0,
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
		.ms = 3500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_E3,
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
		.v = 4,
		.ms = 3500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_A5,
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
		.v = 2,
		.ms = 3500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs5,
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
		.ms = 3500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs5,
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
		.ms = 3750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_A3,
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
		.ms = 4000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
			.duration_ms = 30,
			.envelope = 0,
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
		.ms = 4000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_B3,
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
		.v = 4,
		.ms = 4000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_B5,
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
		.v = 4,
		.ms = 4250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs6,
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
		.v = 1,
		.ms = 4375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_B3,
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
		.ms = 4500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs6,
			.duration_ms = 30,
			.envelope = 0,
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
		.ms = 4500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_B3,
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
			.frequency_hz = NOTE_Fs5,
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
		.ms = 4500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs5,
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
		.ms = 4625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_B3,
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
		.ms = 4750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
			.duration_ms = 30,
			.envelope = 0,
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
		.ms = 4750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs4,
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
		.v = 4,
		.ms = 4750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_B5,
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
		.ms = 4875,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
			.duration_ms = 30,
			.envelope = 0,
			.phase = 0,
			.amplitude_dBFS = -6,
			.restart = true,
			.type = AUDIO_OUT_TYPE_NES_NOISE,
			.nes_noise.lfsr_val = 0x1234,
			.nes_noise.mode_flag = true,
		}
	},
	{
		.v = 0,
		.ms = 5000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
			.duration_ms = 30,
			.envelope = 0,
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
		.ms = 5000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_A3,
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
		.v = 4,
		.ms = 5000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_A5,
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
		.v = 1,
		.ms = 5125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_A3,
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
		.ms = 5375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_A3,
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
		.ms = 5500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs6,
			.duration_ms = 30,
			.envelope = 0,
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
		.ms = 5500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_A3,
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
			.frequency_hz = NOTE_Fs5,
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
		.ms = 5500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs6,
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
		.v = 4,
		.ms = 5500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs5,
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
			.frequency_hz = NOTE_Fs5,
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
		.ms = 5625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs6,
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
		.v = 4,
		.ms = 5625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs5,
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
			.frequency_hz = NOTE_B3,
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
		.v = 2,
		.ms = 5750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs5,
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
		.ms = 5750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs6,
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
		.v = 4,
		.ms = 5750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs5,
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
			.frequency_hz = NOTE_Fs3,
			.duration_ms = 30,
			.envelope = 0,
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
		.ms = 6000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
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
		.v = 2,
		.ms = 6000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_E5,
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
		.ms = 6000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_B5,
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
		.v = 4,
		.ms = 6000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_B4,
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
			.frequency_hz = NOTE_Fs5,
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
		.ms = 6250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs6,
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
		.v = 4,
		.ms = 6250,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs5,
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
		.ms = 6375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
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
			.frequency_hz = NOTE_Fs5,
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
		.ms = 6375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs6,
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
		.v = 4,
		.ms = 6375,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs5,
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
		.ms = 6500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs6,
			.duration_ms = 30,
			.envelope = 0,
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
		.ms = 6500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
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
			.frequency_hz = NOTE_Fs5,
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
		.ms = 6500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs6,
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
		.v = 4,
		.ms = 6500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs5,
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
		.ms = 6625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
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
		.ms = 6625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs5,
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
		.ms = 6625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs6,
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
		.v = 4,
		.ms = 6625,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs5,
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
		.ms = 6750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
			.duration_ms = 30,
			.envelope = 0,
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
		.ms = 6750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_A3,
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
		.ms = 6875,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs3,
			.duration_ms = 30,
			.envelope = 0,
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
		.ms = 6875,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs5,
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
		.ms = 6875,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs6,
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
		.v = 4,
		.ms = 6875,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs5,
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
			.frequency_hz = NOTE_Fs3,
			.duration_ms = 30,
			.envelope = 0,
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
		.ms = 7000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_E3,
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
		.v = 4,
		.ms = 7000,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs5,
			.duration_ms = 1000,
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
			.frequency_hz = NOTE_E3,
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
		.ms = 7125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs5,
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
		.ms = 7125,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Cs6,
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
			.frequency_hz = NOTE_E3,
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
		.ms = 7500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs6,
			.duration_ms = 30,
			.envelope = 0,
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
		.ms = 7500,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_E3,
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
			.frequency_hz = NOTE_Fs5,
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
			.frequency_hz = NOTE_Cs6,
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
			.frequency_hz = NOTE_A3,
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
		.v = 2,
		.ms = 7750,
		.spec = {
			.callback = NULL,
			.frequency_hz = NOTE_Fs5,
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
			.frequency_hz = NOTE_Cs6,
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

const struct audio_out_section DR_BAD_GUY = {
	.length = ARRAY_SIZE(DR_BAD_GUY_NOTES),
	.notes = DR_BAD_GUY_NOTES,
	.next = NULL,
};
