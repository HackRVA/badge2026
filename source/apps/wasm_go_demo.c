#include <assert.h>

#include "menu.h"
#include "wasmrt.h"
#include "xorshift.h"

#include "go_demo.wasm.h"

static struct wasmrt wasm;

static enum gowasmdemo_state_t {
	GOWASMDEMO_INIT = 0,
	GOWASMDEMO_RUN,
	GOWASMDEMO_EXIT,
} gowasmdemo_state = GOWASMDEMO_INIT;

static void close_cb(void)
{
	/* we need to stop running wasmrt so that we can clean it up and exit */
  gowasmdemo_state = GOWASMDEMO_EXIT;
}

void gowasmdemo_cb(__attribute__((unused)) struct menu_t *m)
{
	switch (gowasmdemo_state) {
	case GOWASMDEMO_INIT:
		gowasmdemo_state = GOWASMDEMO_RUN;
		wasm = wasmrt_create();
		wasmrt_load_app(
			&wasm, out_go_demo_wasm, out_go_demo_wasm_len, &close_cb);
		break;
	case GOWASMDEMO_RUN:
		wasmrt_update(&wasm);
		break;
	case GOWASMDEMO_EXIT:
    /* return to init state to reinit the wasmrt next time we load the app */
		gowasmdemo_state = GOWASMDEMO_INIT;
		wasmrt_cleanup(&wasm);
		pop_app();
		break;
	default:
		break;
	}
}
