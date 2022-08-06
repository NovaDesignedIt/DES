/*
 * des-mva.h
 */

#ifndef DES_MVA_H_
#define DES_MVA_H_

#define NUM_USER_INPUTS 12
typedef enum {
	LS = 0,
	RS = 1,
	WS = 2,
	LO = 3,
	LC = 4,
	RO = 5,
	RC = 6,
	GRU = 7,
	GRL = 8,
	GLU = 9,
	GLL = 10,
	XIT = 11
} UsrIn;

/**
 * 
 *  Messages corresponding to each user input event.. not each state transition or transition input. but what the user gets from 
 * 	des_inputs.
 */

const char *inMessage[NUM_USER_INPUTS] = { "entering", "leaving", "\nweight: ",
		"\nLeft door opening", "\nLeft door closing", "\nRight door opening",
		"\nRight door closed", "\nRight Guard Unlocked", "\nRight Guard Locked",
		"\nLeft Guard Unlocked", "\nLeft Guard Locked", "\nExiting :)" };
/**
 * STATES
 * the states used in our FSM to determine which door the DES will attend to. 
 */

#define NUM_STATES 6
typedef enum {
	/*START_STATE*/S1 = 0,
	/*READY_STATE*/S2 = 1,
	/*LEFT_STATE */S3 = 2,
	/*RIGHT_STATE*/S4 = 3,
	/*EXIT_STATE */S5 = 4,

	/*STOP_STATE */S6 = 5 // QUESTIONING THIS???
} State;

#define NUM_INPUTS 5
typedef enum {
	/*
	 input events with [ ] are automatic. not all inputs trigger a state change in the machine. however we can point out critical ones that can trigger a state change in the machine
	 for example : if you are in ready state and you get rs # nothing will happen because that input does not trigger an event because in ready state the machine disregards it.
	 but. if for example we are in right state rs # would trigger the machine to revert to ready because in RIGHT_STATE the machine needs to be deactivated.
	 example 2. if we are in ready state and no one entering the building.  gru command will allow the machine to move from ready to RIGHT_STATE, but in the RIGHT_STATE the only acceptable command
	 would 'ro' after the fact. once the machine has recongnized that the rc has happened it will listen for 'rs#' | 'exit' exclusivly .. does this mean that if someone doesn't bagde out they're violating something???..
	 example 3. if we are in LEFT_STATE then the machine will await the weight scan and close the door. then WAIT for the 'gll' command to be back to ready state.
	 */
	LEFT_GUARD_DEACTIVATE = 0, // ENTER - entering gate 		- ls # - > glu -> lo
	LEFT_GUARD_ACTIVATE = 1, // ENTER - leaving gate 		- [ ws # -> lc ] -> gll
	RIGHT_GUARD_DEACTIVATE = 2,	// EXIT - entering gate 		- gru -> [ ro -> rc ] 
	RIGHT_GUARD_ACTIVATE = 3,	// EXIT - leaving gate			- grl -> rs #
	EXIT_BUTTON = 4				// EXIT the machine 			- exit

} Input;

/* 
 Refered to my Compiler notes to build a transition table. The Stop state currently has no utility because when the machine is in a
 left state and doesn't receive it's acceptable commands it ignores them. if person has scan and the left guard is unlocked...
 simply ignore the GRU command if it appears. S3 simply says: I'm looking for either these 3 commands in sequence..  ( [ ws # -> lc ] -> gll )
 or exit state if the machine only lets in 1 person at a time. If it's tending to someone entering. the commands  it doesn't listen for will simply
 have no effect.
 */
const State transition_table[NUM_STATES][NUM_INPUTS] = {
/*		  			LGD		LGA		RGD		RGA		XIT		*/
/*START_STATE	*/{ S2, S2, S2, S2, S2 },
/*READY_STATE	*/{ S3, S2, S4, S2, S5 },
/*LEFT_STATE	*/{ S3, S2, S3, S3, S5 },
/*RIGHT_STATE	*/{ S4, S4, S4, S2, S5 },
/*EXIT_STATE	*/{ S1, S1, S1, S1, S1 },

/*STOP_STATE 	?*/{ S1, S1, S1, S1, S1 } // don't really need this this state. useless.

};

/*
 each state has a set of conditions so that the input of the user will determine a state change. by defining enums to these conditions
 it makes for better code readability. a door can open or closed. a person can be scanned or not_scanned or weighed or not_weighed.
 the controller will check these conditions in certain states.
 */
typedef enum {

	WEIGHED = 1,
	NOT_WEIGHED = 0,
	SCANNED = 1,
	NOT_SCANNED = 0,
	OPEN = 1,
	CLOSED = 0

} Condition;

#define NUM_OUTPUTS 6
/*
 typedef enum
 {
 START_MSG = 0,
 READY_MSG = 1,
 LEFT_DOWN_MSG = 2,
 RIGHT_DOWN_MSG = 3,
 EXIT_MSG = 6
 } Output;*/

/*
 Person struct to handle the message passaing between the controller server side and input client side.
 this struct passes a user event to the controller to handle the event in the switch branch.
 */
typedef struct {
	int id;
	int weight;
	UsrIn event;
} Person;

/*
 enums message_type so that the display can determine what type of message came through and if it should display weight or an ID
 */

typedef enum {
	WEIGHT_SCAN = 0, ID_SCAN = 1, REG_MSG = 2, EXIT_MSG = 4,
} message_type;

/*
 Display Struct for sending a message to the display server it contains message_type to determine what will the display do with it.
 it contains a person so that when ever we handle input. we can just forward the person object to the display to handle it's properties(weight, id..)
 the message that will be displayed. they're in order by user_inputs.
 */
typedef struct {
	message_type message_t;
	int data;
	char message[50];
} Display;

/*
 the display object needed string message corresponding to each user event.
 */

const char * outMessage[NUM_OUTPUTS] = { "START_STATE", "READY_STATE",
		"LEFT_GATE IN USE", "RIGHT_GATE IN USE", "EXIT", "Stop Message." };
/**
 * 
 * 	Function prototype for the state transition table.
 */
State get_next_state(State current_state, Input input);

#endif /* DES_MVA_H_ */
