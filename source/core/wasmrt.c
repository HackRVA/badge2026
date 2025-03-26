#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wasm3.h>

#include "button.h"
#include "colors.h"
#include "framebuffer.h"
#include "palette.h"
#include "wasmrt.h"
#include "xorshift.h"

static struct wasmrt *active_rt = NULL;

/*
 * maybe reevaluate how to do this.
 * i.e. what happens when the world fails...
 * some recovery is likely needed...
 */
#ifdef TARGET_SIMULATOR
#include <unistd.h>
#define FATAL(func, msg)                                                       \
	{                                                                      \
		printf("Fatal %s: %s\n", func, msg);                           \
		while (1) {                                                    \
			usleep(100000);                                        \
		}                                                              \
	}
#else
#define FATAL(func, msg)                                                       \
	{                                                                      \
		printf("Fatal %s: %s\n", func, msg);                           \
		while (1) {                                                    \
			sleep_ms(100);                                         \
		}                                                              \
	}
#endif

static struct palette default_palette = {
	.colors =
		{
			PACKRGB888(0, 0, 0),
			PACKRGB888(127, 36, 84),
			PACKRGB888(28, 43, 83),
			PACKRGB888(0, 135, 81),
			PACKRGB888(171, 82, 54),
			PACKRGB888(96, 88, 79),
			PACKRGB888(195, 195, 198),
			PACKRGB888(255, 241, 233),
			PACKRGB888(237, 27, 81),
			PACKRGB888(250, 162, 27),
			PACKRGB888(247, 236, 47),
			PACKRGB888(93, 187, 77),
			PACKRGB888(81, 166, 220),
			PACKRGB888(131, 118, 156),
			PACKRGB888(241, 118, 166),
			PACKRGB888(252, 204, 171),
		},
};

m3ApiRawFunction(Math_random)
{
	(void)runtime;
	(void)_sp;
	(void)_ctx;
	(void)_mem;
	m3ApiReturnType(unsigned int) m3ApiGetArg(unsigned int, num);
	unsigned int r = xorshift(&num);

	m3ApiReturn(r);
}

/* Memcpy is generic, and much faster in native code*/
/*m3ApiRawFunction(Dino_memcpy) {*/
/*  m3ApiGetArgMem(uint8_t *, dst) m3ApiGetArgMem(uint8_t *, src)*/
/*      m3ApiGetArgMem(uint8_t *, dstend)*/
/**/
/*          do {*/
/*    *dst++ = *src++;*/
/*  }*/
/*  while (dst < dstend)*/
/*    ;*/
/**/
/*  m3ApiSuccess();*/
/*}*/

/*void abort(int message, int fileName, int lineNumber, int columnNumber) {*/
/*  printf("Abort called! Message: %d, File: %d, Line: %d, Column: %d\n",
 * message,*/
/*         fileName, lineNumber, columnNumber);*/
/*}*/

m3ApiRawFunction(fb_clear_wasm)
{
	(void)runtime;
	(void)_sp;
	(void)_ctx;
	(void)_mem;
	FbClear();
	m3ApiSuccess();
}

m3ApiRawFunction(fb_color_wasm)
{
	(void)runtime;
	(void)_sp;
	(void)_ctx;
	(void)_mem;
	m3ApiGetArg(uint16_t, color);
	FbColor(color);
	m3ApiSuccess();
}

m3ApiRawFunction(fb_move_wasm)
{
	(void)runtime;
	(void)_sp;
	(void)_ctx;
	(void)_mem;
	m3ApiGetArg(uint8_t, x);
	m3ApiGetArg(uint8_t, y);
	FbMove(x, y);
	m3ApiSuccess();
}

m3ApiRawFunction(fb_point_wasm)
{
	(void)runtime;
	(void)_sp;
	(void)_ctx;
	(void)_mem;
	m3ApiGetArg(uint8_t, x);
	m3ApiGetArg(uint8_t, y);
	FbPoint(x, y);
	m3ApiSuccess();
}

m3ApiRawFunction(fb_horizontal_line_wasm)
{
	(void)runtime;
	(void)_sp;
	(void)_ctx;
	(void)_mem;
	m3ApiGetArg(uint8_t, x1);
	m3ApiGetArg(uint8_t, y1);
	m3ApiGetArg(uint8_t, x2);
	m3ApiGetArg(uint8_t, y2);
	FbHorizontalLine(x1, y1, x2, y2);
	m3ApiSuccess();
}

m3ApiRawFunction(fb_vertical_line_wasm)
{
	(void)runtime;
	(void)_sp;
	(void)_ctx;
	(void)_mem;
	m3ApiGetArg(uint8_t, x1);
	m3ApiGetArg(uint8_t, y1);
	m3ApiGetArg(uint8_t, x2);
	m3ApiGetArg(uint8_t, y2);
	FbVerticalLine(x1, y1, x2, y2);
	m3ApiSuccess();
}

m3ApiRawFunction(fb_line_wasm)
{
	(void)runtime;
	(void)_sp;
	(void)_ctx;
	(void)_mem;
	m3ApiGetArg(uint8_t, x0);
	m3ApiGetArg(uint8_t, y0);
	m3ApiGetArg(uint8_t, x1);
	m3ApiGetArg(uint8_t, y1);
	FbLine(x0, y0, x1, y1);
	m3ApiSuccess();
}

m3ApiRawFunction(fb_rectangle_wasm)
{
	(void)runtime;
	(void)_sp;
	(void)_ctx;
	(void)_mem;
	m3ApiGetArg(uint8_t, width);
	m3ApiGetArg(uint8_t, height);
	FbRectangle(width, height);
	m3ApiSuccess();
}

m3ApiRawFunction(fb_circle_wasm)
{
	(void)runtime;
	(void)_sp;
	(void)_ctx;
	(void)_mem;
	m3ApiGetArg(int, x);
	m3ApiGetArg(int, y);
	m3ApiGetArg(int, r);
	FbCircle(x, y, r);
	m3ApiSuccess();
}

m3ApiRawFunction(fb_filled_circle_wasm)
{
	(void)runtime;
	(void)_sp;
	(void)_ctx;
	(void)_mem;
	m3ApiGetArg(int, x);
	m3ApiGetArg(int, y);
	m3ApiGetArg(int, r);
	FbFilledCircle(x, y, r);
	m3ApiSuccess();
}

m3ApiRawFunction(fb_write_string_wasm)
{
	(void)runtime;
	(void)_sp;
	(void)_ctx;
	(void)_mem;
	m3ApiGetArgMem(const char *, string);
	FbWriteString(string);
	m3ApiSuccess();
}

m3ApiRawFunction(fb_filled_rectangle_wasm)
{
	(void)runtime;
	(void)_sp;
	(void)_ctx;
	(void)_mem;
	m3ApiGetArg(uint8_t, width);
	m3ApiGetArg(uint8_t, height);
	FbFilledRectangle(width, height);
	m3ApiSuccess();
}

m3ApiRawFunction(fb_swap_buffers_wasm)
{
	(void)runtime;
	(void)_sp;
	(void)_ctx;
	(void)_mem;
	FbSwapBuffers();
	m3ApiSuccess();
}

m3ApiRawFunction(palette_draw_grid_wasm)
{
	(void)runtime;
	(void)_sp;
	(void)_ctx;
	(void)_mem;
	m3ApiGetArg(uint8_t, x);
	m3ApiGetArg(uint8_t, y);
	m3ApiGetArg(uint8_t, s);
	palette_draw_grid(default_palette, x, y, s);
	m3ApiSuccess();
}

m3ApiRawFunction(palette_get_color_from_index_wasm)
{
	(void)runtime;
	(void)_sp;
	(void)_ctx;
	(void)_mem;
	m3ApiReturnType(uint16_t);
	m3ApiGetArg(uint8_t, index);
	uint16_t color = palette_color_from_index(default_palette, index);
	m3ApiReturn(color);
}

m3ApiRawFunction(palette_get_color_wasm)
{
	(void)runtime;
	(void)_sp;
	(void)_ctx;
	(void)_mem;
	m3ApiReturnType(uint16_t);
	m3ApiGetArg(uint8_t, index);
	uint16_t color = palette_get_color(default_palette, index);
	m3ApiReturn(color);
}

m3ApiRawFunction(fb_rounded_rect_wasm)
{
	(void)runtime;
	(void)_sp;
	(void)_ctx;
	(void)_mem;
	m3ApiGetArg(uint8_t, x);
	m3ApiGetArg(uint8_t, y);
	m3ApiGetArg(uint8_t, s);
	FbRoundedRect(x, y, s);
	m3ApiSuccess();
}

#define MAX_DIGITS_U8 3
m3ApiRawFunction(u8ToString_wasm)
{
	(void)runtime;
	(void)_sp;
	(void)_ctx;
	(void)_mem;
	m3ApiReturnType(const char *) m3ApiGetArg(uint8_t, value);

	char *str = (char *)malloc(MAX_DIGITS_U8 + 1);
	if (str == NULL) {
		m3ApiReturn(NULL);
	}

	snprintf(str, MAX_DIGITS_U8 + 1, "%u", value);

	m3ApiReturn(str);
}

#define BUTTON_STATE_ADDR 0x0000

static void button_callback(BADGE_BUTTON button, bool state)
{
	if (!active_rt || !active_rt->mem)
		return;

	uint32_t *input = (uint32_t *)(active_rt->mem + BUTTON_STATE_ADDR);
	if (state) {
		switch (button) {
		case BADGE_BUTTON_UP:
			printf("UP button pressed\n");
			*input |= 0x1;
			break;
		case BADGE_BUTTON_DOWN:
			printf("DOWN button pressed\n");
			*input |= 0x2;
			break;
		case BADGE_BUTTON_LEFT:
			printf("LEFT button pressed\n");
			*input |= 0x4;
			break;
		case BADGE_BUTTON_RIGHT:
			printf("RIGHT button pressed\n");
			*input |= 0x8;
			break;
		case BADGE_BUTTON_A:
			printf("A button pressed\n");
			*input |= 0x10;
			break;
		case BADGE_BUTTON_B:
			printf("B button pressed\n");
			*input |= 0x20;
			break;
		default:
			printf("Unknown button pressed\n");
			break;
		}
	} else {
		switch (button) {
		case BADGE_BUTTON_UP:
			*input &= ~0x1;
			break;
		case BADGE_BUTTON_DOWN:
			*input &= ~0x2;
			break;
		case BADGE_BUTTON_LEFT:
			*input &= ~0x4;
			break;
		case BADGE_BUTTON_RIGHT:
			*input &= ~0x8;
			break;
		case BADGE_BUTTON_A:
			*input &= ~0x10;
			break;
		case BADGE_BUTTON_B:
			*input &= ~0x20;
			break;
		default:
			break;
		}
	}
}

static void load_wasm(
	struct wasmrt *rt, unsigned char *wasm_app, int wasm_app_size)
{
	M3Result result = m3Err_none;

	if (rt->runtime)
		m3_FreeRuntime(rt->runtime);

	rt->runtime = m3_NewRuntime(rt->env, 8192, NULL);
	if (!rt->runtime)
		FATAL("NewRuntime", "failed");

	result = m3_ParseModule(rt->env, &rt->module, wasm_app, wasm_app_size);
	if (result)
		FATAL("ParseModule", result);

	result = m3_LoadModule(rt->runtime, rt->module);
	if (result)
		FATAL("LoadModule", result);

	m3_LinkRawFunction(rt->module, "Math", "random", "f()", &Math_random);

	m3_LinkRawFunction(rt->module, "Fb", "clear", "v()", &fb_clear_wasm);
	m3_LinkRawFunction(rt->module, "Fb", "color", "v(i)", &fb_color_wasm);
	m3_LinkRawFunction(rt->module, "Fb", "move", "v(ii)", &fb_move_wasm);
	m3_LinkRawFunction(
		rt->module, "Fb", "rectangle", "v(ii)", &fb_rectangle_wasm);
	m3_LinkRawFunction(
		rt->module, "Fb", "circle", "v(iii)", &fb_circle_wasm);
	m3_LinkRawFunction(rt->module, "Fb", "filledCircle", "v(iii)",
		&fb_filled_circle_wasm);
	m3_LinkRawFunction(rt->module, "Fb", "filledRectangle", "v(ii)",
		&fb_filled_rectangle_wasm);

	m3_LinkRawFunction(rt->module, "Fb", "roundedRectangle", "v(iii)",
		&fb_rounded_rect_wasm);
	m3_LinkRawFunction(
		rt->module, "Fb", "swapBuffers", "v(v)", &fb_swap_buffers_wasm);

	/*m3_LinkRawFunction(*/
	/*	rt->module, "Fb", "writeString", "v(*)", &fb_write_string_wasm);*/

	m3_LinkRawFunction(rt->module, "Palette", "drawGrid", "v(iii)",
		&palette_draw_grid_wasm);
	m3_LinkRawFunction(rt->module, "Palette", "getColor", "i(i)",
		&palette_get_color_wasm);
	m3_LinkRawFunction(rt->module, "Palette", "getColorFromIndex", "i(i)",
		&palette_get_color_from_index_wasm);

	m3_LinkRawFunction(
		rt->module, "Utils", "u8ToString", "i(i)", &u8ToString_wasm);

	rt->mem = m3_GetMemory(rt->runtime, NULL, 0);
	if (!rt->mem)
		FATAL("GetMemory", "failed");

#ifdef BUILD_SIMULATOR
	static uint8_t dummy_memory[64 * 1024];
	if (!rt->mem) {
		printf("WARNING: Using dummy memory\n");
		rt->mem = dummy_memory;
	}
#endif

	m3_FindFunction(&rt->func_update, rt->runtime, "update");
	m3_FindFunction(&rt->func_render, rt->runtime, "render");

	active_rt = rt;
}

static void wasmrt_init(void)
{
	button_set_interrupt(button_callback);

	printf("\nWasm3 v" M3_VERSION " (" M3_ARCH "), build " __DATE__
	       " " __TIME__ "\n");
}

struct wasmrt wasmrt_create(void)
{
	IM3Environment env = m3_NewEnvironment();
	if (!env)
		FATAL("NewEnvironment", "failed");

	return (struct wasmrt){
		.env = env,
		.runtime = NULL,
		.module = NULL,
		.func_run = NULL,
		.func_update = NULL,
		.func_render = NULL,
		.func_init = NULL,
		.func_checkButtons = NULL,
		.mem = NULL,
	};
}

void wasmrt_cleanup(struct wasmrt *rt)
{
	if (rt->runtime) {
		m3_FreeRuntime(rt->runtime);
		rt->runtime = NULL;
	}
	if (rt->env) {
		m3_FreeEnvironment(rt->env);
		rt->env = NULL;
	}
	if (active_rt == rt)
		active_rt = NULL;

	rt->module = NULL;
	rt->func_run = NULL;
	rt->func_update = NULL;
	rt->func_render = NULL;
	rt->func_init = NULL;
	rt->func_checkButtons = NULL;
	rt->mem = NULL;

	printf("Wasm runtime cleaned up.\n");
}

void wasmrt_load_app(
	struct wasmrt *rt, unsigned char *wasm_app, int wasm_app_size)
{
	wasmrt_init();
	load_wasm(rt, wasm_app, wasm_app_size);
}

void wasmrt_update(struct wasmrt *rt)
{
	M3Result result;

	if (rt->func_update) {
		result = m3_CallV(rt->func_update);
		if (result) {
			M3ErrorInfo info;
			m3_GetErrorInfo(rt->runtime, &info);
			printf("Error in update: %s (%s)\n", result,
				info.message);
		}
	}

	if (rt->func_render) {
		result = m3_CallV(rt->func_render);
		if (result) {
			M3ErrorInfo info;
			m3_GetErrorInfo(rt->runtime, &info);
			printf("Error in render: %s (%s)\n", result,
				info.message);
		}
	}
}

