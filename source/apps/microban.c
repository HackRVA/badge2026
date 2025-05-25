#include<stdio.h>
#include<string.h>
#include "colors.h"
#include "utils.h"
#include "menu.h"
#include "ui.h"
#include "button.h"
#include "framebuffer.h"
#include "badge.h"
#include "key_value_storage.h"
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
    WIN,
    LEVEL_MENU,
    ABOUT,
};

#define MENU_ENTRIES 4
enum microcan_menu {
    MENU_RESET, /*MENU_SKIP,*/ MENU_LEVEL_MENU, MENU_ABOUT, MENU_EXIT
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
    bool pushing;
} Player;



static enum microban_state_t microban_state = MICROBAN_INIT;
static enum microban_state_run run_state = GAMEPLAY;
static int screen_changed = 0;

static int tick;
// static int stats.level_number;


static int menu_selection = 0;
static int menu_selection_level = 1;
static bool menu_streak_popup = false;

static Camera camera;
static Player player;
static const Input input_clear = {0};
static Input input = input_clear;

// static int streak = 0;
static int moves = 0;
static int moves_tried = 0;

static bool DrawHud = false;

static struct Stats {
    bool levels_completed[MAX_LEVELS];
    bool levels_unlocked[MAX_LEVELS];
    int  best_moves[MAX_LEVELS];
    int  streak;
    int  best_streak;
    int  level_number;
} stats;

static bool levelmenufromcompleted;

static int level_width(void) {
    return microban_levels_rects[stats.level_number].width;
}
static int level_height(void) {
    return microban_levels_rects[stats.level_number].height;
}

static void microban_save_game(void)
{
    // struct badgey_state state;
    // badgey_serialize_state(&state);
    // stats.level_number = stats.level_number;
    bool saved = flash_kv_store_binary("MICROBAN_SAVED_GAME", &stats, sizeof(stats));
    if (!saved) {
#if TARGET_SIMULATOR
        fprintf(stderr, "Failed to save game.\n");
#endif
        // set_badgey_state(BADGEY_INITIAL_MENU);
        // status_message("Failed to save\ngame\n");
        return;
    } else {
        #if TARGET_SIMULATOR
            fprintf(stderr,"SAVED THE MICROBAN\n");
        #endif
    }

    // set_badgey_state(BADGEY_INITIAL_MENU);
}




static void microban_restore_game(void)
{
    struct Stats state;

    memset(&state, 0, sizeof(state));

    bool ok = flash_kv_get_binary("MICROBAN_SAVED_GAME", &state, sizeof(state));
    if (!ok) {
#if TARGET_SIMULATOR
        // fprintf(stderr, "Failed to read MICROBAN_SAVED_GAME: %s\n", strerror(errno));
#endif
        // set_badgey_state(BADGEY_INITIAL_MENU);
        // status_message("Failed to read\nsaved game\n");
        return;
    } else {
        stats = state;
    }
}


uint16_t nice_clear_cycle[255];

#define BREAKPOINT_COUNT 7
static const int PROGRESS_BREAKPOINTS[BREAKPOINT_COUNT] = {1, 30, 60, 90, 120, 150, MAX_LEVELS};
static const int LEVELS_TO_PROGRESS = 10;

static bool unlock_levels(void) {
    for (int bp = 0; bp < BREAKPOINT_COUNT - 2; bp++) {
        int count = 0;
        bool requirement_met = false;
        for (int lvl = PROGRESS_BREAKPOINTS[bp]; lvl < PROGRESS_BREAKPOINTS[bp + 1]; lvl++) {
            if (stats.levels_completed[lvl] == true) count++;
        }
        if (count >= LEVELS_TO_PROGRESS) requirement_met = true;
        if (requirement_met) {
            for (int lvl = PROGRESS_BREAKPOINTS[bp + 1]; lvl < PROGRESS_BREAKPOINTS[bp + 2]; lvl++) {
                stats.levels_unlocked[lvl] = true;
            }
        }
    }
    return false;
}

static uint8_t current_level_data[32768]; //huge only for secret level, every other level could be 1024, I think.
static struct asset2 current_level = {
    .type = PICTURE8BIT,
    .seqNum = 1,
    .x = 16,
    .y = 16,
    .colormap = (const uint16_t *) microban_levels_colormap,
    .pixel = (const unsigned char *) current_level_data,
};



static int wrap(int i, int i_max) {
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

    levelmenufromcompleted = false;

    player.position = (Point){0,0};
    player.pushing = false;
    input = input_clear;

    moves = 0;
    moves_tried = 0;

    menu_selection_level = stats.level_number;

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

static bool check_level(void) {
    // if (stats.level_number == 0) return false;
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
    player.facing = S;
    tick = 0;
    stats.level_number = 0;
    stats.streak = 0;
    camera.offset.x = LCD_XSIZE / 2 - TILE_SIZE/2;
    camera.offset.y = LCD_YSIZE / 2 - TILE_SIZE/2;
    FbFont(FONT_SERIF);

    //unlock levels 0-29. (level 0 should be unlocked even though you start from 1)
    for (int lvl = 0; lvl < MAX_LEVELS; lvl += 1) {
        stats.levels_completed[lvl] = false;
        stats.best_moves[lvl] = 0;
        if (lvl < PROGRESS_BREAKPOINTS[1]) {
            stats.levels_unlocked[lvl] = true;
        }
    }

    microban_restore_game();

    for (int lvl = 1; lvl < MAX_LEVELS; lvl++) {
        if (stats.levels_unlocked[lvl] && !stats.levels_completed[lvl]) {
            stats.level_number = lvl;
            set_level(stats.level_number);
            run_state = GAMEPLAY;
            break;
        }
    }

    FbPaletteCycleInit(nice_clear_cycle, NICE_CLEAR_colormap, ARRAY_SIZE(NICE_CLEAR_colormap));

}

static void process_input_WIN(void) {
    if (input.APressed || input.BPressed) {
        // stats.level_number = wrap(stats.level_number + 1, MAX_LEVELS);
        // set_level(stats.level_number);
        // run_state = GAMEPLAY;
        int nextlvl = wrap(stats.level_number + 1, MAX_LEVELS);
        if (stats.levels_unlocked[nextlvl] == true) {
            stats.level_number = nextlvl;
            set_level(stats.level_number);
            run_state = GAMEPLAY;
        } else {
            run_state = LEVEL_MENU;
            levelmenufromcompleted = true;
        }
    }
}

static void process_input_LEVEL_MENU(void) {
    if (input.BPressed) {
        if (menu_streak_popup) {
            menu_streak_popup = false;
        }
        else if(!levelmenufromcompleted) {
            menu_selection_level = stats.level_number;
            run_state = GAMEPLAY;
        }
    } else if (input.APressed && stats.levels_unlocked[menu_selection_level] == true) {
        if (menu_selection_level == stats.level_number) {
            if (!levelmenufromcompleted) run_state = GAMEPLAY;
        } else {
            if (stats.streak >= 3 && !menu_streak_popup) {
                menu_streak_popup = true;
            } else {
                stats.streak = 0;
                menu_streak_popup = false;
                stats.level_number = menu_selection_level;
                set_level(stats.level_number);
                run_state = GAMEPLAY;
            }
        }
    } else if (input.downPressed && !menu_streak_popup) {
        menu_selection_level += 1;
        if (menu_selection_level > MAX_LEVELS - 1) menu_selection_level = MAX_LEVELS - 1;
    } else if (input.upPressed && !menu_streak_popup) {
        menu_selection_level -= 1;
        if (menu_selection_level < 1) menu_selection_level = 1;
    }
}

static void process_input_PAUSE(void) {
    if (input.downPressed) {
        menu_selection = wrap(menu_selection + 1, MENU_ENTRIES);
    } else if (input.upPressed) {
        menu_selection = wrap(menu_selection - 1, MENU_ENTRIES);
    } else if (input.APressed) {
        if (menu_selection == MENU_RESET) {
            set_level(stats.level_number);
            if (moves > 0) stats.streak = 0;
            run_state = GAMEPLAY;
        }
        // if (menu_selection == MENU_SKIP) {
        //     int nextlvl = wrap(stats.level_number + 1, MAX_LEVELS);
        //     if (stats.levels_unlocked[nextlvl] == true) {
        //         stats.level_number = nextlvl;
        //         set_level(stats.level_number);
        //         stats.streak = 0;
        //         run_state = GAMEPLAY;
        //     } else {
        //         run_state = LEVEL_MENU;
        //         // levelmenufromcompleted = true;
        //     }

        // }
        if (menu_selection == MENU_LEVEL_MENU) {
            run_state = LEVEL_MENU;
        }
        if (menu_selection == MENU_ABOUT) {
            run_state = ABOUT;
        }
        if (menu_selection == MENU_EXIT) {
            if (moves > 0) stats.streak = 0;
            microban_state = MICROBAN_EXIT;
        }
        if (menu_selection != MENU_ABOUT) menu_selection = 0;

    } else if (input.BPressed) {
        run_state = GAMEPLAY;
    }
}

static void process_input_GAMEPLAY(void) {
    bool move_tried = (input.upPressed || input.rightPressed || input.downPressed || input.leftPressed);
    bool moved = false;
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
        DrawHud = !DrawHud;
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
        moves_tried += 1;
        Tile new_position_tile = get_tile(new_position);
        if (new_position_tile == BLOCK || new_position_tile == BLOCK_ON_TARGET|| new_position_tile == WALL) {
            player.pushing = true;
        }
        if (new_position_tile == VOID || new_position_tile == FLOOR || new_position_tile == TARGET) {
            player.position = new_position;
            player.pushing = false;
            moved = true;
        } else if (new_position_tile == BLOCK || new_position_tile == BLOCK_ON_TARGET) {
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
                moved = true;
            }
        }

        if (moved) {
            moves += 1;
        }

        bool level_complete = check_level();
        if (level_complete) {
            stats.streak += 1;
            stats.levels_completed[stats.level_number] = true;
            int prev_best_moves = stats.best_moves[stats.level_number];
            if (moves < prev_best_moves || prev_best_moves == 0) stats.best_moves[stats.level_number] = moves;
            if (stats.streak > stats.best_streak) stats.best_streak = stats.streak;
            unlock_levels();
            run_state = WIN;

            microban_save_game();
        }
    }
}



static void check_buttons(void)
{
    int down_latches = button_down_latches();
    int mask = button_mask();

    input.upPressed =    (BUTTON_PRESSED(BADGE_BUTTON_UP,    down_latches));
    input.downPressed =  (BUTTON_PRESSED(BADGE_BUTTON_DOWN,  down_latches));
    input.leftPressed =  (BUTTON_PRESSED(BADGE_BUTTON_LEFT,  down_latches));
    input.rightPressed = (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches));
    input.upHeld =       (BUTTON_PRESSED(BADGE_BUTTON_UP,    mask));
    input.downHeld =     (BUTTON_PRESSED(BADGE_BUTTON_DOWN,  mask));
    input.leftHeld =     (BUTTON_PRESSED(BADGE_BUTTON_LEFT,  mask));
    input.rightHeld =    (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, mask));
    input.APressed =     (BUTTON_PRESSED(BADGE_BUTTON_A,     down_latches));
    input.BPressed =     (BUTTON_PRESSED(BADGE_BUTTON_B,     down_latches));
    input.AHeld =        (BUTTON_PRESSED(BADGE_BUTTON_A,     mask));
    input.BHeld =        (BUTTON_PRESSED(BADGE_BUTTON_B,     mask));
}

static void draw_level(const struct asset2 *asset, Camera camera) {
    int y, x, texture_row, texture_x, tile_x, tile_y, camera_x, camera_y;
    
    // unsigned char *pixdata;
    // unsigned char pixel;
    

    for (y = 0; y < level_height(); y++) {
        texture_row = (y % asset->y) * asset->x;
        for (x = 0; x < level_width(); x++) {
            texture_x = x % asset->x; //factor seqNum here
            // pixdata = (unsigned short*) &(asset->pixel16[texture_row + texture_x]);
            uint8_t pixdata = asset->pixel[texture_row + texture_x];
            Tile tile = (Tile)pixdata; /* 1 pixel per 2 bytes */
            switch (tile) {
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

            FbImageRect(&possum2, camera_x, camera_y, tile_x, tile_y, TILE_SIZE, TILE_SIZE, MAGENTA);
        }
    }
}



static void draw_hud(void)
{   
    if (stats.levels_completed[stats.level_number] == true) {
        // FbMove(0,0);
        // FbColor(GREEN);
        // FbWriteString("")
        FbImageRect(&possum2, 0, 0, 48, 48, 8, 8, MAGENTA);
    }
    char buf[20];
    FbColor(WHITE);
    FbMove(8, 0);
    snprintf(buf, sizeof(buf), "%d,%d moves:%d", player.position.x, player.position.y, moves);
    FbWriteString(buf);

}


static void camera_move(void)
{   
    int width = level_width() * TILE_SIZE;
    int height = level_height() * TILE_SIZE;
    int x = player.position.x * TILE_SIZE;
    int y = player.position.y * TILE_SIZE;
    camera.target.x = x;
    camera.target.y = y;
    
    if (stats.level_number == 0) return;

    if (width - TILE_SIZE <= LCD_XSIZE) {
        camera.target.x = width/2;
    } else if (x < LCD_XSIZE/2) {
        camera.target.x = LCD_XSIZE/2;
    } else if (x >= width - LCD_XSIZE/2) {
        camera.target.x = width - LCD_XSIZE/2;
    }

    if (height - TILE_SIZE <= LCD_YSIZE) {
        camera.target.y = height/2;
    } else if (y < LCD_YSIZE/2) {
        camera.target.y = LCD_YSIZE/2;
    } else if (y >= height - LCD_YSIZE/2) {
        camera.target.y = height - LCD_YSIZE/2;
    }
}


static void draw_level_menu(void) {
    if (!screen_changed)
        return;

    int x, y, texture_x,texture_row;
    const struct asset2 *world = &microban_levels;    
    Rectangle level_rect = microban_levels_rects[menu_selection_level];
    bool selection_unlocked = stats.levels_unlocked[menu_selection_level] == true;

    if (selection_unlocked) {
        for (y = 0; y < level_rect.height; y++) {
            texture_row = (y + level_rect.y) * world->x;
            for (x = 0; x < level_rect.width; x++) {
                texture_x = (x + level_rect.x);
                uint8_t pixdata = world->pixel[texture_row + texture_x];
                unsigned int tile = (Tile)pixdata;

                FbImageRect(&possum2, x * 8 + 24, y * 8 + 8, tile * 8, 32, 8, 8, MAGENTA);
            }
        }
    } else {
        FbColor(GREY8);
        FbMove(40, LCD_YSIZE/2 - 8);
        FbWriteString("CLEAR 10 ROOMS");
        FbMove(60, LCD_YSIZE/2);
        FbWriteString("TO UNLOCK");
    }

    char buf[20];
    for (int row = 1; row < MAX_LEVELS; row += 1) {
        int y = (row - menu_selection_level + 6) * 8;

        if (y < 0) continue;
        if (y > LCD_YSIZE) break;

        FbColor(WHITE);
        if (row == stats.level_number) {
            FbImageRect(&possum2, 0, y, 5 * 8, 32, 8, 8, MAGENTA);
        } else if (row == menu_selection_level) {
            FbMove(0, y);
            FbWriteString(">");
        }


        if(stats.levels_completed[row] == true) {
            FbColor(GREEN);
        } else if(stats.levels_unlocked[row] == false) {
            FbColor(GREY8);
        }

        FbMove(8, y);
        snprintf(buf, sizeof(buf), "%d", row);
        FbWriteString(buf);

        for(int i = 0; i < BREAKPOINT_COUNT; i++) {
            if (row == PROGRESS_BREAKPOINTS[i]) {
                FbColor(WHITE);
                FbClippedLine(0, y-1, 32, y-1);
            }
        }
    }

    if (stats.levels_completed[menu_selection_level] == true) {
        FbMove(32, 0);
        snprintf(buf, sizeof(buf), "best moves: %d", stats.best_moves[menu_selection_level]);
        FbWriteString(buf);
    }
    FbColor(WHITE);
    if (selection_unlocked) {
        FbMove(LCD_XSIZE/2, LCD_YSIZE - 16);
        FbWriteString("A: travel");
    }
    FbMove(LCD_XSIZE/2, LCD_YSIZE - 8);
    FbWriteString("B: return");
    

    if (menu_streak_popup) {

        struct ui_text_box info_box = {
            .x = 16,
            .y = 16,
            .width = LCD_XSIZE - 32,
            .height = 32,
            .text = "Sure? Streak will be lost!",
            .outline_size = 1,
            .outline_color = WHITE,
            .fill_color = BLACK,
            .text_color = WHITE,
        };
        ui_text_box_draw(info_box);
    }


    FbSwapBuffers();
    screen_changed = 1;
}


static void DrawStringDropshadow(const char *string, unsigned char x, unsigned char y, unsigned short text_color, unsigned short background_color) {
    FbBackgroundColor(G_Fb.transIndex);
    FbColor(background_color);
    FbMove(x - 2, y + 1);
    FbWriteString(string);
    FbMove(x - 2, y);
    FbWriteString(string);
    FbMove(x - 1, y + 1);
    FbWriteString(string);
    FbMove(x - 1, y);
    FbWriteString(string);

    FbMove(x, y);
    FbColor(text_color);
    FbWriteString(string);
}

// static void DrawString(const char *string, unsigned char x, unsigned char y, unsigned short text_color, unsigned short background_color) {
//     FbColor(background_color);
//     FbColor(text_color);
//     FbMove(x, y);
//     FbWriteString(string);
// }


static void draw_screen(void)
{
    if (!screen_changed)
        return;
    
    camera_move();

    FbImageRect(&bluestreet, 0, 0, camera.target.x + stats.level_number*211, camera.target.y + stats.level_number*151, LCD_XSIZE, LCD_YSIZE, MAGENTA);


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
        ptex_y += TILE_SIZE * 2;
    } else if (player.pushing) {
        ptex_y += TILE_SIZE;
    }

    //walk frame 2
    if (moves_tried % 2 == 0) {
        ptex_x += TILE_SIZE * 4;
    }

    FbImageRect(&possum2, camera_x, camera_y, ptex_x, ptex_y, TILE_SIZE, TILE_SIZE, MAGENTA);

    switch (run_state) {
    case GAMEPLAY:
        if (DrawHud) draw_hud();
        break;
    case WIN: {


        char buf[20];
        const int unit = 12;
        int x = 8;
        int y = 4;

        FbBackgroundColor(G_Fb.transIndex);
        FbColor(YELLOW);

        int nice_x = tick*8 - LCD_XSIZE + 4;
        if (nice_x > 4) nice_x = 4;
        int clear_x = LCD_XSIZE - tick*8 + 4;
        if (clear_x < 4) clear_x = 4;
        FbImageRect4bit_Palette(&NICE_CLEAR, nice_x, y, 0, 1, NICE_CLEAR.x, 24, MAGENTA, nice_clear_cycle);
        FbImageRect4bit_Palette(&NICE_CLEAR, clear_x, y + 24, 0, 25, NICE_CLEAR.x, 24, MAGENTA, nice_clear_cycle);
        if (tick % 4 == 0) FbPaletteCycle(nice_clear_cycle, 3, 8);

        if (nice_x < 4) break;

        y += NICE_CLEAR.y + 4;


        // FbMove(x, y);
        // FbWriteString("NICE CLEAR!!");
        // y += unit;

        // FbMove(x, y);
        snprintf(buf, sizeof(buf), "room %d complete.", stats.level_number);
        DrawStringDropshadow(buf, x, y, YELLOW, BLACK);
        y += unit;

        snprintf(buf, sizeof(buf), "moves: %d", moves);
        DrawStringDropshadow(buf, x, y, YELLOW, BLACK);
        y += unit;


        // FbMove(x, y);
        // FbWriteString("try next?");
        // y += unit;

        if (stats.streak >= 3) {
            y += unit;
            snprintf(buf, sizeof(buf), "STREAK: %d!!", stats.streak);
            DrawStringDropshadow(buf, x, y, nice_clear_cycle[4], BLACK);
        }

        break;
    }

    case PAUSE:
        for (int i = 0; i < LCD_XSIZE * LCD_YSIZE; i++) {
            int y = i / LCD_XSIZE;
            int x = i % LCD_XSIZE;
            if (y % 2 == 0 && x % 2 == 0) {
                FbPlacePoint(BLACK, x, y);
            } else if (x % 2 != 0 && y % 2 != 0) {
                FbPlacePoint(BLACK, x, y);
            }
        }
        unsigned const short shadow = PACKRGB888(199, 21, 133);
        int y = 16;
        DrawStringDropshadow(">", 8, y + 12*menu_selection, YELLOW, shadow);
        DrawStringDropshadow("reset level", 16, y, WHITE, shadow);
        y += 12;
        DrawStringDropshadow("select level", 16, y, WHITE, shadow);
        y += 12;
        DrawStringDropshadow("about microban", 16, y, WHITE, shadow);
        y += 12;
        DrawStringDropshadow("exit", 16, y, WHITE, shadow);
        break;

    case LEVEL_MENU: 
        break;
    case ABOUT: {
        FbImageRect(&bluestreet, 0, 0, tick, -tick/2, LCD_XSIZE, LCD_YSIZE, MAGENTA);
        int x, y;
        x = 4;
        y = 4;
        unsigned const short shadow = PACKRGB888(199, 21, 133);
        DrawStringDropshadow("MICROBAN, 2025", x, y, WHITE, shadow);
        y += 12;
        DrawStringDropshadow("Programmed by Zach", x, y, WHITE, shadow);
        y += 8;
        DrawStringDropshadow("with artwork by", x + 8, y, WHITE, shadow);
        y += 8;
        DrawStringDropshadow("Ann & Hannah.", x + 16, y, WHITE, shadow);
        y += 12;
        DrawStringDropshadow("Original Sokoban", x, y, WHITE, shadow);
        y += 8;
        DrawStringDropshadow("game design by", x + 8, y, WHITE, shadow);
        y += 8;
        DrawStringDropshadow("Thinking Rabbit,", x + 16, y, WHITE, shadow);
        y += 8;
        DrawStringDropshadow("1982.", x + 24, y, WHITE, shadow);
        y += 12;
        DrawStringDropshadow("Including puzzles", x, y, WHITE, shadow);
        y += 8;
        DrawStringDropshadow("by David W Skinner,", x + 8, y, WHITE, shadow);
        y += 8;
        DrawStringDropshadow("Yoshio Murase,", x + 16, y, WHITE, shadow);
        y += 8;
        DrawStringDropshadow("& Zach Smith.", x + 24, y, WHITE, shadow);
        break;
        }
    }


    // int bus_x = (LCD_XSIZE - microban_busstop.x) / 2;
    // int bus_y = (LCD_YSIZE - microban_busstop.y) / 2;
    // FbPlaceFilledRectangle(bus_x - 1, bus_y - 1, microban_busstop.x + 2, microban_busstop.y + 2, BLACK);
    // FbImagePlace(&microban_busstop, bus_x, bus_y, MAGENTA);
    
    FbSwapBuffers();
    screen_changed = 1;
}



static void microban_run(void)
{   
    enum microban_state_run old_state = run_state;
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
    case LEVEL_MENU:
        process_input_LEVEL_MENU();
        break;
    case ABOUT:
        if(input.APressed || input.BPressed) {
            run_state = PAUSE;
        }
        break;
    }

    if (old_state != run_state) {
        tick = 0;
    }

    switch (run_state) {
    case GAMEPLAY:
        draw_screen();
        break;
    case PAUSE:
        draw_screen();
        break;
    case WIN:
        draw_screen();
        break;
    case LEVEL_MENU:
        draw_level_menu();
        break;
    case ABOUT:
        draw_screen();
        break;
    }
    FbBackgroundColor(BLACK);
}


static void microban_exit(void)
{
    microban_state = MICROBAN_INIT; /* So that when we start again, we do not immediately exit */
    FbFont(FONT);
    pop_app();
}

void microban_cb(struct badge_app *app)
{
    if (app->wake_up) {
        FbFont(FONT_SERIF);
        screen_changed = 1;
        app->wake_up = false;
    }


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

