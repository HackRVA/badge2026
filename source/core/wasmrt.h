#ifndef WASMRT_H
#define WASMRT_H

#include <stdint.h>
#include <wasm3.h>

struct wasmrt {
	IM3Environment env;
	IM3Runtime runtime;
	IM3Module module;
	IM3Function func_update;
	IM3Function func_draw;
	uint32_t button_mask;
  void (*close_cb)(void);
};

struct wasmrt wasmrt_create(void);
void wasmrt_load_app(
	struct wasmrt *rt, unsigned char *wasm_app, int wasm_app_size, void (*close_fn)(void));
void wasmrt_update(struct wasmrt *rt);
void wasmrt_cleanup(struct wasmrt *rt);

#endif
