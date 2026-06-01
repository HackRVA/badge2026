/*********************************************

An implementation of Conway's Game of Life

 Author: Paul Chang <paulc1231@gmail.com>
 (c) 2021 Paul Chang
**********************************************/

#include <string.h>
#include <stdio.h>
#include "colors.h"
#include "badge.h"
#include "button.h"
#include "framebuffer.h"
#include "rtc.h"

#include "xorshift.h"

static unsigned int gen_count = 0;
static unsigned int max_gen = 100;
static volatile int last_time;

/* One cell per pixel, filling the whole screen */
#define GRID_W LCD_XSIZE
#define GRID_H LCD_YSIZE
#define GRID_SIZE (GRID_W * GRID_H)

#define ALIVE 1
#define DEAD 0

#define TRUE 1
#define FALSE 0

static struct Grid
{
	struct Cell
	{
		unsigned char alive;
	} cells[GRID_SIZE];
} grid, next_generation_grid;

/* Program states.  Initial state is GAME_OF_LIFE_INIT */
enum game_of_life_state_t
{
	GAME_OF_LIFE_INIT,
	GAME_OF_LIFE_SPLASH_SCREEN,
	GAME_OF_LIFE_RUN,
	GAME_OF_LIFE_RENDER,
	GAME_OF_LIFE_EXIT,
};

enum game_of_life_cmd_t
{
	CMD_PAUSE,
	CMD_RESUME
};

static enum game_of_life_state_t game_of_life_state = GAME_OF_LIFE_INIT;
static enum game_of_life_cmd_t game_of_life_cmd = CMD_RESUME;

static int is_in_range(int current_index)
{
	return current_index >= 0 && current_index < GRID_SIZE;
}

static int find_index(int x, int y)
{
	return y * GRID_W + x;
}

static int is_cell_alive(struct Grid *grid, int x, int y)
{
	if (x >= 0 && x < GRID_W && y >= 0 && y < GRID_H)
	{
		int index = find_index(x, y);

		if (is_in_range(index))
		{
			return grid->cells[index].alive == ALIVE;
		}

		return FALSE;
	}

	return FALSE;
}

static void next_generation(unsigned int alive_count, unsigned int cell, int current_index)
{
	if (cell == ALIVE)
	{
		if (alive_count == 2 || alive_count == 3)
		{
			next_generation_grid.cells[current_index].alive = ALIVE;
		}
		else
		{
			next_generation_grid.cells[current_index].alive = DEAD;
		}
	}
	else
	{
		if (alive_count == 3)
		{ // only way a dead cell can be revived if it has exactly 3 alive neighbors
			next_generation_grid.cells[current_index].alive = ALIVE;
		}
		else
		{
			next_generation_grid.cells[current_index].alive = DEAD;
		}
	}
}

static void update_current_generation_grid(void)
{
	grid = next_generation_grid;
}

static int get_cell_x_pos(int cell_index)
{
	return cell_index % GRID_W;
}

static int get_cell_y_pos(int cell_index)
{
	return cell_index / GRID_W;
}

static void figure_out_alive_cells(void)
{
	unsigned int alive_count = 0;
	// neighbors: LEFT, RIGHT, TOP, DOWN, TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT
	int neighbor_x_offset[8] = { -1, 1, 0, 0, -1, -1, 1, 1};
	int neighbor_y_offset[8] = { 0, 0, -1, 1, -1, 1, -1, 1};
	int n;

	for (n = 0; n < GRID_SIZE; n++)
	{
		int i, neighbor_x, neighbor_y;

		for (i = 0; i < 8; i++)
		{
			neighbor_x = get_cell_x_pos(n) + neighbor_x_offset[i];
			neighbor_y = get_cell_y_pos(n) + neighbor_y_offset[i];

			if (is_cell_alive(&grid, neighbor_x, neighbor_y))
			{
				alive_count++;
			}
		}

		next_generation(alive_count, grid.cells[n].alive, n);
		alive_count = 0;
	}

	update_current_generation_grid();
}

#define GEN_INTERVAL_MS 150

static int get_time(void)
{
	return (int) rtc_get_ms_since_boot();
}

static void move_to_next_gen_every_second(void)
{
	volatile int current_time = get_time();
	if ((current_time - last_time) >= GEN_INTERVAL_MS)
	{
		if (game_of_life_cmd == CMD_RESUME)
		{
			last_time = current_time;
			figure_out_alive_cells();
			gen_count++;
			return;
		}
		/* endgame */
		return;
	}
}

static void init_cells(void)
{
	int i;

	gen_count = 0;
	game_of_life_cmd = CMD_RESUME;

    int timestamp = (int)rtc_get_ms_since_boot();
	for (i = 0; i < GRID_SIZE; i++)
	{
		grid.cells[i].alive = xorshift((unsigned int *)&timestamp) % 2;
	}

	next_generation_grid = grid;

	last_time = get_time();
}

static void render_next_gen_text(unsigned int gen_count)
{
	char next_gen_text[13];
	char gen_num_text[4];
	FbColor(GREEN);
	FbMove(LCD_XSIZE / 4, LCD_YSIZE - 7);

	strcpy(next_gen_text, "NEXT GEN ");
	sprintf( gen_num_text, "%d", (int)gen_count);
	strcat(next_gen_text, gen_num_text);

	FbWriteLine(next_gen_text);
}

static void render_cell(int grid_x, int grid_y, int alive)
{
	if (!alive)
		return;

	FbPoint(grid_x, grid_y);
}

static void render_cells(void)
{
	int i;

	FbColor(WHITE);
	for (i = 0; i < GRID_SIZE; i++)
	{
		render_cell(get_cell_x_pos(i), get_cell_y_pos(i), grid.cells[i].alive);
	}
}

static void render_game(void)
{
	FbClear();

	render_cells();
	render_next_gen_text(gen_count);
	FbSwapBuffers();
}

static void render_end_game_screen(void)
{
	static int already_rendered = 0;

	if (!already_rendered) {
		FbClear();
		FbColor(GREEN);
		FbMove(20, 40);
		FbWriteString("Thank you\nfor playing!\n\n\nPress B button\nto exit");
		FbSwapBuffers();
		already_rendered = 1;
	}

	int down_latches = button_down_latches();
	if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)
#if BADGE_HAS_ROTARY_SWITCHES
		|| BUTTON_PRESSED(BADGE_BUTTON_ENCODER_2_SW, down_latches)
#endif
		)
	{
		game_of_life_state = GAME_OF_LIFE_EXIT;
		already_rendered = 0;
	}
}

static void game_of_life_render(void)
{
	static unsigned int prev_gen_count = (unsigned int) -1;

	if (prev_gen_count == gen_count) {
		game_of_life_state = GAME_OF_LIFE_RUN;
		return;
	}

	prev_gen_count = gen_count;
	render_game();

	game_of_life_state = GAME_OF_LIFE_RUN;
}

static void check_buttons(void)
{
    int down_latches = button_down_latches();

	if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)
#if BADGE_HAS_ROTARY_SWITCHES
		 || BUTTON_PRESSED(BADGE_BUTTON_ENCODER_2_SW, down_latches)
#endif
		)
	{
		game_of_life_state = GAME_OF_LIFE_EXIT;
		return;
	}
	if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches))
	{
		game_of_life_cmd = CMD_RESUME;
		return;
	}
	if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches))
	{
		game_of_life_cmd = CMD_PAUSE;
		return;
	}
	if (gen_count == max_gen)
	{
		game_of_life_cmd = CMD_PAUSE;
		render_end_game_screen();
		return;
	}
	game_of_life_state = GAME_OF_LIFE_RENDER;
}

static void render_splash_screen(void)
{
	FbColor(GREEN);
	FbMove(10, 30);
	FbWriteString("Game of Life\n\nPress A\nTo Start\n\nPress B\nto exit");
	FbSwapBuffers();
}

static void game_of_life_init(void)
{
	FbInit();
	FbClear();

	game_of_life_state = GAME_OF_LIFE_SPLASH_SCREEN;
}

static void game_of_life_splash_screen(void)
{
	static char already_rendered_splash_screen = 0;
    int down_latches = button_down_latches();

	if (!already_rendered_splash_screen) {
		render_splash_screen();
		already_rendered_splash_screen = 1;
	}

	if (
#if BADGE_HAS_ROTARY_SWITCHES
	BUTTON_PRESSED(BADGE_BUTTON_ENCODER_SW, down_latches) ||
#endif
		BUTTON_PRESSED(BADGE_BUTTON_A, down_latches))
	{
		already_rendered_splash_screen = 0;
		init_cells();
		game_of_life_state = GAME_OF_LIFE_RUN;
	}
	else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)
#if BADGE_HAS_ROTARY_SWITCHES
		 || BUTTON_PRESSED(BADGE_BUTTON_ENCODER_2_SW, down_latches)
#endif
		)
	{
		already_rendered_splash_screen = 0;
		game_of_life_state = GAME_OF_LIFE_EXIT;
	}
}

static void game_of_life_run(void)
{
	move_to_next_gen_every_second();
	check_buttons();
}

static void game_of_life_exit(void)
{
	game_of_life_state = GAME_OF_LIFE_INIT; /* So that when we start again, we do not immediately exit */
	pop_app();
}

void game_of_life_cb(__attribute__((unused)) struct badge_app *app)
{
	switch (game_of_life_state)
	{
	case GAME_OF_LIFE_INIT:
		game_of_life_init();
		break;
	case GAME_OF_LIFE_SPLASH_SCREEN:
		game_of_life_splash_screen();
		break;
	case GAME_OF_LIFE_RUN:
		game_of_life_run();
		break;
	case GAME_OF_LIFE_RENDER:
		game_of_life_render();
		break;
	case GAME_OF_LIFE_EXIT:
		game_of_life_exit();
		break;
	default:
		break;
	}
}


