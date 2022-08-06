#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <string.h>
#include <unistd.h>

#include "../../des_controller/src/des-mva.h"

char input_event[10];
char input[10];
int coid = 0;
int n = 0;
int code = 0;

void get_input() {
	fgets(input_event, sizeof(input_event), stdin);

	if (sscanf(input_event, "%s", &n) != 1) {
		/* error parsing input */
	}
}
void send_msg_server() {
	if (MsgSend(coid, &input, sizeof(input), &n, sizeof(int)) == -1L) {
		fprintf(stderr, "Error during MsgSend\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

}

void send_person_server(Person person) {
	if (MsgSend(coid, &person, sizeof(person), &n, sizeof(int)) == -1L) {
		fprintf(stderr, "Error during MsgSend\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

}

int main(int argc, char *argv[]) {

	//clear the buffer
	setvbuf(stdout, NULL, _IOLBF, 0);
	setvbuf(stdin, NULL, _IOLBF, 0);

	/*Phase I
	 Get controller's PID from command-line arguments.
	 */
	if (argc != 2) {
		printf("Bad Usage: incorrect number of arguments");
		//On Failure: print usage message and EXIT_FAILURE
		exit(EXIT_FAILURE);
	}

	//	Call ConnectAttach() to attach to controller's channel
	coid = ConnectAttach(ND_LOCAL_NODE, atoi(argv[1]), 1, _NTO_SIDE_CHANNEL, 0);
	if (coid == -1) {
		//On Failure: print error message and EXIT_FAILURE
		fprintf(stderr, "Couldn't ConnectAttach\n");
		perror(NULL);
		sleep(10);
		exit(EXIT_FAILURE);
	}
	Person person;
	//Phase II
	int finished = 1;
	int auto_input = 1;
	while (finished == 1) {
		//Prompt User for DES input-event (printf())
		fflush(stdout);
		puts(
				"Enter the event type ( ls = left scan, rs = right scan, ws = weight scan, lc = left close, lo = left open, rc = right close, ro =  right open, gll = guard left lock, glu = guard left unlock, grl = guard right locked, gru, guard right unlocked\n exit or EXIT = exit program\n\n");
		//Get input-event from User (scanf())
		if (auto_input) {
			get_input();
		}

		//IF input-event == "ls" THEN
		if (strcmp(input_event, "ls\n") == 0) {
			//Left Scan
			puts("\n(left scan)Enter ID: ");

			get_input();
			code = atoi(input_event);

			person.id = code;
			person.event = LS;
			send_person_server(person);
		} else if (strcmp(input_event, "rs\n") == 0) {
			//Right Scan.
			puts("\n(right scan) Enter ID: ");
			get_input();
			code = atoi(input_event);
			person.id = code;
			person.event = RS;
			send_person_server(person);
		}
		//weight Scale
		else if (strcmp(input_event, "ws\n") == 0) {
			//TODO: apply what you know and repeat for "ws", with the notable exception: prompt User for person's weight
			puts("\n(Weight scale)Enter Weight: ");
			get_input();
			code = atoi(input_event);
			person.weight = code;
			person.event = WS;
			send_person_server(person);
			person.event = LC;
			send_person_server(person);

		}
		// left-open
		else if (strcmp(input_event, "lo\n") == 0) {
			person.event = LO;
			send_person_server(person);
		}
		// right-open
		else if (strcmp(input_event, "ro\n") == 0) {
			person.event = RO;
			send_person_server(person);
			person.event = RC;
			send_person_server(person);
		}
		// left-close
		else if (strcmp(input_event, "lc\n") == 0) {
			person.event = LC;
			send_person_server(person);
		}
		//  right close
		else if (strcmp(input_event, "rc\n") == 0) {
			person.event = RC;
			send_person_server(person);

		}
		// guard right unlock
		else if (strcmp(input_event, "gru\n") == 0) {
			person.event = GRU;
			send_person_server(person);
		}
		// guard right lock
		else if (strcmp(input_event, "grl\n") == 0) {
			person.event = GRL;
			send_person_server(person);
		}
		// guard left lock
		else if (strcmp(input_event, "gll\n") == 0) {
			person.event = GLL;
			send_person_server(person);
		}
		// guard left unlock
		else if (strcmp(input_event, "glu\n") == 0) {
			person.event = GLU;
			send_person_server(person);
		}
		// exit
		else if ((strcmp(input_event, "exit\n") == 0)
				|| (strcmp(input_event, "EXIT\n") == 0)) {
			person.event = XIT;
			send_person_server(person);
			finished = 0;
			break;

		} else {

		}

		//Send Person object to controller (server); no message from controller is returned to client.

		//Phase III
		//Call ConnectDetach() to detach from controller's channel
	}

	ConnectDetach(coid);
	puts("EXIT_SUCCESS");
	;
	return EXIT_SUCCESS;
}
