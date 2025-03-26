#include <assert.h>

#include "menu.h"
#include "wasmrt.h"
#include "xorshift.h"

#include "as_demo.wasm.h"

static struct wasmrt wasm;

static enum aswasmdemo_state_t {
	ASWASMDEMO_INIT = 0,
	ASWASMDEMO_RUN,
	ASWASMDEMO_EXIT,
} aswasmdemo_state = ASWASMDEMO_INIT;

static void close_cb(void)
{
	/* we need to stop running wasmrt so that we can clean it up and exit */
  aswasmdemo_state = ASWASMDEMO_EXIT;
}

void wasmdemo_cb(__attribute__((unused)) struct menu_t *m)
{
	switch (aswasmdemo_state) {
	case ASWASMDEMO_INIT:
		aswasmdemo_state = ASWASMDEMO_RUN;
		wasm = wasmrt_create();
		wasmrt_load_app(&wasm, out_as_demo_wasm, out_as_demo_wasm_len, &close_cb);
		break;
	case ASWASMDEMO_RUN:
		wasmrt_update(&wasm);
		break;
	case ASWASMDEMO_EXIT:
    /* return to init state to reinit the wasmrt next time we load the app */
		aswasmdemo_state = ASWASMDEMO_INIT;
		wasmrt_cleanup(&wasm);
		pop_app();
		break;
	default:
		break;
	}
}
