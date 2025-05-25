#include "colors.h"
#include "menu.h"
#include "button.h"
#include "framebuffer.h"
#include "badge.h"
#include "stdio.h"
#include "stdlib.h"
#include "rtc.h"
#include "badge.h"
#include "simonSays_assets.h"
#include "audio.h"
#include "xorshift.h"
#include "dynmenu.h"
#include "music.h"
#include "led_pwm.h"

//load instances of all the sprite assets
const struct asset2 *alldark_p = &alldark;
const struct asset2 *upblue_p = &allup;
const struct asset2 *leftred_p = &allleft;
const struct asset2 *downgreen_p = &alldown;
const struct asset2 *rightyellow_p = &allright;

//arbitrary max level
const int MAX_TURNS = 100;
      int SPEED = 500;

 struct audio_out_spec spec;

/* Program states.  Initial state is MYPROGRAM_INIT */
enum simonSays_state_t {
	SIMONSAYS_INIT,
	SIMONSAYS_BEEP,
	SIMONSAYS_WAIT,
	SIMONSAYS_INSTRUMENT,
	SIMONSAYS_INSTRUMENT_SETUP,
	SIMONSAYS_PLAYER_SETUP,
	SIMONSAYS_PLAYER_RUN,
	SIMONSAYS_PLAYBACK_SETUP,
	SIMONSAYS_PLAYBACK_RUN,
	SIMONSAYS_PLAYER_LOSE,
	SIMONSAYS_DISCO,
	SIMONSAYS_MENU,
	SIMONSAYS_EXIT
};
// CHOICES THE PLAYER CAN MAKE
typedef enum Choice {
	UP,
	RIGHT,
	DOWN,
	LEFT,
	NONE
}Choice;


enum Choice sequence[100];
Choice dPad;

static enum simonSays_state_t simonSays_state = SIMONSAYS_INIT;
static enum simonSays_state_t waitstate;
int waitms;

uint64_t now, then;
bool listening,waitingForRelease,actuallyReleased,playing,started, timesUp,changeNote;
bool timeIsSet=false;
int usedTurns,it;
static struct dynmenu menu;

static void simonSaysMenu(void)
{
	static int menu_setup = 0;

	static struct dynmenu_item menu_item[5];

	if (!menu_setup) {
		dynmenu_init(&menu, menu_item, 5);
		dynmenu_clear(&menu);
		dynmenu_set_title(&menu, "Simon Says", "", "");
		dynmenu_add_item(&menu, "Play Simon", 0, 1);
		dynmenu_add_item(&menu, "PLAY Songs", 0, 2);
		dynmenu_add_item(&menu, "Disco", 0, 3);
		dynmenu_add_item(&menu, "EXIT", 0, 4);
		menu_setup = 1;
	}

	if (!dynmenu_let_user_choose(&menu))
		return; // let dynmenu take over as runningApp for a bit

		// dynmenu is done, the user has made their choice.
	switch (dynmenu_get_user_choice(&menu)) { // get choice and reset for next time.
			case 1:
				simonSays_state = SIMONSAYS_PLAYBACK_SETUP;
				break;
			case 2:
				simonSays_state = SIMONSAYS_INSTRUMENT_SETUP;
				break;
			case 3:
				simonSays_state = SIMONSAYS_DISCO;
				break;
			case 4:
				simonSays_state = SIMONSAYS_EXIT;
				break;
			case DYNMENU_SELECTION_ABORTED:
				simonSaysMenu();
				break;
			default:
				simonSaysMenu();
				break;
		}
}





void waitForState(enum simonSays_state_t returnTo){

	if(timeIsSet){
		now = rtc_get_ms_since_boot();
		if(now<then){
			return;
		}
		else{
			simonSays_state = returnTo;
			timeIsSet = false;
			timesUp = true;
			return;
		}
	}

}

void simonSays_wait(int ms,enum simonSays_state_t state){
	simonSays_state = SIMONSAYS_WAIT;
	waitms = ms;
	waitstate = state;
	timeIsSet = true;
	then = rtc_get_ms_since_boot()+ms;
}

void soundStart (int freq){
	audio_out_beep(freq,30000);
}


void soundStop (void){
	audio_out_beep(0,0);
}

//DISPLAYING THE RIGHT CHOICES
void showChoice(Choice c){
	const struct asset2 *temp;
	int freq=0;
	switch (c){
		case UP:
			temp = upblue_p;
			//freq = 196;

			freq = NOTE_G4;
			soundStart(freq);
			led_pwm_enable(BADGE_LED_RGB_RED,   0);
			led_pwm_enable(BADGE_LED_RGB_GREEN, 0);
			led_pwm_enable(BADGE_LED_RGB_BLUE, 30);
			break;
		case RIGHT:
			temp = rightyellow_p;
			//freq = 261;

			freq = NOTE_E4;
			soundStart(freq);
			led_pwm_enable(BADGE_LED_RGB_RED,  30);
			led_pwm_enable(BADGE_LED_RGB_GREEN,30);
			led_pwm_enable(BADGE_LED_RGB_BLUE,  0);
			break;
		case DOWN:
			temp = downgreen_p;
			//freq = 392;

			freq= NOTE_C4;
			soundStart(freq);
			led_pwm_enable(BADGE_LED_RGB_RED,   0);
			led_pwm_enable(BADGE_LED_RGB_GREEN,30);
			led_pwm_enable(BADGE_LED_RGB_BLUE,  0);
			break;
		case LEFT:
			temp = leftred_p;
			//freq = 329;
			soundStart(NOTE_G3);
			freq = NOTE_G3;
			led_pwm_enable(BADGE_LED_RGB_RED,  30);
			led_pwm_enable(BADGE_LED_RGB_GREEN, 0);
			led_pwm_enable(BADGE_LED_RGB_BLUE,  0);
			break;
		default:
			temp = alldark_p;
			freq = 0;
			led_pwm_disable(BADGE_LED_RGB_RED);
			led_pwm_disable(BADGE_LED_RGB_GREEN);
			led_pwm_disable(BADGE_LED_RGB_BLUE);
			break;
	}

	FbClear();
	FbColor(WHITE);
	FbMove(16,0);
	FbImage2(temp, 0);
	FbSwapBuffers();
}

void playChoice(Choice c){
	showChoice(c);
	simonSays_wait(SPEED,SIMONSAYS_PLAYBACK_RUN);
	showChoice(NONE);
	soundStop();
}

//THIS IS TO POLL THE COLLABORATIVE WORKSPACE
//NOT SURE IF ITS EVEN REALLY NEEDED.
//PROBABLY KEEPS THE SCREENSAVOR IN CHECK THO
void checkin(void)
{
	button_reset_last_input_timestamp();
}

//RANDOMIZE THE SEQUENCE
	Choice randChoice(void){
	uint64_t timestamp= rtc_get_ms_since_boot();
	unsigned int my_state = 0xa5a5a5a5 ^ timestamp;
	return (xorshift(&my_state) % 4);
}



void newRound(void){
	sequence[usedTurns] = randChoice();
	if(SPEED>100){
	SPEED = SPEED - 50;
	}
	usedTurns++;
}

void resetButtons(void){
	showChoice(NONE);
	dPad = NONE;
	soundStop();
	waitingForRelease = false;
	actuallyReleased = false;
	listening = true;
}

//CHECKS TO SEE WHAT BUTTONS ARE PRESSED
void check_buttons(void){
	int down_latches = button_down_latches();
	int up_latches = button_up_latches();

	if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
				playing = false;
				simonSays_state = SIMONSAYS_INIT;
		return;
	}

	else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
				playing = false;
				simonSays_state = SIMONSAYS_INIT;
		return;
	}

	if(listening){
		if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down_latches)) {
			dPad = LEFT;
		}
		else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT,down_latches)) {
			dPad = RIGHT;
		}
		else if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches)) {
			dPad = UP;
		}
		else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN,down_latches))
		{
			dPad = DOWN;
		}
	}

	if(waitingForRelease){
		if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, up_latches)) {
			if(dPad == LEFT){
				actuallyReleased = true;
			}
		}
		else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT,up_latches)) {
			if(dPad == RIGHT){
				actuallyReleased = true;
			}
		}
		else if (BUTTON_PRESSED(BADGE_BUTTON_UP, up_latches)) {
			if(dPad == UP){
				actuallyReleased = true;
			}
		}
		else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN,up_latches))
		{
			if(dPad == DOWN){
				actuallyReleased = true;
			}
		} else if (BUTTON_PRESSED(BADGE_BUTTON_A, up_latches)) {
			//simonSays_state = SIMONSAYS_EXIT;
		}
		else if (BUTTON_PRESSED(BADGE_BUTTON_B, up_latches)) {
			//simonSays_state = SIMONSAYS_EXIT;
		}
	}

}


void disco (void){
	checkin();
	playing = true;
	check_buttons();
	uint64_t timestamp= rtc_get_ms_since_boot();
	unsigned int my_state = 0xa5a5a5a5 ^ timestamp;
	if(xorshift(&my_state) % 2){
	showChoice(xorshift(&my_state) % 4);
	timestamp= rtc_get_ms_since_boot();
	my_state = 0xa5a5a5a5 ^ timestamp;
	if(xorshift(&my_state) % 2){
	led_pwm_enable(BADGE_LED_RGB_RED,  xorshift(&my_state) % 255);
	}
	timestamp= rtc_get_ms_since_boot();
	my_state = 0xa5a5a5a5 ^ timestamp;
	if(xorshift(&my_state) % 2){
	led_pwm_enable(BADGE_LED_RGB_GREEN, xorshift(&my_state) % 255);
	}
	timestamp= rtc_get_ms_since_boot();
	my_state = 0xa5a5a5a5 ^ timestamp;
	if(xorshift(&my_state) % 2){
	led_pwm_enable(BADGE_LED_RGB_BLUE,  xorshift(&my_state) % 255);
	}
	timestamp= rtc_get_ms_since_boot();
	my_state = 0xa5a5a5a5 ^ timestamp;
	soundStart(xorshift(&my_state) % 2000);
	timestamp= rtc_get_ms_since_boot();
	my_state = 0xa5a5a5a5 ^ timestamp;
		check_buttons();
	if(playing){
	simonSays_wait(xorshift(&my_state) % 200,SIMONSAYS_DISCO);
	}
	}
	else{
		check_buttons();
		showChoice(NONE);
	}


}


void clearSequence(void){
	for(int i=0;i<MAX_TURNS;i++){
		sequence[i] = NONE;
	}
	it = 0;
	usedTurns =0;
}

//SETS UP OUR WORK ENVIRONMENT
	void simonSays_init(void){




	started = false;
	it = 0;
	usedTurns=0;
	SPEED = 500;
	clearSequence();
	newRound();
	timeIsSet = false;
	listening = false;
	waitingForRelease = false;
	FbInit();
	FbClear();
	FbMove(16,0);
	FbSwapBuffers();
	soundStop();
	simonSays_state = SIMONSAYS_MENU;
}

	void playerSetup(void){
		it = 0;
		dPad = NONE;
		listening = true;
		check_buttons();
		now = rtc_get_ms_since_boot();
		then = rtc_get_ms_since_boot()+3000;
		simonSays_state = SIMONSAYS_PLAYER_RUN;

	}
	void playerRun(void){
		now = rtc_get_ms_since_boot();
		if(now>then){
			simonSays_state = SIMONSAYS_PLAYER_LOSE;
		}
		if(waitingForRelease){
		if(actuallyReleased){
			if(sequence[it]==dPad){
				//if last one
				if((it>98)|(sequence[it+1]==NONE)){
					resetButtons();
					newRound();
					simonSays_wait(SPEED,SIMONSAYS_PLAYBACK_SETUP);
					return;
				}
				else{
					then = then + SPEED;
					resetButtons();
					it++;
					return;
				}
			}
			else{
				resetButtons();
				simonSays_state = SIMONSAYS_PLAYER_LOSE;
				return;
			}
		}
		check_buttons();
		return;
		}
		if(dPad==NONE){
				check_buttons();
				return;
			}
		else{
			showChoice(dPad);
			waitingForRelease = true;
			listening = false;
			check_buttons();
			return;
		}
	}
	void musicSetup(void){
		showChoice(NONE);
		dPad = NONE;
		waitingForRelease =false;
		actuallyReleased = false;
		listening = true;
		check_buttons();
		simonSays_state = SIMONSAYS_INSTRUMENT;
	}
	void playMusic (void){
		check_buttons();
		if(waitingForRelease){
			if(actuallyReleased){
				resetButtons();
				return;
			}
			check_buttons();
			return;
		}
		if(dPad!=NONE){
			showChoice(dPad);
			waitingForRelease = true;
			listening = false;
			check_buttons();
			return;
		}
		check_buttons();
	}

	void playbackSetup(void){
		playing = false;
		it = 0;
		simonSays_state = SIMONSAYS_PLAYBACK_RUN;
	}
	void playbackRun(void){
		if(playing){
			soundStop();
			playing = false;
			showChoice(NONE);
			simonSays_wait(SPEED,SIMONSAYS_PLAYBACK_RUN);
			//check for last
			if(sequence[it+1]==NONE){
				simonSays_state = SIMONSAYS_PLAYER_SETUP;
				return;
			}
			else{
				it++;
				return;
			}
		}
		else{
			showChoice(sequence[it]);
			playing = true;
			simonSays_wait(SPEED,SIMONSAYS_PLAYBACK_RUN);
			return;
		}
	}

void playerLose(void){

	spec = (const struct audio_out_spec) {
		.callback = NULL,
		.frequency_hz = 93*42,
		.duration_ms = 3000,
		.decay = 0,
		.phase = 0,
		.amplitude_dBFS = -3,
		.restart = false,
		.type = AUDIO_OUT_TYPE_NES_NOISE,
		.nes_noise.lfsr_val = 1,
		.nes_noise.mode_flag = true ,
	};


		clearSequence();
		soundStop();
		showChoice(NONE);
        (void) audio_out_play(0, &spec);

		simonSays_state = SIMONSAYS_INIT;

	}


//THIS GETS OUT OF THE PROGRAM CLEANLY
void simonSays_exit(void){
	soundStop();
	simonSays_state = SIMONSAYS_INIT; /* So that when we start again, we do not immediately exit */
	pop_app();
}


/* collabortive processing interface */
void simonSays_cb(__attribute__((unused)) struct badge_app *app){
	switch (simonSays_state) {
	case SIMONSAYS_INIT:
		simonSays_init();
		break;
	case SIMONSAYS_WAIT:
		waitForState(waitstate);
		break;
	case SIMONSAYS_PLAYER_SETUP:
		playerSetup();
		break;
	case SIMONSAYS_PLAYER_RUN:
		playerRun();
		break;
	case SIMONSAYS_PLAYBACK_SETUP:
		playbackSetup();
		break;
	case SIMONSAYS_PLAYBACK_RUN:
		playbackRun();
		break;
	case SIMONSAYS_PLAYER_LOSE:
		playerLose();
		break;
	case SIMONSAYS_MENU:
		simonSaysMenu();
		break;
	case SIMONSAYS_INSTRUMENT_SETUP:
		musicSetup();
		break;
	case SIMONSAYS_INSTRUMENT:
		playMusic();
		break;
	case SIMONSAYS_DISCO:
		disco();
		break;
	case SIMONSAYS_EXIT:
		simonSays_exit();
		break;
	default:
		break;
	}
}

