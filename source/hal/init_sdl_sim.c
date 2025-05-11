//
// Created by Samuel Jones on 2/21/22.
//

#include "init.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <SDL.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#include "framebuffer.h"
#include "display.h"
#include "led_pwm.h"
#include "button.h"
#include "button_coords.h"
#include "button_sdl_ui.h"
#include "ir.h"
#include "rtc.h"
#include "flash_storage.h"
#include "led_pwm_sdl.h"
#include "sim_lcd_params.h"
#include "png_utils.h"
#include "trig.h"
#include "quat.h"
#include "uid.h"
#include "audio.h"
#include "xorshift.h"
#include "sim_slider_input.h"
#include "utils.h"
#include "analog.h"

#define UNUSED __attribute__((unused))
#define ARRAYSIZE(x) (sizeof(x) / sizeof((x)[0]))

static int sim_argc;
static char** sim_argv;
static int fullscreen = 0;
static int fullscreen_flag = 0; /* set by '-f' argument to main */
static int initial_zoom_count = 0; /* set by '-z' flag */
static int hot_restart = 0;
static struct timeval sim_start_time;
static char **saved_args; /* for hot restarting */
static int must_redraw_window = 1;

static char *executable_dir;

static struct color_sensor_ui {
	struct sim_slider_input *input[5];
	float color_value[5];
} color_sensor_ui = { 0 };

static struct analog_sensor_ui {
	struct sim_slider_input *input[5];
	float value[5];
} analog_sensor_ui = { 0 };

static void color_sensor_ui_callback(__attribute__((unused)) struct sim_slider_input *s,
				__attribute__((unused))  float v)
{
	// FIXME remove this
	button_reset_last_input_timestamp(); /* inhibit screensaver */
}

static void color_sensor_ui_mouse_input(SDL_Window *w, struct SDL_MouseButtonEvent *event)
{
	for (int i = 0; i < (int) ARRAY_SIZE(color_sensor_ui.input); i++)
		slider_input_button_press(w, color_sensor_ui.input[i], (int) event->x, (int) event->y);
}

static void setup_color_sensor_ui(void)
{
	for (int i = 0; i < (int) ARRAY_SIZE(color_sensor_ui.input); i++) {
		color_sensor_ui.input[i] = slider_input_create(0.02, 0.05 + i * 0.05, 0.2, 0.02,
						&color_sensor_ui.color_value[i], 0);
		slider_set_button_press_callback(color_sensor_ui.input[i], color_sensor_ui_callback);
		color_sensor_ui.color_value[i] = 0.5;
	}
	slider_input_set_color(color_sensor_ui.input[0], 255, 0, 0, 255); /* red */
	slider_input_set_color(color_sensor_ui.input[1], 0, 255, 0, 255); /* green */
	slider_input_set_color(color_sensor_ui.input[2], 0, 0, 255, 255); /* blue */
	slider_input_set_color(color_sensor_ui.input[3], 255, 255, 255, 255); /* white */
	slider_input_set_color(color_sensor_ui.input[4], 255, 0, 255, 255); /* magenta */
	color_sensor_ui_callback(0, 0); /* Set initial color sample to match sliders */
}

static void free_color_sensor_ui(void)
{
	for (int i = 0; i < (int) ARRAY_SIZE(color_sensor_ui.input); i++) {
		free(color_sensor_ui.input[i]);
		color_sensor_ui.input[i] = NULL;
	}
}

static void draw_color_sensor_ui(SDL_Window *w, SDL_Renderer *r)
{
	for (int i = 0; i < (int) ARRAY_SIZE(color_sensor_ui.input); i++)
		slider_input_draw(w, r, color_sensor_ui.input[i]);
}

static void analog_sensor_ui_callback(__attribute__((unused)) struct sim_slider_input *s,
				__attribute__((unused))  float v)
{
	struct analog_sim_values values;

	/* 3250 mV = High-Z -- basically, the battery voltage, I think */
	values.value[ANALOG_CHAN_0] = (int) (3250 * analog_sensor_ui.value[0]);
	values.value[ANALOG_CHAN_0] = (int) (3250 * analog_sensor_ui.value[1]);
	values.value[ANALOG_CHAN_0] = (int) (2000 * analog_sensor_ui.value[2]);
	values.value[ANALOG_CHAN_VOLUME] = (int) (3250 * analog_sensor_ui.value[3]);
	/* not sure about this one... */
	values.value[ANALOG_CHAN_MCU_TEMP] = (int) (706 * analog_sensor_ui.value[4]);
	analog_sensors_set_values(values);
	button_reset_last_input_timestamp(); /* inhibit screensaver */
}

static void analog_sensor_ui_mouse_input(SDL_Window *w, struct SDL_MouseButtonEvent *event)
{
	for (int i = 0; i < (int) ARRAY_SIZE(analog_sensor_ui.input); i++)
		slider_input_button_press(w, analog_sensor_ui.input[i], (int) event->x, (int) event->y);
}

static void setup_analog_sensor_ui(void)
{
	for (int i = 0; i < (int) ARRAY_SIZE(analog_sensor_ui.input); i++) {
		analog_sensor_ui.input[i] = slider_input_create(0.75, 0.05 + i * 0.05, 0.2, 0.02,
						&analog_sensor_ui.value[i], 0);
		slider_set_button_press_callback(analog_sensor_ui.input[i], analog_sensor_ui_callback);
		analog_sensor_ui.value[i] = 0.5;
	}
	analog_sensor_ui_callback(0, 0);
}

static void free_analog_sensor_ui(void)
{
	for (int i = 0; i < (int) ARRAY_SIZE(analog_sensor_ui.input); i++) {
		free(analog_sensor_ui.input[i]);
		analog_sensor_ui.input[i] = NULL;
	}
}

static void draw_analog_sensor_ui(SDL_Window *w, SDL_Renderer *r)
{
	for (int i = 0; i < (int) ARRAY_SIZE(analog_sensor_ui.input); i++)
		slider_input_draw(w, r, analog_sensor_ui.input[i]);
}

__attribute__((unused)) static void draw_sensor_ui(SDL_Window *w, SDL_Renderer *r)
{
	draw_color_sensor_ui(w, r);
	draw_analog_sensor_ui(w, r);
}

static void sensor_ui_mouse_input(SDL_Window *w, struct SDL_MouseButtonEvent *event)
{
	color_sensor_ui_mouse_input(w, event);
	analog_sensor_ui_mouse_input(w, event);
}

static void setup_sensor_uis(void)
{
	setup_color_sensor_ui();
	setup_analog_sensor_ui();
}

static void free_sensor_ui(void)
{
	free_color_sensor_ui();
	free_analog_sensor_ui();
}

// Forward declaration
void hal_start_sdl(int *argc, char ***argv);

// Do hardware-specific initialization.
void hal_init(void) {

    display_init_gpio();
    led_pwm_init_gpio();
    button_init_gpio();
    ir_init();
    display_reset();
    rtc_init_badge(0);
}

void *main_in_thread(void* params) {
    int (*main_func)(int, char**) = params;
    main_func(sim_argc, sim_argv);
    return NULL;
}

static struct option long_options[] = {
	{ "badge-id", required_argument, NULL, 'i' },
	{ "fullscreen", no_argument, NULL, 'f' },
	{ "hotrestart", no_argument, NULL, 'h' },
	{ "zoom", required_argument, NULL, 'z' },
	{ NULL, 0, 0, 0 },
};

static void usage(void)
{
	fprintf(stderr, "usage: badge [--badge-id 0x1234567812345678 ] [ --fullscreen ] [ --hotrestart ] [ --zoom n ]\n");
	exit(1);
}

static void process_options(int argc, char **argv)
{
	int c, rc;
	uint64_t badge_id;

	while (1) {
		int option_index;
		c = getopt_long(argc, argv, "fhi:z:", long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 'i':
			rc = sscanf(optarg, "%lx", &badge_id);
			if (rc != 1) {
				fprintf(stderr, "Failed to parse badge ID '%s', using default\n", optarg);
			} else {
				fprintf(stderr, "Using custom badge ID: 0x%016lx\n", badge_id);
				set_custom_badge_id(badge_id);
			}
			break;
		case 'f': /* full screen mode */
			fullscreen_flag = 1;
			break;
		case 'z': /* zoom level */
			{
				int zoom_level;
				int rc = sscanf(optarg, "%d", &zoom_level);
				if (rc != 1) {
					usage();
					__builtin_unreachable();
				} else {
					initial_zoom_count = zoom_level;
				}
			}
			break;
		case 'h':
			hot_restart = 1;
			break;
		default:
			usage();
			__builtin_unreachable();
			break;
		}
	}
}

/* Copy program arguments into an something suitable for calling execv() */
static void save_args(int argc, char *argv[], char ***saved_argv)
{
	*saved_argv = calloc(sizeof((*saved_argv)[0]), argc + 1);
	for (int i = 0; i < argc; i++)
		(*saved_argv)[i] = strdup(argv[i]);
	(*saved_argv)[argc] = NULL;
}

/* Free args allocated by save_args(); */
static void free_argv(char **argv)
{
        for (int i = 0; argv[i]; i++)
                free(argv[i]);
        free(argv);
}

int hal_run_main(int (*main_func)(int, char**), int argc, char** argv)
{
    sim_argc = argc;
    sim_argv = argv;

    save_args(argc, argv, &saved_args); /* For later hot restart */

    gettimeofday(&sim_start_time, NULL);

    pthread_t app_thread;
    pthread_create(&app_thread, NULL, main_in_thread, main_func);

    process_options(argc, argv);
    audio_init();
    hal_start_sdl(&argc, &argv);

    return 0;
}

void hal_deinit(void) {
    flash_deinit();
    printf("stub fn: %s in %s\n", __FUNCTION__, __FILE__);
    free_argv(saved_args);
}

void hal_reboot(void) {
    printf("stub fn: %s in %s\n", __FUNCTION__, __FILE__);
    exit(0);
}

uint32_t hal_disable_interrupts(void)
{
	disable_interrupts();
	return 0;
}

void hal_restore_interrupts(__attribute__((unused)) uint32_t state)
{
	enable_interrupts();
}

static char *badge_image_pixels, *rotated_badge_image_pixels, *badge_background_pixels;
static char *quit_confirm_pixels;
int quit_confirm_active = 0;
static char *led_pixels;
static int badge_image_width, badge_image_height; /* badge in its "normal" orientation */
static int rotated_badge_image_width, rotated_badge_image_height; /* rotated 90 deg CCW orientation */
static int badge_background_width, badge_background_height;
static int quit_confirm_width, quit_confirm_height;
static int led_width, led_height;
static SDL_Joystick *joystick = NULL;

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *pix_buf, *landscape_pix_buf, *badge_image, *rotated_badge_image, *badge_background_image;
static SDL_Texture *led_image;
static SDL_Texture *quit_confirm_image;
static char *program_title;
extern int lcd_brightness;

static void draw_led_text(SDL_Renderer *renderer, int x, int y)
{
#define LETTER_SPACING 12
    /* Literally draws L E D */
    /* Draw L */
    SDL_RenderDrawLine(renderer, x, y, x, y - 10);
    SDL_RenderDrawLine(renderer, x, y, x + 8, y);

    x += LETTER_SPACING;

    /* Draw E */
    SDL_RenderDrawLine(renderer, x, y, x, y - 10);
    SDL_RenderDrawLine(renderer, x, y, x + 8, y);
    SDL_RenderDrawLine(renderer, x, y - 5, x + 5, y - 5);
    SDL_RenderDrawLine(renderer, x, y - 10, x + 8, y - 10);

    x += LETTER_SPACING;

    /* Draw D */
    SDL_RenderDrawLine(renderer, x, y, x, y - 10);
    SDL_RenderDrawLine(renderer, x, y, x + 8, y);
    SDL_RenderDrawLine(renderer, x, y - 10, x + 8, y - 10);
    SDL_RenderDrawLine(renderer, x + 8, y - 10, x + 10, y - 5);
    SDL_RenderDrawLine(renderer, x + 8, y, x + 10, y - 5);
}

void flareled(unsigned char r, unsigned char g, unsigned char b)
{
    led_color.red = r;
    led_color.green = g;
    led_color.blue = b;
}

static void draw_badge_background(void)
{
	if (badge_background_image)
		SDL_RenderCopy(renderer, badge_background_image, NULL, NULL);
}

static void draw_badge_image(struct sim_lcd_params *slp)
{
	static int created_textures = 0;
	float bx1, by1, bx2, by2;
	float cx1, cy1, cx2, cy2;
	float sx1, sy1, sx2, sy2;
	int sx, sy;
	struct lcd_to_circuit_board_relation lcdp;

	if (!created_textures) {
		if (rotated_badge_image_pixels) {
			rotated_badge_image = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC,
					rotated_badge_image_width, rotated_badge_image_height);
			SDL_SetTextureBlendMode(rotated_badge_image, SDL_BLENDMODE_BLEND);
			SDL_UpdateTexture(rotated_badge_image, NULL, rotated_badge_image_pixels, rotated_badge_image_width * 4);
		}
		if (badge_image_pixels) {
			badge_image = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC,
							badge_image_width, badge_image_height);
			SDL_SetTextureBlendMode(badge_image, SDL_BLENDMODE_BLEND);
			SDL_UpdateTexture(badge_image, NULL, badge_image_pixels, badge_image_width * 4);
		}
		if (badge_background_pixels) {
			badge_background_image = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC,
							badge_background_width, badge_background_height);
			SDL_UpdateTexture(badge_background_image, NULL, badge_background_pixels, badge_background_width * 4);
		}
		if (led_pixels) {
			led_image = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC,
							led_width, led_height);
			SDL_SetTextureBlendMode(led_image, SDL_BLENDMODE_BLEND);
			SDL_UpdateTexture(led_image, NULL, led_pixels, led_width * 4);
		}
		if (quit_confirm_pixels) {
			quit_confirm_image = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC,
							quit_confirm_width, quit_confirm_height);
			SDL_SetTextureBlendMode(quit_confirm_image, SDL_BLENDMODE_BLEND);
			SDL_UpdateTexture(quit_confirm_image, NULL, quit_confirm_pixels, quit_confirm_width * 4);
		}
		created_textures = 1;
	}

        SDL_GetWindowSize(window, &sx, &sy);

	/* get corners of the screen inside the badge image */
	if (slp->orientation == SIM_LCD_ORIENTATION_ROTATED)
		lcdp = rotated_lcd_to_board();
	else
		lcdp = unrotated_lcd_to_board();

	/* corners of the sim screen on the computer screen */
	sx1 = slp->xoffset;
	sy1 = slp->yoffset;
	sx2 = slp->xoffset + slp->width;
	sy2 = slp->yoffset + slp->height;

	/* where corners of badge image land on screen, by similar triangles */
	float fx = (sx2 - sx1) / (lcdp.x2 - lcdp.x1);
	float fy = (sy2 - sy1) / (lcdp.y2 - lcdp.y1);
	// fx = fy;
	bx1 =   sx1 - fx * lcdp.x1;
	by1 =   sy1 - fy * lcdp.y1;
	if (slp->orientation == SIM_LCD_ORIENTATION_ROTATED) {
		bx2 =   bx1 + fx * (rotated_badge_image_width - 1);
		by2 =   by1 + fy * (rotated_badge_image_height - 1);
	} else {
		bx2 =   bx1 + fx * (badge_image_width - 1);
		by2 =   by1 + fy * (badge_image_height - 1);
	}

	/* note, some or all of bx1, by1, bx2, by2 may be off screen. Compute clip rect */
	if (bx1 < 0) {
		cx1 = -bx1 / fx;
		bx1 = 0;
	} else {
		cx1 = 0;
	}
	if (by1 < 0) {
		cy1 = -by1 / fy;
		by1 = 0;
	} else {
		cy1 = 0;
	}
	if (bx2 >= sx) {
		if (slp->orientation == SIM_LCD_ORIENTATION_ROTATED)
			cx2 = rotated_badge_image_width - (bx1 - sx) / fx;
		else
			cx2 = badge_image_width - (bx1 - sx) / fx;
		bx2 = sx - 1;
	} else {
		if (slp->orientation == SIM_LCD_ORIENTATION_ROTATED)
			cx2 = rotated_badge_image_width - 1;
		else
			cx2 = badge_image_width - 1;
	}
	if (by2 >= sy) {
		if (slp->orientation == SIM_LCD_ORIENTATION_ROTATED)
			cy2 = rotated_badge_image_height - (by2 - sy) / fy;
		else
			cy2 = badge_image_height - (by2 - sy) / fy;
		by2 = sy - 1;
	} else {
		if (slp->orientation == SIM_LCD_ORIENTATION_ROTATED)
			cy2 = rotated_badge_image_height - 1;
		else
			cy2 = badge_image_height - 1;
	}

	SDL_Rect from_rect = { (int) cx1, (int) cy1, (int) (cx2 - cx1), (int) (cy2 - cy1) };
	SDL_Rect to_rect = { (int) bx1, (int) by1, (int) (bx2 - bx1), (int) (by2 - by1) };
	if (slp->orientation == SIM_LCD_ORIENTATION_ROTATED && rotated_badge_image)
		SDL_RenderCopy(renderer, rotated_badge_image, &from_rect, &to_rect);
	else if (badge_image)
		SDL_RenderCopy(renderer, badge_image, &from_rect, &to_rect);
}

static void draw_button_press(struct button_coord *b)
{
    SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);
    SDL_RenderFillRect(renderer, &(SDL_Rect) { b->x - 20, b->y - 20, 40, 40} );
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderFillRect(renderer, &(SDL_Rect) { b->x - 10, b->y - 10, 20, 20} );
}

#if BADGE_HAS_ROTARY_SWITCHES
static void draw_rotary_button_position(struct button_coord *button, int which_rotary)
{
	float x, y;
	int angle = sim_get_rotary_angle(which_rotary);

	x = (cosine(angle) * 40) >> 8;
	y = (sine(angle) * 40) >> 8;

	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
	SDL_RenderDrawLine(renderer, button->x, button->y, button->x + x, button->y + y);
}
#endif

static void draw_button_inputs(struct sim_lcd_params *slp)
{
	struct sim_button_status button_status;
	int w, h;

	if (slp->orientation == SIM_LCD_ORIENTATION_UNROTATED) {
		w = badge_image_width;
		h = badge_image_height;
	} else {
		w = rotated_badge_image_width;
		h = rotated_badge_image_height;
	}
	struct button_coord_list bcl = get_button_coords(slp, w, h);
	button_status = get_sim_button_status();

	/* TODO: now there are 4 buttons, A, B, and the two rotary switches */
	if (button_status.button_a)
		draw_button_press(&bcl.a_button);
	if (button_status.button_b)
		draw_button_press(&bcl.b_button);
	if (button_status.dpad_right)
		draw_button_press(&bcl.dpad_right);
	if (button_status.dpad_left)
		draw_button_press(&bcl.dpad_left);
	if (button_status.dpad_up)
		draw_button_press(&bcl.dpad_up);
	if (button_status.dpad_down)
		draw_button_press(&bcl.dpad_down);
#if BADGE_HAS_ROTARY_SWITCHES
	if (button_status.left_rotary_button)
		draw_button_press(&bcl.left_rotary);
	if (button_status.right_rotary_button)
		draw_button_press(&bcl.right_rotary);
	draw_rotary_button_position(&bcl.right_rotary, 0);
	draw_rotary_button_position(&bcl.left_rotary, 1);
#endif
	if (button_status.record)
		draw_button_press(&bcl.record);
	if (button_status.play)
		draw_button_press(&bcl.play);
	if (button_status.fastforward)
		draw_button_press(&bcl.fastforward);
	if (button_status.stop_eject)
		draw_button_press(&bcl.stop_eject);
	if (button_status.rewind)
		draw_button_press(&bcl.rewind);
}

static void draw_flare_led(struct sim_lcd_params *slp)
{
	int w, h, x, y, i, j;
	char *p;

	/* Draw simulated flare LED */
	SDL_GetWindowSize(window, &x, &y);
	x = x - 100;
	y = 50;
	draw_led_text(renderer, x, y);
	SDL_SetRenderDrawColor(renderer, led_color.red, led_color.blue, led_color.green, 0xff);
	SDL_RenderFillRect(renderer, &(SDL_Rect) { x, y + 20, 51, 51} );
	SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
	SDL_RenderDrawRect(renderer, &(SDL_Rect) { x, y + 20, 50, 50} );

	/* If LED is too dim, don't draw it. */
	if (led_color.red < 75 && led_color.green < 75 && led_color.blue < 75)
		return;

	if (slp->orientation == SIM_LCD_ORIENTATION_UNROTATED) {
		w = badge_image_width;
		h = badge_image_height;
	} else {
		w = rotated_badge_image_width;
		h = rotated_badge_image_height;
	}
	struct button_coord_list bcl = get_button_coords(slp, w, h);
	x = bcl.led.x - led_width / 2;
	y = bcl.led.y - led_height / 2;
	if (led_pixels) {
		for (i = 0; i < led_height; i++) {
			for (j = 0; j < led_width; j++) {
				p = &led_pixels[i * led_width * 4 + j * 4];
				p[0] = (char) led_color.red;
				p[1] = (char) led_color.green;
				p[2] = (char) led_color.blue;
			}
		}
	}
	if (led_image) {
		SDL_SetTextureBlendMode(led_image, SDL_BLENDMODE_BLEND);
		SDL_UpdateTexture(led_image, NULL, led_pixels, led_width * 4);
		SDL_RenderCopy(renderer, led_image, NULL, &(SDL_Rect) { x, y, led_width, led_height});
	}
}

int quit_confirmed(int mousex, int mousey)
{
	int x, y, x1, y1, x2, y2, w, h;

	SDL_GetWindowSize(window, &w, &h);

	x = w / 2 - quit_confirm_width / 2;
	y = h / 2 - quit_confirm_height / 2;

	/* coords of "YES" box */
	x1 = x + 24;
	y1 = y + 72;
	x2 = x + 101;
	y2 = y + 123;

	if (mousex >= x1 && mousex <= x2 && mousey >= y1 && mousey <= y2)
		return 1;
	return 0;
}

static void maybe_draw_quit_confirmation(void)
{
	int x, y, x1, y1;

	SDL_GetWindowSize(window, &x, &y);

	x1 = x / 2 - quit_confirm_width / 2;
	y1 = y / 2 - quit_confirm_height / 2;

	if (!quit_confirm_active)
		return;
	if (quit_confirm_image)
		SDL_RenderCopy(renderer, quit_confirm_image, NULL, &(SDL_Rect) { x1, y1, quit_confirm_width, quit_confirm_height });
}

#define HAVE_ACCELEROMETER 0
#if HAVE_ACCELEROMETER
static union quat badge_orientation = IDENTITY_QUAT_INITIALIZER;
static union vec3 gravity_vector = { { 0.0f, 0.0f, 1.0f } };

static union vec3 badge_orientation_points[] = {
	{ { -100.0f, -50.0f, 0.0f } },
	{ { 100.0f, -50.0f, 0.0f } },
	{ { 100.0f, 50.0f, 0.0f } },
	{ { -100.0f, 50.0f, 0.0f } },
	{ { -100.0f, -50.0f, 0.0f } },

	{ { -1, -1, -1 } },

	{ { -50.0f, -40.0f, 0.0f } },
	{ { 50.0f, -40.0f, 0.0f } },
	{ { 50.0f, 45.0f, 0.0f } },
	{ { -50.0f, 45.0f, 0.0f } },
	{ { -50.0f, -40.0f, 0.0f } },

	{ { -1, -1, -1 } },

	{ { -90, -45, 0 } },
	{ { -80, -45, 0 } },
	{ { -80, -35, 0 } },
	{ { -90, -35, 0 } },
	{ { -90, -45, 0 } },

	{ { -1, -1, -1 } },

	{ { 90, -45, 0 } },
	{ { 80, -45, 0 } },
	{ { 80, -35, 0 } },
	{ { 90, -35, 0 } },
	{ { 90, -45, 0 } },

	{ { -1, -1, -1 } },

	{ { 60, 35, 0} },
	{ { 70, 35, 0} },
	{ { 70, 45, 0} },
	{ { 60, 45, 0} },
	{ { 60, 35, 0} },

	{ { -1, -1, -1 } },

	{ { 80, 25, 0} },
	{ { 90, 25, 0} },
	{ { 90, 35, 0} },
	{ { 80, 35, 0} },
	{ { 80, 25, 0} },

	{ { -1, -1, -1 } },

	{ { -80, 20, 0 } },
	{ { -80, 45, 0 } },

	{ { -1, -1, -1 } },

	{ { -90, 33, 0 } },
	{ { -70, 33, 0 } },
};

static union vec3 orientation_indicator_position = { { 0.0f, 0.0f, 100.0f } };
#endif

#define BADGE_ORIENTATION_X (100.0f)
#define BADGE_ORIENTATION_Y (500.0f)

#if HAVE_ACCELEROMETER
static void draw_badge_orientation_indicator(SDL_Renderer *renderer, float x, float y, float scale, union vec3 *badge_position, union quat *orientation)
{
	const int n = ARRAYSIZE(badge_orientation_points);
	const float camera_z = 100.0f;
	union vec3 indicator[ARRAYSIZE(badge_orientation_points)];

	/* Start with the original 3D orientation indicator coordinates */
	memcpy(indicator, badge_orientation_points, sizeof(indicator));

	/* Rotate the badge into its current 3D orientation */
	for (int i = 0; i < n; i++)
		quat_rot_vec_self(&indicator[i], orientation);

	/* Translate the badge into its current 3D position */
	for (int i = 0; i < n; i++) {
		indicator[i].v.x += badge_position->v.x;
		indicator[i].v.y += badge_position->v.y;
		indicator[i].v.z += badge_position->v.z;
	}

	/* Do perspective transformation */
	for (int i = 0; i < n; i++) {
		indicator[i].v.x = (camera_z * indicator[i].v.x) / (camera_z + indicator[i].v.z);
		indicator[i].v.y = (camera_z * indicator[i].v.y) / (camera_z + indicator[i].v.z);
	}

	/* translate and scale projected (x,y) coords */
	for (int i = 0; i < n; i++) {
		indicator[i].v.x = (scale * indicator[i].v.x) + x;
		indicator[i].v.y = (scale * indicator[i].v.y) + y;
	}

	/* Check if we are seeing the front or the back side of the indicator */
	union vec3 badge_normal = { { 0.0f, 0.0f, -1.0f } };
	union vec3 to_camera = { { 0.0f, 0.0f, -1.0f } };
	quat_rot_vec_self(&badge_normal, orientation);
	float dot = vec3_dot(&badge_normal, &to_camera);

	/* Compute the new gravity vector */
	union quat inverse_rotation;
	gravity_vector.v.x = 0.0f;
	gravity_vector.v.y = 0.0f;
	gravity_vector.v.z = -1.0f;
	quat_inverse(&inverse_rotation, orientation);
	quat_rot_vec_self(&gravity_vector, &inverse_rotation);

	/* Draw the orientation indicator */
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
	SDL_RenderFillRect(renderer, &(SDL_Rect) { x - 75, y - 75, 150, 150} );

	if (dot >= 0) /* we see front of badge, blue */
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xff, 0xff);
	else /* we see back of badge, red */
		SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);
	union vec3 *prev = &indicator[0];
	for (int i = 1; i < n; i++) {
		if (fabsf(badge_orientation_points[i].v.z - -1.0f) < 0.001) {
			prev = &indicator[i + 1];
			i++;
			continue;
		}
		float x1 = prev->v.x;
		float y1 = prev->v.y;
		float x2 = indicator[i].v.x;
		float y2 = indicator[i].v.y;
		SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
		prev = &indicator[i];
	}
}
#endif

static int draw_window(SDL_Renderer *renderer, SDL_Texture *texture, SDL_Texture *landscape_texture)
{
    extern uint8_t display_array[LCD_YSIZE][LCD_XSIZE][3];

#define ECONOMIZE_DRAWING 1
#if ECONOMIZE_DRAWING
    static uint8_t prev_display_array[LCD_YSIZE][LCD_XSIZE][3];
    static int first_time = 1;
#endif

#define COUNT_DRAW_CALLS 0
#if COUNT_DRAW_CALLS
    static int total_calls = 0;
    static int total_draw_calls = 0;

    total_calls++;
    if ((total_calls % 0x100) == 0)
	fprintf(stderr, "total calls = %d, draw calls = %d\n", total_calls, total_draw_calls);
#endif

#if ECONOMIZE_DRAWING
    if (first_time) {
	memcpy(prev_display_array, display_array, sizeof(prev_display_array));
	first_time = 0;
    }

    /* If the display hasn't changed, and no events, then no need to draw the exact
     * same thing again.
     */
    if (!must_redraw_window &&
	memcmp(prev_display_array, display_array, sizeof(prev_display_array)) == 0)
	return 0;
    memcpy(prev_display_array, display_array, sizeof(prev_display_array));
    must_redraw_window = 0;
#endif

#if COUNT_DRAW_CALLS
    total_draw_calls++;
#endif

    struct sim_lcd_params slp = get_sim_lcd_params();

    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderClear(renderer);

    draw_badge_background();
    draw_badge_image(&slp);
    draw_button_inputs(&slp);

    /* Draw the pixels of the screen */


    /* This:
     *
     *    SDL_UpdateTexture(texture, NULL, display_array, LCD_XSIZE * 3);
     *
     * doesn't work right for some reason I don't quite.  The texture apparently
     * wants the data in BGRA order.
     *
     * In any case, for now, we can copy display_array inserting the
     * alpha channel that SDL_RenderCopy seems to expect. Modern
     * computers can do this copy in microseconds, so it's not a big deal.
     */

    /* Copy display_array[] but add on an alpha channel. SDL_RenderCopy() seems to need it.
     * Plus we try to use it to implement LCD brightness. */
    static uint8_t display_array_with_alpha[LCD_YSIZE][LCD_XSIZE][4];
    static uint8_t landscape_display_array_with_alpha[LCD_XSIZE][LCD_YSIZE][4];

    if (slp.orientation == SIM_LCD_ORIENTATION_LANDSCAPE) { /* LCD screen orientation */
        float level = (float) lcd_brightness / 255.0f;
        for (int y = 0; y < LCD_YSIZE; y++) {
            for (int x = 0; x < LCD_XSIZE; x++) {
                /* SDL texture seems to want data in BGRA order, and since we're copying
                 * anyway, we can emulate LCD brightness here too. */
                display_array_with_alpha[y][x][2] = (uint8_t) (level * display_array[y][x][0]);
                display_array_with_alpha[y][x][1] = (uint8_t) (level * display_array[y][x][1]);
                display_array_with_alpha[y][x][0] = (uint8_t) (level * display_array[y][x][2]);
	        /* I tried to implement lcd brightness via alpha channel, but it doesn't seem to work */
                /* display_array_with_alpha[y][x][3] = 255 - lcd_brightness; */
                display_array_with_alpha[y][x][3] = 255;
            }
        }
        SDL_UpdateTexture(texture, NULL, display_array_with_alpha, LCD_XSIZE * 4);
        SDL_Rect from_rect = { 0, 0, LCD_XSIZE, LCD_YSIZE };
        SDL_Rect to_rect = { slp.xoffset, slp.yoffset, slp.width, slp.height };
        SDL_RenderCopy(renderer, texture, &from_rect, &to_rect);
    } else { /* portrait */
        float level = (float) lcd_brightness / 255.0f;
        for (int x = 0; x < LCD_XSIZE; x++) {
            for (int y = 0; y < LCD_YSIZE; y++) {
                /* SDL texture seems to want data in BGRA order, and since we're copying
                 * anyway, we can emulate LCD brightness here too. */
                landscape_display_array_with_alpha[x][LCD_YSIZE - y - 1][2] = (uint8_t) (level * display_array[y][x][0]);
                landscape_display_array_with_alpha[x][LCD_YSIZE - y - 1][1] = (uint8_t) (level * display_array[y][x][1]);
                landscape_display_array_with_alpha[x][LCD_YSIZE - y - 1][0] = (uint8_t) (level * display_array[y][x][2]);
	        /* I tried to implement lcd brightness via alpha channel, but it doesn't seem to work */
                /* display_array_with_alpha[y][x][3] = 255 - lcd_brightness; */
                landscape_display_array_with_alpha[x][y][3] = 255;
            }
        }
        SDL_UpdateTexture(landscape_texture, NULL, landscape_display_array_with_alpha, LCD_YSIZE * 4);
        SDL_Rect from_rect = { 0, 0, LCD_YSIZE, LCD_XSIZE };
        SDL_Rect to_rect = { slp.xoffset, slp.yoffset, slp.width, slp.height };
        SDL_RenderCopy(renderer, landscape_texture, &from_rect, &to_rect);
    }


    /* Draw a border around the simulated screen */
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderDrawLine(renderer, slp.xoffset - 1, slp.yoffset - 1., slp.xoffset + slp.width + 1, slp.yoffset - 1); /* top */
    SDL_RenderDrawLine(renderer, slp.xoffset - 1, slp.yoffset - 1., slp.xoffset - 1, slp.yoffset + slp.height + 1); /* left */
    SDL_RenderDrawLine(renderer, slp.xoffset - 1, slp.yoffset + slp.height + 1, slp.xoffset + slp.width + 1, slp.yoffset + slp.height + 1); /* bottom */
    SDL_RenderDrawLine(renderer, slp.xoffset + slp.width + 1, slp.yoffset - 1, slp.xoffset + slp.width + 1, slp.yoffset + slp.height + 1); /* right */

    draw_flare_led(&slp);

#if HAVE_ACCELEROMETER
    draw_badge_orientation_indicator(renderer, BADGE_ORIENTATION_X, BADGE_ORIENTATION_Y, 1.0f,
		&orientation_indicator_position, &badge_orientation);
#endif

    /* 2025, don't have sensors this year */
    /* draw_sensor_ui(window, renderer); */

    maybe_draw_quit_confirmation();

    SDL_RenderPresent(renderer);
    return 0;
}

static void enable_sdl_fullscreen_sanity(void)
{
	/* If SDL_VIDEO_MINIMIZE_ON_FOCUS_LOSS isn't set to zero,
	 * fullscreen window behavior is *insane* by default.
	 *
	 * Alt-tab and Alt-left-arrow and Alt-right-arrow will *minimize*
	 * the window, pushing it to the bottom of the stack, so when you
	 * alt-tab again, and expect the window to re-appear, it doesn't.
	 * Instead, a different window appears, and you have to alt-tab a
	 * zillion times through all your windows until you finally get to
	 * the bottom where your minimized fullscreen window sits, idiotically.
	 *
	 * Let's make sanity the default.  The last parameter of setenv()
	 * says do not overwrite the value if it is already set. This will
	 * allow for any completely insane individuals who somehow prefer
	 * this idiotc behavior to still have it.  But they will not get
	 * it by default.
	 */

	char *v = getenv("SDL_VIDEO_MINIMIZE_ON_FOCUS_LOSS");
	if (v && strncmp(v, "1", 1) == 0) {
		fprintf(stderr, "You have SDL_VIDEO_MINIMIZE_ON_FOCUS_LOSS set to 1!\n");
		fprintf(stderr, "I highly recommend you set it to zero. But it's your sanity\n");
		fprintf(stderr, "at stake, not mine, so whatever. Let's proceed anyway.\n");
	}
	setenv("SDL_VIDEO_MINIMIZE_ON_FOCUS_LOSS", "0", 0);	/* Final 0 means don't override user's prefs */
								/* I am Very tempted to set it to 1. */
}

static int start_sdl(void)
{
    enable_sdl_fullscreen_sanity();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
        fprintf(stderr, "Unable to initialize SDL (Video):  %s\n", SDL_GetError());
        return 1;
    }
    if (SDL_NumJoysticks() >= 1)
        joystick = SDL_JoystickOpen(0);
    if (SDL_Init(SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "Unable to initialize SDL (Events):  %s\n", SDL_GetError());
        return 1;
    }

    executable_dir = SDL_GetBasePath();

    atexit(SDL_Quit);
    return 0;
}

static void load_image(char *filename, char **pixels, int *width, int *height)
{
	int w = 0, h = 0, a = 0;
	char whynot[1024];
	char path[PATH_MAX];
	struct stat statbuf;

	snprintf(path, sizeof(path), "%s", filename);
#ifndef __EMSCRIPTEN__
	int rc = stat(path, &statbuf);
	if (rc < 0 && executable_dir) {
		/* Try relative to the executable dir */
		snprintf(path, sizeof(path), "%s/../%s", executable_dir, filename);
	}
#endif

	if (*pixels)
		return;

	*pixels = png_utils_read_png_image(path,
		0, 0, 0, &w, &h, &a, whynot, sizeof(whynot) - 1);
	if (!*pixels) {
		fprintf(stderr, "Failed to load image \"%s\": %s\n", path, whynot);
		return;
	}
	*width = w;
	*height = h;
}

static void load_badge_images(void)
{

	badge_image_width = 1024;
	badge_image_height = 723;
	load_image("../images/badge-image-1024.png", &badge_image_pixels,
			&badge_image_width, &badge_image_height);
	rotated_badge_image_width = 723;
	rotated_badge_image_height = 1024;
	load_image("../images/badge-image-rotated.png", &rotated_badge_image_pixels,
			&rotated_badge_image_width, &rotated_badge_image_height);
	badge_background_width = 1024;
	badge_background_height = 672;
	load_image("../images/badge-background.png", &badge_background_pixels,
			&badge_background_width, &badge_background_height);
	led_width = 160;
	led_height = 160;
	load_image("../images/badge-led.png", &led_pixels,
			&led_width, &led_height);
	quit_confirm_width = 256;
	quit_confirm_height = 186;
	load_image("../images/really-quit.png", &quit_confirm_pixels,
			&quit_confirm_width, &quit_confirm_height);
}

void toggle_fullscreen_mode(void)
{
	if (fullscreen)
		SDL_SetWindowFullscreen(window, 0);
	else
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	fullscreen = !fullscreen;
	/* configure event takes care of resizing window */
}

static void setup_window_and_renderer(SDL_Window **window, SDL_Renderer **renderer,
				SDL_Texture **texture, SDL_Texture **landscape_texture)
{
    char window_title[1024];

    load_badge_images();
    snprintf(window_title, sizeof(window_title), "HackRVA Badge Emulator - %s", program_title);
    free(program_title);
    *window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               0, 0, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
    if (!*window) {
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_SetWindowSize(*window, 800, 600);
    // SDL_SetWindowFullscreen(*window, SDL_WINDOW_FULLSCREEN_DESKTOP);

    *renderer = SDL_CreateRenderer(*window, -1, 0);
    if (!*renderer) {
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        exit(1);
    }

    *texture = SDL_CreateTexture(*renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, LCD_XSIZE, LCD_YSIZE);
    if (!*texture) { 
        fprintf(stderr, "Could not create texture: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_SetTextureBlendMode(*texture, SDL_BLENDMODE_BLEND);
    *landscape_texture = SDL_CreateTexture(*renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, LCD_YSIZE, LCD_XSIZE);
    if (!*landscape_texture) { 
        fprintf(stderr, "Could not create landscape texture: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_SetTextureBlendMode(*landscape_texture, SDL_BLENDMODE_BLEND);

    SDL_ShowWindow(*window);
    SDL_RenderClear(*renderer);
    SDL_RenderPresent(*renderer);
}

int buttonfuzzer_on = 0;

static int buttonfuzzer(SDL_Event *event)
{
	static unsigned int seed = 0xa5a5a5a5;

	if (!buttonfuzzer_on)
		return 0; 

	if ((xorshift(&seed) % 100) < 75)
		return 0;

	static const SDL_Event keypress_template = { 
		.key = {
			.type = SDL_KEYDOWN,
			.timestamp = 0,
			.windowID = 0,
			.state = SDL_PRESSED,
			.repeat = 0,
		}
	};

	static const SDL_Event keyrelease_template = {
		.key = {
			.type = SDL_KEYUP,
			.timestamp = 0,
			.windowID = 0,
			.state = SDL_RELEASED,
			.repeat = 0,
		}
	};

	int n = (xorshift(&seed) % 22); /* eleven buttons, press or release = 22 combos */

	if (n < 11)
		*event = keyrelease_template;
	else
		*event = keypress_template;

	switch (n % 11) {
	case 0:
		event->key.keysym.sym = SDLK_SPACE; /* A button */
		break;
	case 1:
		event->key.keysym.sym = SDLK_b; /* B button */ 
		break;
	case 2:
		event->key.keysym.sym = SDLK_UP; /* up d-pad */
		break;
	case 3:
		event->key.keysym.sym = SDLK_LEFT; /* left d-pad */
		break;
	case 4:
		event->key.keysym.sym = SDLK_RIGHT; /* right d-pad */
		break;
	case 5:
		event->key.keysym.sym = SDLK_DOWN; /* down d-pad */
		break;
	case 6: event->key.keysym.sym = SDLK_1; /* record */
		break;
	case 7: event->key.keysym.sym = SDLK_2; /* play */
		break;
	case 8: event->key.keysym.sym = SDLK_3; /* fastforward */
		break;
	case 9: event->key.keysym.sym = SDLK_4; /* stop_eject */
		break;
	case 10: event->key.keysym.sym = SDLK_5; /* rewind */
		break;
	}
	return 1;
}

static void process_events(SDL_Window *window)
{
    SDL_Event event;
    struct button_coord_list bcl;
    struct sim_lcd_params slp;
    int w, h;

    while (SDL_PollEvent(&event) || buttonfuzzer(&event)) {
        switch (event.type) {
        case SDL_KEYDOWN:
            key_press_cb(&event.key.keysym);
	    must_redraw_window = 1;
            break;
        case SDL_KEYUP:
            key_release_cb(&event.key.keysym);
	    must_redraw_window = 1;
            break;
        case SDL_QUIT:
            /* Handle quit requests (like Ctrl-c). */
            time_to_quit = 1;
	    must_redraw_window = 1;
            break;
        case SDL_WINDOWEVENT:
            handle_window_event(window, event);
	    must_redraw_window = 1;
            break;
        case SDL_MOUSEBUTTONDOWN:
            slp = get_sim_lcd_params();
            if (slp.orientation == SIM_LCD_ORIENTATION_UNROTATED) {
                w = badge_image_width;
                h = badge_image_height;
            } else {
                w = rotated_badge_image_width;
                h = rotated_badge_image_height;
            }
            bcl = get_button_coords(&slp, w, h);
            mouse_button_down_cb(&event.button, &bcl);
            sensor_ui_mouse_input(window, &event.button);
	    must_redraw_window = 1;
            break;
        case SDL_MOUSEBUTTONUP:
            mouse_button_up_cb(&event.button);
	    must_redraw_window = 1;
            break;
        case SDL_MOUSEMOTION:
            if (!event.motion.state) /* button held? */
                break;
            /* Check if motion is inside orientation indicator */
            if (event.motion.x < BADGE_ORIENTATION_X - 65)
                break;
            if (event.motion.x > BADGE_ORIENTATION_X + 65)
                break;
            if (event.motion.y < BADGE_ORIENTATION_Y - 65)
                break;
            if (event.motion.y > BADGE_ORIENTATION_Y + 65)
                break;
#if HAVE_ACCELEROMETER
            /* We have mouse motion with button held, inside the orientation indicator... */
            float vector_len;
            if (event.motion.state & SDL_BUTTON_RMASK)
                vector_len = 150.0f; /* fine control */
            else if (event.motion.state & SDL_BUTTON_MMASK)
                vector_len = 75.0f; /* medium control */
            else
                vector_len = 25.0f; /* coarse control */
	    union vec3 u, v;
            u.v.x = 0.0f;
            u.v.y = 0.0f;
            u.v.z = -vector_len;
            v.v.x = event.motion.xrel;
            v.v.y = event.motion.yrel;
            v.v.z = -vector_len;
	    union quat q, new_orientation;
            quat_from_u2v(&q, &u, &v);
	    quat_mul(&new_orientation, &q, &badge_orientation);
            quat_normalize_self(&new_orientation);
            badge_orientation = new_orientation;
	    must_redraw_window = 1;
#endif
            break;
        case SDL_MOUSEWHEEL:
            slp = get_sim_lcd_params();
            if (slp.orientation == SIM_LCD_ORIENTATION_UNROTATED) {
                w = badge_image_width;
                h = badge_image_height;
            } else {
                w = rotated_badge_image_width;
                h = rotated_badge_image_height;
            }
            bcl = get_button_coords(&slp, w, h);
            mouse_scroll_cb(&event.wheel, &bcl);
	    must_redraw_window = 1;
            break;
        case SDL_JOYAXISMOTION:
        case SDL_JOYBALLMOTION:
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
        case SDL_JOYHATMOTION:
            joystick_event_cb(window, event);
	    must_redraw_window = 1;
            break;
        }
    }
}

static void wait_until_next_frame(void)
{
    static uint32_t next_frame = 0;

    if (next_frame == 0)
        next_frame = SDL_GetTicks() + 33;  /* 30 Hz */
    uint32_t now = SDL_GetTicks();
    if (now < next_frame)
        SDL_Delay(next_frame - now);
    next_frame += 33; /* 30 Hz */
}

#ifdef __linux__
static void do_hot_restart(char *path)
{
	fprintf(stderr, "New executable detected, hot restarting!\n");
#if 0
	fprintf(stderr, "path = '%s'\n", path);
	for (int i = 0; saved_args[i] != NULL; i++)
		fprintf(stderr, "  argv[%d] = '%s'\n", i, saved_args[i]);
#endif
	(void) execvp(path, saved_args);
	/* we should not get here, if we're here, bad juju has happened */
	write(1, "Bad juju: exevp() failed\n", 25);
	raise(SIGTRAP); /* abort, or invoke debugger */
}
#endif

static void maybe_hot_restart(void)
{
	/* for now, hot restart only works on linux */
#ifdef __linux__

	char path[PATH_MAX];

	if (!hot_restart)
		return;

	static int counter = 0;

	counter++;
	if (counter > 100)
		counter = 0;

	if (counter != 0)
		return;

	/* Find our current executable file */
	ssize_t len = readlink("/proc/self/exe", path, sizeof(path));
	if (len < 0) {
		fprintf(stderr, "readlink(\"/proc/self/exe\"): %s\n", strerror(errno));
		return;
	}
	if (len < (long) sizeof(path))
		path[len] = '\0';

	/* Interestingly, if you recompile while the program is running, the
	 * /proc/self/exe symlink will contain the original pathname with
	 * " (deleted)" appended to it. WTF? Ok.
	 */
	if (len > 10) {
		char *x = &path[len - 10];
		if (strcmp(x, " (deleted)") == 0)
			*x = '\0'; /* Cut off the " (deleted)" to get the actual path. */
	}

	/* stat our executable to get its last modification time */
	static struct stat statbuf;
	int rc = stat(path, &statbuf);
	if (rc < 0) {
		fprintf(stderr, "stat(\"%s\"): %s\n", path, strerror(errno));
		return;
	}

	/* Is our executable newer than what we are currently running?  If so, hot restart */
	if (statbuf.st_mtim.tv_sec > sim_start_time.tv_sec) {
		do_hot_restart(path);
		__builtin_unreachable();
	}
#endif
}

void hal_start_sdl(UNUSED int *argc, UNUSED char ***argv)
{
    int first_time = 1, second_time = 0;

    program_title = strdup((*argv)[0]);
    if (start_sdl())
	exit(1);
    setup_window_and_renderer(&window, &renderer, &pix_buf, &landscape_pix_buf);
    flareled(0, 0, 0);

    init_sim_lcd_params();

    setup_sensor_uis();

    while (!time_to_quit) {
	if (second_time) {
            /* Not sure why I need to wait for the 2nd time for this to work. */
            simulator_zoom_ui(0.5);
	    second_time = 0;
	    if (fullscreen_flag && !fullscreen)
		toggle_fullscreen_mode();
            if (initial_zoom_count != 0) {
		if (initial_zoom_count > 0) {
			for (int i = 0; i < initial_zoom_count; i++) {
			    simulator_zoom_ui(1.1);
			}
		} else if (initial_zoom_count < 0) {
			for (int i = 0; i < -initial_zoom_count; i++) {
			    simulator_zoom_ui(0.9);
			}
		}
            }
	}
	sim_button_status_countdown(&must_redraw_window);
	draw_window(renderer, pix_buf, landscape_pix_buf);

	if (first_time) {
            int sx, sy;
            SDL_GetWindowSize(window, &sx, &sy);
            adjust_sim_lcd_params_defaults(sx, sy);
            set_sim_lcd_params_default();
            first_time = 0;
	    second_time = 1;
        }

	process_events(window);
	wait_until_next_frame();
	maybe_hot_restart();
    }
    free_sensor_ui();
    SDL_DestroyWindow(window);
    SDL_QuitSubSystem(SDL_INIT_EVENTS);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    free(badge_image_pixels);
    free(rotated_badge_image_pixels);
    SDL_Quit();


    printf("\n\n\n");
    printf("If you see leak sanitizer complaining about memory and _XlcDefaultMapModifiers\n");
    printf("it's because SDL is programmed by monkeys.\n");
    printf("\n\n\n");
    return;
}
