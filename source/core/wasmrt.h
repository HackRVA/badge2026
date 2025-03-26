#ifndef WASMRT_H
#define WASMRT_H

#include <wasm3.h>
#include <stdint.h>

struct wasmrt {
	IM3Environment env;
	IM3Runtime runtime;
	IM3Module module;
	IM3Function func_run;
	IM3Function func_update;
	IM3Function func_draw;
	IM3Function func_init;
	IM3Function func_checkButtons;
  uint32_t button_mask;
};

struct wasmrt wasmrt_create(void);
void wasmrt_load_app(struct wasmrt *rt, unsigned char *wasm_app, int wasm_app_size);
void wasmrt_update(struct wasmrt *rt);
void wasmrt_cleanup(struct wasmrt *rt);

#endif
