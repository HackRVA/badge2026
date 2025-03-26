#include <assert.h>

#include "menu.h"
#include "wasmrt.h"
#include "xorshift.h"
#include "gol.wasm.h"

static struct wasmrt wasm;

static enum golwasm_state_t {
	GOLWASM_INIT = 0,
	GOLWASM_RUN,
	GOLWASM_EXIT,
} golwasm_state = GOLWASM_INIT;

static void close_cb(void)
{
	/* we need to stop running wasmrt so that we can clean it up and exit */
  golwasm_state = GOLWASM_EXIT;
}

void golwasm_cb(__attribute__((unused)) struct menu_t *m)
{
	switch (golwasm_state) {
	case GOLWASM_INIT:
		golwasm_state = GOLWASM_RUN;
		wasm = wasmrt_create();
		wasmrt_load_app(
			&wasm, out_gol_wasm, out_gol_wasm_len, &close_cb);
		break;
	case GOLWASM_RUN:
		wasmrt_update(&wasm);
		break;
	case GOLWASM_EXIT:
    /* return to init state to reinit the wasmrt next time we load the app */
		golwasm_state = GOLWASM_INIT;
		wasmrt_cleanup(&wasm);
		pop_app();
		break;
	default:
		break;
	}
}
