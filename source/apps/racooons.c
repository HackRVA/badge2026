#include<stdio.h>
#include<string.h>
#include "framebuffer.h" // "Hey, Artist! You draw stuff."
#include "colors.h"      // "Bring the crayons."
#include "button.h"      // "Pilot, report button presses!"
#include "random.h"      // "Randomizer, give me chance!"
#include "xorshift.h"    // "Math guy, help with randomness."
#include "badge.h"       // "Mission control systems."
#include "racooons.h"    // "Me, myself, and my own header file."

#define MAX_ITEMS 10
#define PLAYER_WIDTH 8
#define PLAYER_HEIGHT 8
#define ITEM_WIDTH 8
#define ITEM_HEIGHT 8
#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 128



enum game_state {
	STATE_MENU,
	STATE_INIT,
    STATE_RUN,
	STATE_GAME_OVER,
    STATE_EXIT
};

typedef struct {
	int x;
    int y;
} Position;

typedef struct {
	Position pos;
    int speed;
	bool caught;
    bool active;
    bool good;  // true = catch, false = avoid
} FallingItem;

static Position player = {80, 110};  // Center bottom
static FallingItem items[MAX_ITEMS];
static int tick = 0;
static int score = 0;
static int lives = 5;
static bool alive = true;
static int player_flash_timer = 0;
static bool last_catch_good = true;
static enum game_state current_state = STATE_MENU;

static int random_num(int n) {
	static unsigned int state = 0;
	int x;

	if (state == 0)
		random_insecure_bytes((uint8_t *) &state, sizeof(state));
	x = xorshift(&state);
	if (x < 0) x = -x;
	return x % n;
}

int getRandom(int min, int max) {
	return min + random_num(max - min + 1);
}

void spawn_item(void) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (!items[i].active) {
            items[i].pos.x = getRandom(0, SCREEN_WIDTH - ITEM_WIDTH);
            items[i].pos.y = 0;
            items[i].speed = getRandom(1, 3);
            items[i].good = getRandom(0, 1);
            items[i].active = true;
			items[i].caught = false;
            break;
        }
    }
}


void update_items(void) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (items[i].active) {
            items[i].pos.y += items[i].speed;
            if (items[i].pos.y > SCREEN_HEIGHT) {
                items[i].active = false; // reset if off-screen
            }
        }
    }
}

void check_collisions(void) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (!items[i].active) continue;

        bool caught = (
            items[i].pos.y + ITEM_WIDTH >= player.y &&
            items[i].pos.y <= player.y + ITEM_WIDTH &&
            items[i].pos.x + ITEM_WIDTH >= player.x &&
            items[i].pos.x <= player.x + PLAYER_WIDTH
        );

        if (caught) {
			items[i].caught = true;
            items[i].active = false;

            if (items[i].good) {
				score += 1;
				last_catch_good = true;
				player_flash_timer = 5;
            }

			if (!items[i].good) {
				lives -= 1;
				last_catch_good = false;
				player_flash_timer = 5;
			
				if (lives <= 0) {
					current_state = STATE_GAME_OVER;
				}
        	}
    	}
	}
}

void draw_game(void) {
    FbInit();

    // Clear background
    FbColor(BLACK);
    FbMove(0, 0);
    FbFilledRectangle(SCREEN_WIDTH, SCREEN_HEIGHT);
	
	// Draw score
	FbColor(YELLOW);
	FbMove(5, 5);
	char buf[20];
	snprintf(buf, sizeof(buf), "Score: %d", score);
	FbWriteString(buf);

	// Lives display using red circles
	FbMove(SCREEN_WIDTH - 50, 5);
	FbColor(WHITE);
	FbWriteString("Lives:");

	for (int i = 0; i < 5; i++) {
		FbColor(i < lives ? RED : WHITE);
		FbMove(SCREEN_WIDTH - 40 + i * 10, 15);
		FbFilledRectangle(6, 6); 
	}
	
	// Draw player
	if (player_flash_timer > 0) {
	    FbColor(last_catch_good ? GREEN : RED);
	    FbMove(player.x - 2, player.y - 2);  // slightly bigger
	    FbFilledRectangle(PLAYER_WIDTH + 4, PLAYER_HEIGHT + 4);
	    player_flash_timer--;  // tick down the timer
	} else {
	    FbColor(WHITE);
	    FbMove(player.x, player.y);
	    FbFilledRectangle(PLAYER_WIDTH, PLAYER_HEIGHT);
	}

    // Draw falling items
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (items[i].active) {
            FbColor(items[i].good ? GREEN : RED);
            FbMove(items[i].pos.x, items[i].pos.y);
            FbFilledRectangle(ITEM_WIDTH, ITEM_HEIGHT);
        }
    }


    FbSwapBuffers();
}


void handle_input(void) {
    int btns = button_mask();

    if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, btns)) {
        player.x -= 3;
        if (player.x < 0) player.x = 0;
    }
    if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, btns)) {
        player.x += 3;
        if (player.x > SCREEN_WIDTH - PLAYER_WIDTH)
            player.x = SCREEN_WIDTH - PLAYER_WIDTH;
    }
	if (BUTTON_PRESSED(BADGE_BUTTON_B, btns)) {
		current_state = STATE_MENU;

	}
}

void update_game(void) {
    tick++;

    if (tick % 15 == 0) {
        spawn_item();
    }

    handle_input();
    update_items();
    check_collisions();
    draw_game();
}

void draw_menu(void) {
	FbInit();
    FbColor(BLACK);
    FbMove(0, 0);
    FbFilledRectangle(SCREEN_WIDTH, SCREEN_HEIGHT);

    FbColor(YELLOW);
    FbMove(20, 30);
    FbWriteString("RACOONS");

    FbMove(20, 60);
    FbWriteString("A = Start");

    FbMove(20, 70);
    FbWriteString("B = Quit");

    FbSwapBuffers();

}

void handle_menu_input(void) {
    int btns = button_down_latches();

    if (BUTTON_PRESSED(BADGE_BUTTON_A, btns)) {
        current_state = STATE_INIT;
    }

    if (BUTTON_PRESSED(BADGE_BUTTON_B, btns)) {
        current_state = STATE_EXIT;
    }
}

void draw_game_over(void) {
    FbInit();
    FbColor(BLACK);
    FbMove(0, 0);
    FbFilledRectangle(SCREEN_WIDTH, SCREEN_HEIGHT);

    FbColor(RED);
    FbMove(40, 50);
    FbWriteString("GAME OVER");

    FbColor(YELLOW);
    FbMove(30, 70);
    FbWriteString("A: Try Again");
    FbMove(30, 80);
    FbWriteString("B: Quit");

    FbSwapBuffers();
}

void handle_game_over_input(void) {
    int btns = button_down_latches();

    if (BUTTON_PRESSED(BADGE_BUTTON_A, btns)) {
        current_state = STATE_INIT;
        lives = 3;
        score = 0;
    }
    if (BUTTON_PRESSED(BADGE_BUTTON_B, btns)) {
        current_state = STATE_EXIT;
    }
}


void racooons_cb(__attribute__((unused)) struct badge_app *app) {
    switch (current_state) {
		case STATE_MENU:
			draw_menu();
			handle_menu_input();
			break;

        case STATE_INIT:
            for (int i = 0; i < MAX_ITEMS; i++) items[i].active = false;
            current_state = STATE_RUN;
            break;

        case STATE_RUN:
            update_game();
            break;
		
		case STATE_GAME_OVER:
			draw_game_over();
			handle_game_over_input();
			break;

        case STATE_EXIT:
            pop_app(); 
            break;
    }
}
