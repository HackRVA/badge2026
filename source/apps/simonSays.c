

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

const struct asset2 *alldark_p = &alldark;
const struct asset2 *upblue_p = &upblue;
const struct asset2 *leftred_p = &leftred;
const struct asset2 *downgreen_p = &downgreen;
const struct asset2 *rightyellow_p = &rightyellow;
const int MAX_TURNS = 100;


/* Program states.  Initial state is MYPROGRAM_INIT */
enum simonSays_state_t {
	SIMONSAYS_INIT,
	SIMONSAYS_GAMELOOP,
	SIMONSAYS_PLAY,
	SIMONSAYS_SPIRAL,
	SIMONSAYS_TEST,
	SIMONSAYS_ADD,
	SIMONSAYS_EXIT
};
typedef enum Choice {
	UP,
	RIGHT,
	DOWN,
	LEFT,
	NONE
}Choice;

typedef struct Turn{
	Choice choice;
	struct Turn *next;
	int turnNum;
}Turn;

Turn *sequence, *activeFrame;
Choice dPad;

static enum simonSays_state_t simonSays_state = SIMONSAYS_INIT;
int turns=1;

static Choice randChoice(void){
	uint64_t timestamp= rtc_get_ms_since_boot();
	unsigned int my_state = 0xa5a5a5a5 ^ timestamp;
	return (xorshift(&my_state) % 4);

}

static void newTurn(void){
	Turn *t;
	if(turns==MAX_TURNS){
		//win the game :)
		//no need to add a link
		return;
	}
	t = (Turn*)malloc(sizeof(Turn));
	t->turnNum =1+turns;

	t->choice = randChoice();
	t->next = NULL;
	turns++;
	for(activeFrame=sequence;activeFrame->next!=NULL;activeFrame=activeFrame->next){

	}
	activeFrame->next=t;
	activeFrame=activeFrame->next;
}

static void checkin(void)
{
	button_reset_last_input_timestamp();
}


static void simonSays_init(void){
	turns=1;
	sequence = (Turn*)malloc(sizeof(Turn));
	sequence->choice = randChoice();
	sequence->next=NULL;
	sequence->turnNum=turns;
	activeFrame = (Turn*)malloc(sizeof(Turn));
	activeFrame = sequence;
	FbInit();
	FbClear();
	simonSays_state = SIMONSAYS_GAMELOOP;
}


static void check_buttons(void){
    int down_latches = button_down_latches();
	if (BUTTON_PRESSED(BADGE_BUTTON_LEFT, down_latches)) {
		printf("left button pressed\n");
		dPad = LEFT;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_RIGHT, down_latches)) {
		printf("right button pressed\n");
		dPad = RIGHT;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_UP, down_latches)) {
		printf("up button pressed\n");
		dPad = UP;
		//simonSays_state = SIMONSAYS_STATEONE;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_DOWN, down_latches))
	{printf("down button pressed\n");
		dPad = DOWN;
	    //simonSays_state = SIMONSAYS_STATETWO;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_A, down_latches)) {
		printf("'A' button pressed\n");
		simonSays_state = SIMONSAYS_EXIT;
	} else if (BUTTON_PRESSED(BADGE_BUTTON_B, down_latches)) {
		printf("'B' button pressed\n");

		simonSays_state = SIMONSAYS_EXIT;
	}
}

static void delay(int x){
	uint64_t first, second;
	first = rtc_get_ms_since_boot();
	second = rtc_get_ms_since_boot();
	int result = ((int)second - (int)first)/100;

	while(result<x){
		check_buttons();
		second = rtc_get_ms_since_boot();
		result = ((int)second - (int)first)/100;
		//printf("%d\n",result);
	}
}



static void showChoice(Choice c, int someTime){
	const struct asset2 *temp;
	int freq=0;
	switch (c){
		case UP:
			temp = upblue_p;
			freq = 440;
			break;
		case RIGHT:
			temp = rightyellow_p;
			freq = 340;
			break;
		case DOWN:
			temp = downgreen_p;
			freq = 240;
			break;
		case LEFT:
			temp = leftred_p;
			freq = 140;
			break;
		default:
			temp = alldark_p;
	}
	audio_out_beep(freq,500);
	FbClear();
	FbColor(WHITE);
	FbMove(16,0);
	FbImage2(temp, 0);
	FbSwapBuffers();
	dPad=NONE;
	delay(someTime);

}

static void playSequence(void){
	Turn *thisTurn = sequence;
	printf("in the play sequence method \n");
	int cycles = 0;
	bool notFirstRun = false;
	do{
		if(notFirstRun){thisTurn=thisTurn->next;}
		showChoice(thisTurn->choice,3);
		showChoice(NONE,1);
		notFirstRun = true;
		cycles++;
		checkin();
		check_buttons();
		FbPushBuffer();
		printf("%d\n",cycles);
	}
	while(thisTurn->next!=NULL);
}

static void roundAndRound(void){
	showChoice(UP,1);
	showChoice(RIGHT,1);
	showChoice(DOWN,1);
	showChoice(LEFT,1);
}
static bool playerTurn(void){
	Turn *thisTurn = sequence;
	printf("entering the dreaded abyss of player turn drama\n");
	bool notFirstRun = false;
	do{
		if(notFirstRun){thisTurn=thisTurn->next;}
		notFirstRun = true;
		bool success = false;
		for(int i=0;i<3;i++){
			check_buttons();
			if(thisTurn->choice==dPad){
				success = true;
				break;
			}
			delay(10);
		}
		if(!success){return false;}
	}
	while(thisTurn->next!=NULL);
	printf("got them all right!\n");
	return true;
}

static void simonSaysPlayGames(void){
	printf("entering the game loop of doom\n");
	playSequence();
	showChoice(NONE,5);
	roundAndRound();
	roundAndRound();
	if(playerTurn()){
		newTurn();
		roundAndRound();
		roundAndRound();
	}
	else{
		printf("----user lost!!!----\n");
		simonSays_state	= SIMONSAYS_INIT;
    }

}
static void eatYourDead(Turn* t){
	if(t->next!=NULL){
		eatYourDead(t->next);
	}
	free(t);
	}


static void simonSays_exit(void){
	simonSays_state = SIMONSAYS_INIT; /* So that when we start again, we do not immediately exit */
	eatYourDead(sequence);
	pop_app();
}

/* You will need to rename myprogram_cb() something else. */
void simonSays_cb(__attribute__((unused)) struct badge_app *app){
	switch (simonSays_state) {
	case SIMONSAYS_INIT:
		simonSays_init();
		break;
	case SIMONSAYS_GAMELOOP:
		simonSaysPlayGames();
		break;
	case SIMONSAYS_PLAY:
		playSequence();
		break;
	case SIMONSAYS_SPIRAL:
		roundAndRound();
		break;
	case SIMONSAYS_ADD:
		newTurn();
		break;

	case SIMONSAYS_EXIT:
		simonSays_exit();
		break;
	default:
		break;
	}
}

