#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/netmgr.h>
#include "des-mva.h"

//Message passing variables Variables
int chid;
int rcvid;
int x = 0;
int go = 1;

//person and display struct

//first dummy input.
Input input = RIGHT_GUARD_ACTIVATE;

//We will want to track the current state and previous state.
State current_state = S1;

//these are our conditional variables that will determin if the machine input is valid.
Condition door = CLOSED;
Condition left_guard = CLOSED;
Condition right_guard = CLOSED;
Condition id = NOT_SCANNED;
Condition weighed = NOT_WEIGHED;

State get_next_state(State current_state, Input input) {
	return transition_table[current_state][input];
}

int main(int argc, char *argv[]) {

	//clear the buffer
	setvbuf(stdout, NULL, _IOLBF, 0);
	setvbuf(stdin, NULL, _IOLBF, 0);

	Person person;

	Display display;
	/* PHASE 0 */

	//command line arguments
	if (argc != 2) {
		printf("Missing PID to display\n");
		exit(EXIT_FAILURE);
	}

	/* PHASE I */

	//Get display's PID from command-line arguments
	//convert pid and check length
	char * pid_arg = argv[1];
	while (pid_arg[x] != '\0') {
		x++;
		if (x > 10) {
			printf("invalid argument");

			//On Failure: print usage message and EXIT_FAILURE
			exit(EXIT_FAILURE);
		}
	}

	long pid_desplay = atoi(argv[1]);
	printf("%ld", pid_desplay);
	fflush(stdout);

	//Call ChannelCreate() to create a channel for the inputs process to attach
	chid = ChannelCreate(0);
	if (chid == -1) {
		perror("Failure to create channel.");

		//On Failure: print error message and EXIT_FAILURE
		exit(EXIT_FAILURE);
	}

	//Call ConnectAttach() to attach to display's channel
	int coid = 0;
	coid = ConnectAttach(ND_LOCAL_NODE, atoi(argv[1]), 1, _NTO_SIDE_CHANNEL, 0);
	if (coid == -1) {
		//On Failure: print error message and EXIT_FAILURE
		fprintf(stderr, "Couldn't ConnectAttach\n");
		perror(NULL);
		sleep(10);
		exit(EXIT_FAILURE);
	}

	//Print controller's PID; inputs needs to know this PID
	printf("\ncontroller pid %ld", getpid());
	fflush(stdout);

	/* Phase II */
	int n = 0;

	//FSM
	while (go == 1) {

		//WHAT'S MY STATE.
		printf("\nSTATE: %s", outMessage[current_state]);
		fflush(stdout);

		if (current_state != S1) {
			//Call MsgReceive() to receive Person object from inputs
			rcvid = MsgReceive(chid, &person, sizeof(person), NULL);
			MsgReply(rcvid, EOK, &n, sizeof(int));

		}

		if (person.event == XIT) {
			/*  STATE CHANGE */
			current_state = get_next_state(current_state, EXIT_BUTTON);

		}

		//TODO - get input event from Person object and advance state machine to next accepting state (or error state)
		switch (current_state) {

		case S1:
			//if in start state what ever state will trigger it in the ready state.
			current_state = get_next_state(current_state, LEFT_GUARD_ACTIVATE);
			strcpy(display.message, "");
			display.message_t = REG_MSG;
			break;

			/** STATE 2 <READY STATE> */
		case S2:
			//the Machine is in S2 but also waiting for action.
			if (left_guard == CLOSED && right_guard == CLOSED
					&& id == NOT_SCANNED) {
				puts("\nWaiting for person...");
			}
			//LS IS THE MAIN POINT OF ENTRY FOR THE LEFT STATE.
			if (person.event == LS) {
				if (left_guard == CLOSED && right_guard == CLOSED
						&& id == NOT_SCANNED) {
					// if the id isn't scanned. then scan it.
					id = SCANNED;
					//set the display message;
					strcpy(display.message, inMessage[LS]);
					display.message_t = ID_SCAN;
					display.data = person.id;
					//printf("*** INTEGER ** : %d",display.person->id);

				}
				//GRU IS A n'OTHER MAIN POINT OF INPUT FOR THE RIGHT STATE
			} else if (person.event == GLU) {
				if (left_guard == CLOSED && right_guard == CLOSED) {
					left_guard = OPEN;

					//set the display message;
					strcpy(display.message, inMessage[GLU]);
					display.message_t = REG_MSG;

				} else {
					strcpy(display.message, "\n");
					display.message_t = REG_MSG;
				}

				break;

				//if LO is given and under the right conditions assign input.
			} else if (person.event == LO) {
				if (id == SCANNED && left_guard == OPEN
						&& /*GATE_CONDITION*/door == CLOSED) {

					//SET THE INPUT.
					door = OPEN;
					/*
					 * if you get here it's because you scanned successfully you unlocked a locked gate
					 * now the gate knows it's in a left state and tends to the left state. effectivly dissarming the left gate.
					 */

					/**
					 * STATE CHANGE
					 */
					current_state = get_next_state(current_state,
							LEFT_GUARD_DEACTIVATE);
					//set the display message;
					strcpy(display.message, inMessage[LO]);
					display.message_t = REG_MSG;

				}

			}
			/**
			 *GRU is only valid if both guards are closed..
			 *  you can't go LS # -> GLU -> GRU ILLEGAL. the state machine wont advance and no output
			 *  will be provided. both gates are closed so the S2 state condition is checked.
			 */
			else if (person.event == GRU) {
				if (left_guard == CLOSED && right_guard == CLOSED
						&& id == NOT_SCANNED) {

					right_guard = OPEN;
					//set the display message;
					strcpy(display.message, inMessage[GRU]);
					display.message_t = REG_MSG;
				}

			}
			/*
			 * Right Opens.. S2 checks if the right guard is open. and the door close.
			 * both conditions check out? the right door Opens.
			 */
			else if (person.event == RO) {
				if (right_guard == OPEN && door == CLOSED) {
					door = OPEN;
					//set the display message;
					strcpy(display.message, inMessage[RO]);
					display.message_t = REG_MSG;
					break;
				}
			}
			/**
			 *  RC Is automatically inputed and it triggers the RIGHT_STATE; also closes the door
			 */
			else if (person.event == RC) {
				if (right_guard == OPEN && door == OPEN) {

					door = CLOSED;

					/** STATE CHANGE */
					current_state = get_next_state(current_state,
							RIGHT_GUARD_DEACTIVATE);

					//set the display message;
					strcpy(display.message, inMessage[RC]);
					display.message_t = REG_MSG;

				}
				break;

			} else {
				strcpy(display.message, "\n");
				display.message_t = REG_MSG;

			}
			break;

			/** STATE 3 <LEFT_STATE> */
		case S3:

			//WEIGHT SCAN right to left close.
			if (person.event == WS) {
				/**
				 * if you've made it to this state then by default the door is open. so you just log the weight
				 */
				if (weighed == NOT_WEIGHED) {
					weighed = WEIGHED;
					strcpy(display.message, inMessage[WS]);
					display.message_t = WEIGHT_SCAN;
					display.data = person.weight;

				}
			} else if (person.event == LC) {
				/**
				 * this should be automatic as per requirement.
				 */
				door = CLOSED;
				strcpy(display.message, inMessage[LC]);
				display.message_t = REG_MSG;

			} else if (person.event == GLL) {
				if (door == CLOSED && weighed == WEIGHED) {

					/**
					 * STATE CHANGE
					 */
					left_guard = CLOSED;
					current_state = get_next_state(current_state,
							LEFT_GUARD_ACTIVATE);
					strcpy(display.message, inMessage[GLL]);
					display.message_t = REG_MSG;

					/**
					 * not cleaning up causing some buggies. . .  if you got here you got your weight successfully
					 * taken and we should clean up and flip the weight condition again....
					 */

					weighed = NOT_WEIGHED;
					id = NOT_SCANNED;

				}

			} else {
				strcpy(display.message, "\n");
				display.message_t = REG_MSG;

			}
			break;

			/** STATE 4 <RIGHT_STATE> */
		case S4:
			/**if you're here it's because you came from S2 so the right Guard is open.
			 the RIGHT_STATE only concerns itself with GRL and RS #
			 */
			if (person.event == GRL) {
				//close the right guard.
				right_guard = CLOSED;

				strcpy(display.message, inMessage[GRL]);
				display.message_t = REG_MSG;

			} else if (person.event == RS) {
				if (right_guard == CLOSED) {

					/**
					 * STATE CHANGE
					 */
					current_state = get_next_state(current_state,
							RIGHT_GUARD_ACTIVATE);

					strcpy(display.message, inMessage[RS]);
					display.message_t = ID_SCAN;
					display.data = person.id;
				}
			} else {
				strcpy(display.message, "");
				display.message_t = REG_MSG;
			}
			break;

			/** STATE 5 <EXIT STATE> */
		case S5:
			go = 0;
			strcpy(display.message, "");
			display.message_t = EXIT_MSG;

			if (MsgSend(coid, &display, sizeof(display), &x, sizeof(x))
					== -1L) {
				fprintf(stderr, "Error during MsgSend\n");
				perror(NULL);
				exit(EXIT_FAILURE);
			}
			ConnectDetach(coid);
			ChannelDestroy(chid);
			break;
		default:
			break;

		} //end of switch

		//send message to the display server if the go variable allows it so.. if it's not terminating the program.
		if (go == 1) {
			if (MsgSend(coid, &display, sizeof(display), &x, sizeof(x))
					== -1L) {
				fprintf(stderr, "Error during MsgSend\n");
				perror(NULL);
				exit(EXIT_FAILURE);
			}
		}

	} //end of while

	/** Phase III */

	//TODO - implement Phase III for the controller process
	puts("\nEXIT_SUCCESS");
	return EXIT_SUCCESS;
}

