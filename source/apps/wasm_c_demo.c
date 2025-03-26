#include <assert.h>

#include "menu.h"
#include "wasmrt.h"
#include "xorshift.h"

#include "cdemo.wasm.h"

static struct wasmrt wasm;

static enum cwasmdemo_state_t {
	CWASMDEMO_INIT = 0,
	CWASMDEMO_RUN,
	CWASMDEMO_EXIT,
} cwasmdemo_state = CWASMDEMO_INIT;

static void close_cb(void)
{
	/* we need to stop running wasmrt so that we can clean it up and exit */
	cwasmdemo_state = CWASMDEMO_EXIT;
}

void cwasmdemo_cb(__attribute__((unused)) struct menu_t *m)
{
	switch (cwasmdemo_state) {
	case CWASMDEMO_INIT:
		cwasmdemo_state = CWASMDEMO_RUN;
		wasm = wasmrt_create();
		wasmrt_load_app(&wasm, out_cdemo_wasm, out_cdemo_wasm_len, &close_cb);
		break;
	case CWASMDEMO_RUN:
		wasmrt_update(&wasm);
		break;
	case CWASMDEMO_EXIT:
    /* return to init state to reinit the wasmrt next time we load the app */
		cwasmdemo_state = CWASMDEMO_INIT;
		wasmrt_cleanup(&wasm);
		pop_app();
	default:
		break;
	}
}
