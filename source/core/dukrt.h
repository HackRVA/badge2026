#ifndef DUKRT_H
#define DUKRT_H

#include "duktape.h"

duk_context *dukrt_create(void);
void dukrt_destroy(duk_context *ctx);
void dukrt_load_script(duk_context *ctx, const char *js_code);
void dukrt_load_script_bytes(
	duk_context *ctx, const unsigned char *js_code, unsigned int length);
void dukrt_update(duk_context *ctx);

#endif // DUKRT_H
