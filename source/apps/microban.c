#include<stdio.h>
#include "colors.h"
#include "menu.h"
#include "button.h"
#include "framebuffer.h"
#include "badge.h"
#include "microban_assets/microban_assets.h"

#define TILE_SIZE 16

enum microban_state_t {
    MICROBAN_INIT,
    MICROBAN_RUN,
    MICROBAN_EXIT,
};

enum microban_state_run {
    GAMEPLAY,
    PAUSE,
    WIN
};

typedef enum Tile {
    VOID,
    FLOOR,
    WALL,
    TARGET,
    PLAYER_ON_TARGET,
    PLAYER_TILE,
    BLOCK,
    BLOCK_ON_TARGET,
} Tile;

typedef enum Cardinal {
    N,S,E,W
} Cardinal;

typedef struct Point {
    int x, y;
} Point;

typedef struct Camera {
    Point offset;
    Point target;
} Camera;

typedef struct Input {
    bool upPressed;
    bool downPressed;
    bool leftPressed;
    bool rightPressed;
    bool upHeld;
    bool downHeld;
    bool leftHeld;
    bool rightHeld;
    bool APressed;
    bool BPressed;
    bool AHeld;
    bool BHeld;
} Input;

typedef struct Player {
    Point position;
    Tile current_tile;
    Cardinal facing;
} Player;



static enum microban_state_t microban_state = MICROBAN_INIT;
static enum microban_state_run run_state = GAMEPLAY;
static int screen_changed = 0;

static int tick;
static int level_number;
static int level_width() {
    return microban_levels_rects[level_number].width;
}
static int level_height() {
    return microban_levels_rects[level_number].height;
}

static int menu_selection = 0;

static Camera camera;
static Player player;
static int MAX_LEVELS = 156;
static const Input input_clear = {0};
static Input input = input_clear;


static uint8_t current_level_data[32768];
static struct asset2 current_level = {
    .type = PICTURE8BIT,
    .seqNum = 1,
    .x = 16,
    .y = 16,
    .colormap = (const uint16_t *) microban_levels_colormap,
    .pixel = (const unsigned char *) current_level_data,
};

int wrapIndex(int i, int i_max) {
   return ((i % i_max) + i_max) % i_max;
}

static Tile get_tile(Point coord) {
    if (coord.x < 0 || coord.x >= level_width() || coord.y < 0 || coord.y >= level_height()) {
        return VOID;
    }
    uint8_t pixdata = current_level.pixel[coord.y * level_width() + coord.x];
    return (Tile)pixdata;
}

static bool tile_valid(Point coord) {
    if (coord.x < 0 || coord.x >= level_width() || coord.y < 0 || coord.y >= level_height()) {
        return false;
    }
    return true;
}

static void set_tile(Point coord, Tile tile) {
    current_level_data[coord.y*level_width() + coord.x] = tile;
}

static void set_level(int levelNo) {
    const struct asset2 *asset = &microban_levels;
    int y, x, texture_row, texture_x, buffer_row;

    player.position = (Point){0,0};
    input = input_clear;



    Rectangle level_rect = microban_levels_rects[levelNo];
    current_level.x = level_rect.width;
    current_level.y = level_rect.height;

    bool player_found = false;

    for (y = 0; y < level_rect.height; y++) {
        texture_row = (y + level_rect.y) * asset->x;
        buffer_row = y * level_rect.width;
        for (x = 0; x < level_rect.width; x++) {
            texture_x = (x + level_rect.x);
            uint8_t pixdata = asset->pixel[texture_row + texture_x];
            unsigned char pixel = (Tile)pixdata;
            current_level_data[buffer_row + x] = pixel;

            if (!player_found) {
                if (pixel == PLAYER_TILE) {
                    player.position = (Point){x, y};
                    set_tile(player.position, FLOOR);
                    player_found = true;
                } else if (pixel == PLAYER_ON_TARGET) {
                    player.position = (Point){x, y};
                    set_tile(player.position, TARGET);
                    player_found = true;
                }
            }

        }
    }
}

static bool check_level() {
    // if (level_number == 0) return false;
    int blocks = 0;
    int targets = 0;
    int blocks_on_targets = 0;
    for (int i = 0; i < level_width()*level_height(); i++) {
        Tile tile = current_level_data[i];
        if (tile == BLOCK) blocks++;
        if (tile == BLOCK_ON_TARGET) blocks_on_targets++;
        if (tile == TARGET) targets++;
    }
    if (targets == 0 && blocks == 0 && blocks_on_targets > 0) {
        return true;
    }
    return false;
}

static void microban_init(void)
{
    FbInit();
    FbClear();
    input = input_clear;
    microban_state = MICROBAN_RUN;
    run_state = GAMEPLAY;
    screen_changed = 1;
    player.position = (Point){0,0};
    tick = 0;
    level_number = 1;
    set_level(level_number);
    camera.offset.x = LCD_XSIZE / 2 - TILE_SIZE/2;
    camera.offset.y = LCD_YSIZE / 2 - TILE_SIZE/2;
}

static void process_input_WIN() {
    if (input.APressed || input.BPressed) {
        level_number = wrapIndex(level_number + 1, MAX_LEVELS);
        set_level(level_number);
        run_state = GAMEPLAY;
    }
}

static void process_input_PAUSE() {
    if (input.downPressed) {
        menu_selection = wrapIndex(menu_selection + 1, 2);
    } else if (input.upPressed) {
        menu_selection = wrapIndex(menu_selection - 1, 2);
    } else if (input.APressed) {
        if (menu_selection == 0) {
            set_level(level_number);
            run_state = GAMEPLAY;
            menu_selection = 0;
        } else if (menu_selection == 1) {
            microban_state = MICROBAN_EXIT;
            menu_selection = 0;
        }
    } else if (input.BPressed) {
        run_state = GAMEPLAY;
    }
}

static void process_input_GAMEPLAY() {
    bool move_tried = (input.upPressed || input.rightPressed || input.downPressed || input.leftPressed);
    Point new_position = player.position;

    if (input.upHeld) {
    }
    if (input.rightHeld) {
    }
    if (input.downHeld) {
    }
    if (input.leftHeld) {
    }
    if (input.AHeld) {
    }
    if (input.BHeld) {
    }

    if (input.APressed) {
    }
    if (input.BPressed) {
        run_state = PAUSE;
        menu_selection = 0;
    }

    if (input.upPressed) {
        new_position.y -= 1;
        player.facing = N;
    } else if (input.rightPressed) {
        new_position.x += 1;
        player.facing = E;
    } else if (input.downPressed) {
        new_position.y += 1;
        player.facing = S;
    } else if (input.leftPressed) {
        new_position.x -= 1;
        player.facing = W;
    }

    if (move_tried) {
        Tile new_position_tile = get_tile(new_position);
        if (new_position_tile == VOID || new_position_tile == FLOOR || new_position_tile == TARGET) {
            player.position = new_position;
        }
        if (new_position_tile == BLOCK || new_position_tile == BLOCK_ON_TARGET) {
            Point block_pos;
            Tile block_pos_tile;
            switch (player.facing) {
            case N:
                block_pos = (Point){new_position.x, new_position.y - 1};
                break;
            case E:
                block_pos = (Point){new_position.x + 1, new_position.y};
                break;
            case S:
                block_pos = (Point){new_position.x, new_position.y + 1};
                break;
            case W:
                block_pos = (Point){new_position.x - 1, new_position.y};
                break;
            }
            block_pos_tile = get_tile(block_pos);
            bool valid = tile_valid(block_pos);
            if (valid && (block_pos_tile == VOID || block_pos_tile == FLOOR || block_pos_tile == TARGET)) {
                if (new_position_tile == BLOCK) {
                    set_tile(new_position, FLOOR);
                } else if (new_position_tile == BLOCK_ON_TARGET) {
                    set_tile(new_position, TARGET);
                }
                if (block_pos_tile == VOID || block_pos_tile == FLOOR) {
                    set_tile(block_pos, BLOCK);
                } else if (block_pos_tile == TARGET) {
                    set_tile(block_pos, BLOCK_ON_TARGET);
                }
                player.position = new_position;
            }
        }

        if (check_level()) {
            run_state = WIN;
        }
    }
}



static void check_buttons(void)
{
    int down_latches = button_down_latches();
    int mask = button_mask();

    input.upPressed =   (BUTTON_PRESSED(BADGE_BUTTON_UP,    down_latches));
    input.downPressed =     (BUTTON_PRESSED(BADGE_BUTTON_DOWN,  down_latches));
    input.leftPressed =     (BUTTON_PRESSED(BADGE_BUTTON_LEFT,  down_latches));
    input.rightPressed = (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches));
    input.upHeld =      (BUTTON_PRESSED(BADGE_BUTTON_UP,    mask));
    input.downHeld =    (BUTTON_PRESSED(BADGE_BUTTON_DOWN,  mask));
    input.leftHeld =    (BUTTON_PRESSED(BADGE_BUTTON_LEFT,  mask));
    input.rightHeld =   (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, mask));
    input.APressed =    (BUTTON_PRESSED(BADGE_BUTTON_A,     down_latches));
    input.BPressed =    (BUTTON_PRESSED(BADGE_BUTTON_B,     down_latches));
    input.AHeld =       (BUTTON_PRESSED(BADGE_BUTTON_A,     mask));
    input.BHeld =       (BUTTON_PRESSED(BADGE_BUTTON_B,     mask));
}

void draw_level(const struct asset2 *asset, Camera camera) {
    int y, x, texture_row, texture_x, tile_x, tile_y, camera_x, camera_y;
    
    // unsigned char *pixdata;
    // unsigned char pixel;
    

    for (y = 0; y < level_height(); y++) {
        texture_row = (y % asset->y) * asset->x;
        for (x = 0; x < level_width(); x++) {
            texture_x = x % asset->x; //factor seqNum here
            // pixdata = (unsigned short*) &(asset->pixel16[texture_row + texture_x]);
            uint8_t pixdata = asset->pixel[texture_row + texture_x];
            Tile pixel = (Tile)pixdata; /* 1 pixel per 2 bytes */
            switch(pixel) {
            case VOID:
                continue;
            case FLOOR:
                tile_x = 0; tile_y = 0;
                break;
            case PLAYER_TILE: //void, floor, player
                tile_x = TILE_SIZE; tile_y = 0;
                break;
                // continue;
            case WALL: //wall
                tile_x = TILE_SIZE; tile_y = TILE_SIZE * 3;
                break;
            case TARGET:
                tile_x = 0; tile_y = TILE_SIZE;
                break;
            case PLAYER_ON_TARGET: //target, player_on_target
                tile_x = TILE_SIZE; tile_y = TILE_SIZE;
                break;
            case BLOCK: //block
                tile_x = TILE_SIZE * 2; tile_y = 0;
                break;
            case BLOCK_ON_TARGET: //block on target
                tile_x = TILE_SIZE * 2; tile_y = TILE_SIZE;
                break;
            }

            camera_x =  -camera.target.x + x*TILE_SIZE + camera.offset.x;
            camera_y =  -camera.target.y + y*TILE_SIZE + camera.offset.y;

            FbImageRect(&chip_16px, camera_x, camera_y, tile_x, tile_y, TILE_SIZE, TILE_SIZE, MAGENTA);
        }
    }
}



static void draw_coord(void)
{
    char buf[20];
    FbColor(WHITE);
    FbMove(2, 2);
    snprintf(buf, sizeof(buf), "%d,%d", player.position.x, player.position.y);
    FbWriteString(buf);
}



static void draw_screen(void)
{
    if (!screen_changed)
        return;

    // FbImagePlace(&eb_bg, -player.position.x/2, 0, MAGENTA);
    FbImageRect(&redblocks, 0, 0, player.position.x, player.position.y, LCD_XSIZE, LCD_YSIZE, MAGENTA);
    
    camera.target = (Point){player.position.x * TILE_SIZE, player.position.y * TILE_SIZE};
    player.current_tile = get_tile(player.position);

    draw_level(&current_level, camera);
    int camera_x =  -camera.target.x + player.position.x*TILE_SIZE + camera.offset.x;
    int camera_y =  -camera.target.y + player.position.y*TILE_SIZE + camera.offset.y;
    int ptex_x, ptex_y;
    switch (player.facing) {
    case N:
        ptex_x = 0*TILE_SIZE; ptex_y = 4*TILE_SIZE;
        break;
    case E:
        ptex_x = 1*TILE_SIZE; ptex_y = 4*TILE_SIZE;
        break;
    case S:
        ptex_x = 2*TILE_SIZE; ptex_y = 4*TILE_SIZE;
        break;
    case W:
        ptex_x = 3*TILE_SIZE; ptex_y = 4*TILE_SIZE;
        break;
    }
    if (player.current_tile == TARGET) {
        ptex_y += TILE_SIZE;
    }

    FbImageRect(&chip_16px, camera_x, camera_y, ptex_x, ptex_y, TILE_SIZE, TILE_SIZE, MAGENTA);

    switch (run_state) {
    case GAMEPLAY:
        draw_coord();
        break;
    case WIN:
        FbColor(YELLOW);
        FbMove(8, 16);
        FbWriteString("NICE CLEAR!!");
        FbMove(8, 28);
        FbWriteString("try next?");
        break;
    case PAUSE:
        FbMove(8, 16 + 12*menu_selection);
        FbWriteString(">");
        FbMove(16, 16);
        FbWriteString("reset");
        FbMove(16, 28);
        FbWriteString("exit");

        break;
    }
    
    FbSwapBuffers();
    screen_changed = 1;
}



static void microban_run(void)
{   
    tick += 1;
    check_buttons();
    switch (run_state) {
    case GAMEPLAY:
        process_input_GAMEPLAY();
        break;
    case PAUSE:
        process_input_PAUSE();
        break;
    case WIN:
        process_input_WIN();
        break;
    }
    draw_screen();
}


static void microban_exit(void)
{
    microban_state = MICROBAN_INIT; /* So that when we start again, we do not immediately exit */
    pop_app();
}

void microban_cb(__attribute__((unused)) struct badge_app *app)
{
    switch (microban_state) {
    case MICROBAN_INIT:
        microban_init();
        break;
    case MICROBAN_RUN:
        microban_run();
        break;
    case MICROBAN_EXIT:
        microban_exit();
        break;
    default:
        break;
    }
}

