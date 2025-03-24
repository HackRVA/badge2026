#include "badge.h"
#include "button.h"
#include "dukrt.h"
#include "framebuffer.h"
#include "menu.h"

static duk_context *ctx;

static enum js_demo_state_t {
	JS_DEMO_INIT = 0,
	JS_DEMO_RUN,
	JS_DEMO_EXIT,
} js_demo_state = JS_DEMO_INIT;

static void js_demo_init(void)
{
	js_demo_state = JS_DEMO_RUN;
	ctx = dukrt_create(); dukrt_load_script(ctx,
		"var x = 0;"
		"var progress = 0;"
		"Fb.clear();"
		"var col = Palette.colorFromIndex(3);"
		"Fb.color(col);"
		"Fb.move(x, 20);"
		"Fb.point(x, 20);"
		"Fb.move(40, 40);"
		"Fb.writeString('hello, world!');"
		"function update() {"
		"  Fb.clear();"
		"  Palette.drawGrid(1, 1, 8);"
		"  Fb.color(Palette.colorFromIndex(3));"
		"  Fb.move(80, 80);"
		"  Fb.filledRectangle(50, 50);"
		"  Fb.color(Palette.colorFromIndex(8));"
		"  Fb.move(x, 20);"
		"  Fb.point(x, 20);"
		"  Fb.move(20, 80);"
		"  Fb.writeString('hello, world!');"
		"  x = (x + 1) % 240;"
		"  var button = {"
		"   x: 10,"
		"   y: 10,"
		"   width: 80,"
		"   height: 30,"
		"   text: 'JS Demo',"
		"   outlineSize: 2,"
		"   outlineColor: Palette.colorFromIndex(1),"
		"   fillColor: Palette.colorFromIndex(2),"
		"   textColor: Palette.colorFromIndex(3)"
		"  };"
		"  UI.buttonDraw(button);"
		"  var progressBar = {"
		"   x: 10,"
		"   y: 50,"
		"   width: 100,"
		"   height: 10,"
		"   outlineSize: 2,"
		"   fillColor: Palette.colorFromIndex(4),"
		"   emptyColor: Palette.colorFromIndex(0),"
		"   outlineColor: Palette.colorFromIndex(1),"
		"   fillPercentage: progress"
		"  };"
		/*"  if (Input.isButtonPressed('A')) {"*/
		/*"     progressBar.fill_color = Palette.colorFromIndex(5);"*/
		/*"     print('hello', Input.getState()); "*/
		/*"  }"*/
		/*"  } else if (Input.isButtonPressed('B')) {"*/
		"  if (Input.isButtonPressed('B')) {"
		"     progressBar.fill_color = Palette.colorFromIndex(9);"
		"     App.pop();"
		"  }"
		"  UI.progressBarDraw(progressBar);"
		"  progress += 0.01;"
		"  if (progress >= 1.0) progress = 0;"
		"}");
}

static void check_buttons(void)
{
#if 0
	int down_latches = button_down_latches();

	if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
		js_demo_state = JS_DEMO_EXIT;
	}
#endif
}

void js_demo_cb(__attribute__((unused)) struct badge_app *a)
{
	switch (js_demo_state) {
	case JS_DEMO_INIT:
		js_demo_init();
		break;
	case JS_DEMO_RUN:
		dukrt_update(ctx);
		FbSwapBuffers();
		break;
	case JS_DEMO_EXIT:
		pop_app();
    /* TODO: cleanup doesn't currently run because the app is being exited from inside the js code */
		js_demo_state = JS_DEMO_INIT;
		dukrt_destroy(ctx);
		break;
	default:
		break;
	}
	check_buttons();
}

