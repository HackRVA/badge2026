

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

const struct asset2 *alldark_p = &alldark;
const struct asset2 *upblue_p = &upblue;
const struct asset2 *leftred_p = &leftred;
const struct asset2 *downgreen_p = &downgreen;
const struct asset2 *rightyellow_p = &rightyellow;
const int MAX_TURNS = 100;


/* Program states.  Initial state is MYPROGRAM_INIT */
enum simonSays_state_t {
	SIMONSAYS_INIT,
	SIMONSAYS_PLAYBACK_SETUP,
	SIMONSAYS_PLAYBACK_RUN,
	SIMONSAYS_PLAYERTURN_SETUP,
	SIMONSAYS_PLAYERTURN_RUN,
	SIMONSAYS_PLAYERLOST,
	SIMONSAYS_ADD,
	SIMONSAYS_GENERIC_DELAY,
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

bool lastone;
int it,usedturns,dReturn,sReturn = 0;
uint64_t stoptime,now,lStop,lNow;

static enum simonSays_state_t simonSays_state = SIMONSAYS_INIT;

//DISPLAYING THE RIGHT CHOICES
void showChoice(Choice c, bool sound){
	const struct asset2 *temp;
	int freq=0;
	switch (c){
		case UP:
			temp = upblue_p;
			//freq = 196;
			freq = NOTE_G4;
			led_pwm_enable(BADGE_LED_RGB_BLUE, 255);
			break;
		case RIGHT:
			temp = rightyellow_p;
			//freq = 261;
			freq = NOTE_E4;
			led_pwm_enable(BADGE_LED_RGB_RED, 255);
			led_pwm_enable(BADGE_LED_RGB_GREEN, 255);
			break;
		case DOWN:
			temp = downgreen_p;
			//freq = 392;
			freq= NOTE_C4;
			led_pwm_enable(BADGE_LED_RGB_GREEN, 255);
			break;
		case LEFT:
			temp = leftred_p;
			//freq = 329;
			freq = NOTE_G3;
			led_pwm_enable(BADGE_LED_RGB_RED, 255);
			break;
		default:
			temp = alldark_p;
			freq = 0;
			led_pwm_disable(BADGE_LED_RGB_RED);
			led_pwm_disable(BADGE_LED_RGB_GREEN);
			led_pwm_disable(BADGE_LED_RGB_BLUE);
			break;
	}
	if(sound) {
		audio_out_beep(freq,500);
	}

	FbClear();
	FbColor(WHITE);
	FbMove(16,0);
	FbImage2(temp, 0);
	FbSwapBuffers();
}



//THIS IS TO POLL THE COLLABORATIVE WORKSPACE
//NOT SURE IF ITS EVEN REALLY NEEDED.
//PROBABLY KEEPS THE SCREENSAVOR IN CHECK THO
void checkin(void)
{
	button_reset_last_input_timestamp();
}
// provided to tax the badge for a set ammount of time
void haltAndCatchFire(){
	now = rtc_get_ms_since_boot();
	uint64_t stopagain = stoptime+150;
	if(now>stoptime){
		showChoice(NONE,false);
		led_pwm_disable(BADGE_LED_RGB_RED);
		led_pwm_disable(BADGE_LED_RGB_GREEN);
		led_pwm_disable(BADGE_LED_RGB_BLUE);
		if(now>stopagain){
		simonSays_state = dReturn;
		}
	}
	checkin();
}
//entrypoint to halt and catch fire
void delay(int ms, int returnTo){
	stoptime = rtc_get_ms_since_boot()+ms;
	dReturn=returnTo;
	simonSays_state = SIMONSAYS_GENERIC_DELAY;
}


//RANDOMIZE THE SEQUENCE
	Choice randChoice(void){
	uint64_t timestamp= rtc_get_ms_since_boot();
	unsigned int my_state = 0xa5a5a5a5 ^ timestamp;
	return (xorshift(&my_state) % 4);
}
//ADD NEW TURNS IN THE SEQUENCE
	void newTurn(void){
	if(usedturns==MAX_TURNS){
		//win the game :)
		//no need to add a link
		simonSays_state = SIMONSAYS_EXIT;
		return; // LETS GET OUT OF THIS PLACE
	}
	dPad=NONE; //CLEAR ANY DPAD DATA
	sequence[usedturns]=randChoice(); //SET NEXT FRAME
	usedturns++; //COUNT 1...2...3... BREATHE
	simonSays_state = SIMONSAYS_PLAYBACK_SETUP;
}


//cleaning house
	void cleanSequence(void){
	for(int i =0;i<MAX_TURNS;i++){
		sequence[i]=NONE;
	}

}

//SETS UP OUR WORK ENVIRONMENT
	void simonSays_init(void){
	usedturns=0; // COUNTS THE TURNS
	cleanSequence(); //CLEANS THE SEQUENCE
	newTurn(); //ADDS A NEW FRAME TO THE SEQUENCE
	FbInit();
	FbClear();
	simonSays_state = SIMONSAYS_PLAYBACK_SETUP;
}
//CHECKS TO SEE WHAT BUTTONS ARE PRESSED
	void check_buttons(void){
    int down_latches = button_down_latches();
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
	} else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
		simonSays_state = SIMONSAYS_EXIT;
	}
	else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
		simonSays_state = SIMONSAYS_EXIT;
	}
}

//dunno if im going to use it...
//i was making simon dance a bit
//may make some different modes
void roundAndRound(){
	showChoice(NONE,false);
	}
//sets up the playback variables
void playbackSetup (void){
	it = 0; //GENERIC GLOBAL ITERATOR
	dPad = NONE; // CLEAR ANY DPAD DATA
	lastone = false; // CLEAR THE KILLSWITCH
	simonSays_state = SIMONSAYS_PLAYBACK_RUN;
}

//playback logic.
void playbackRun (void){

	//IF ITS THE LAST FRAME IN THE SEQUENCE
	if(lastone){
		//START THE PLAYERS TURN
		simonSays_state = SIMONSAYS_PLAYERTURN_SETUP;
		return;
	}
	//IF THIS IS GOING TO BE THE LAST TURN
	if((it+1==usedturns) | (it==99)){
		lastone=true;
	}
	//SHOW THE FRAME AND WAIT A BIT
	showChoice(sequence[it],true);
	check_buttons();
	checkin();
	it++;
	delay(800,SIMONSAYS_PLAYBACK_RUN);
}

//setup to listen and verify player input
void playerTurnSetup(void){
	it=0;
	lStop = (rtc_get_ms_since_boot()+3000);
	simonSays_state = SIMONSAYS_PLAYERTURN_RUN;
	dPad=NONE;
	check_buttons();
}

//playerturn logic.
void playerTurnRun (void){

	//IF THE DPAD MATCHES THE CURRENT
	//NODE IN THE SEQUENCE
	if(dPad==sequence[it]){
		led_pwm_disable(BADGE_LED_RGB_RED);
		led_pwm_disable(BADGE_LED_RGB_GREEN);
		led_pwm_disable(BADGE_LED_RGB_BLUE);
		//PLAY THE CHOICE
		showChoice(dPad,true);
		//IF ITS THE LAST IN THE SEQUENCE
		if(it==(usedturns-1)){
			//sequence complete
			dPad=NONE;
			//ADD A NEW ONE
			delay(800,SIMONSAYS_ADD);
		}
		//IF ITS NOT THE LAST ONE THEN WE
		//KEEP PLAYING THE SEQUENCE
		else{
			it++;
			dPad=NONE;
			check_buttons();
			//GIVE EM SOME EXTRA TIME
			lStop+=1000;
			return;
		}
	}
	else{
			//COUNTDOWN TIMER
		lNow=rtc_get_ms_since_boot();
		bool timesup = (lNow>lStop);
		if(dPad!=NONE||timesup){
			//GOT IT WRONG END THE GAME
			//PROBABLY WILL GO TO A MENU LATER
			//led_pwm_enable(BADGE_LED_RED,255);

			audio_out_beep(42,2000);
			delay(1000,SIMONSAYS_PLAYERLOST);
			return;
		}
		else{
			//NO INPUT SO CHECK BUTTONS AGAIN
			check_buttons();
			return;
		}

	}

}
void playerLost(void){
	FbColor(RED);
	FbMove(10, LCD_YSIZE / 2);
	FbWriteLine("YOU LOSE!");
	FbSwapBuffers();
	delay(2000,SIMONSAYS_EXIT);
}
//THIS GETS OUT OF THE PROGRAM CLEANLY
void simonSays_exit(void){
	cleanSequence();
	simonSays_state = SIMONSAYS_INIT; /* So that when we start again, we do not immediately exit */
	pop_app();
}

/* collabortive processing interface */
void simonSays_cb(__attribute__((unused)) struct badge_app *app){
	switch (simonSays_state) {
	case SIMONSAYS_INIT:
		simonSays_init();
		break;
	case SIMONSAYS_PLAYERTURN_SETUP:
		playerTurnSetup();
		break;
	case SIMONSAYS_PLAYERTURN_RUN:
		playerTurnRun();
		break;
	case SIMONSAYS_PLAYBACK_SETUP:
		playbackSetup();
		break;
	case SIMONSAYS_PLAYBACK_RUN:
		playbackRun();
		break;
	case SIMONSAYS_PLAYERLOST:
		playerLost();
		break;
	case SIMONSAYS_ADD:
		newTurn();
		break;
	case SIMONSAYS_GENERIC_DELAY:
		haltAndCatchFire();
		break;
	case SIMONSAYS_EXIT:
		simonSays_exit();
		break;
	default:
		break;
	}
}

