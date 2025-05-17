
/*********************************************
 *
 *
 *
 **********************************************/

#include "colors.h"
#include "button.h"
#include "framebuffer.h"
#include "badge.h"
#include "simonSays_assets.h"
#include "simonSays.h"

/* Program states.  Initial state is MYPROGRAM_INIT */
enum latchissue_state_t {
    LATCHISSUE_INIT,
    LATCHISSUE_RUN,
    LATCHISSUE_EXIT,
};

const struct asset2 *alldark_p = &alldark;
const struct asset2 *upblue_p = &upblue;
const struct asset2 *leftred_p = &leftred;
const struct asset2 *downgreen_p = &downgreen;
const struct asset2 *rightyellow_p = &rightyellow;


static enum latchissue_state_t latchissue_state = LATCHISSUE_INIT;

typedef enum Choice {
    UP,
    RIGHT,
    DOWN,
    LEFT,
    NONE
}Choice;

Choice dPad;
bool waitForRelease, actuallyReleased;

static void check_buttons(void)
{
    int down_latches = button_down_latches();
    int up_latches = button_up_latches();
    if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down_latches)) {
        dPad = LEFT;
    } else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches)) {
        dPad = RIGHT;
    } else if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches)) {
        dPad = UP;
    } else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches)) {
        dPad = DOWN;
    } else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
        latchissue_state = LATCHISSUE_EXIT;
    } else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
        latchissue_state = LATCHISSUE_EXIT;
    }
    if(waitForRelease){
        if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, up_latches)) {

        } else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches)) {
        } else if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches)) {
        } else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches)) {
        } else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
            latchissue_state = LATCHISSUE_EXIT;
        } else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
            latchissue_state = LATCHISSUE_EXIT;
        }
    }
}
static void showDark(void){
    FbClear();
    FbColor(WHITE);
    FbMove(16,0);
    FbImage2(alldark_p, 0);
    FbSwapBuffers();
}
static void showUP(void){
    FbClear();
    FbColor(WHITE);
    FbMove(16,0);
    FbImage2(upblue_p, 0);
    FbSwapBuffers();
}
static void showDown(void){
    FbClear();
    FbColor(WHITE);
    FbMove(16,0);
    FbImage2(downgreen_p, 0);
    FbSwapBuffers();
}

static void showLeft(void){
    FbClear();
    FbColor(WHITE);
    FbMove(16,0);
    FbImage2(leftred_p, 0);
    FbSwapBuffers();
}

static void showRight(void){
    FbClear();
    FbColor(WHITE);
    FbMove(16,0);
    FbImage2(rightyellow_p, 0);
    FbSwapBuffers();
}

static void latchissue_init(void)
{
    waitForRelease = false;
    dPad = NONE;
    FbInit();
    latchissue_state = LATCHISSUE_RUN;
    showDark();
    check_buttons();
}

static void latchissue_run(void)
{
    if(actuallyReleased){
        //do whatever happens when you let go
        //of the direction
        return;
    }
    if(dPad!=NONE){
        //dpad got pressed do
        //whatever happens when
        //dpad is pressed
        switch(dPad){
            case UP:{
                showUP();
                break;
            }
            case DOWN:{
                showDown();
                break;
            }
            case RIGHT:{
                showRight();
                break;
            }
            case LEFT:{
                showLeft();

                break;
            }
            default:{
                //shouldnt get here
            }
        }
        waitForRelease = true;
        return;
    }
    check_buttons();
}

static void latchissue_exit(void)
{
    latchissue_state = LATCHISSUE_INIT; /* So that when we start again, we do not immediately exit */
    pop_app();
}

/* You will need to rename myprogram_cb() something else. */

void latchissue_cb(struct badge_app *app)
{
    if (app->wake_up)

        switch (latchissue_state) {
            case LATCHISSUE_INIT:
                latchissue_init();
                break;
            case LATCHISSUE_RUN:
                latchissue_run();
                break;
            case LATCHISSUE_EXIT:
                latchissue_exit();
                break;
            default:
                break;
        }
}


