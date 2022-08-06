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
#include "../../des_controller/src/des-mva.h"

int main(int argc, char *argv[]) {

	setvbuf(stdout, NULL, _IOLBF, 0);
	setvbuf(stdin, NULL, _IOLBF, 0);

	int chid;

	int rcvid;
	//char message[50];
	int go = 1;
	int x = 0;
	Display display;

	/* Phase I  */

	//Call ChannelCreate() to create a channel for the controller process to attach
	chid = ChannelCreate(0);
	if (chid == -1) {
		perror("Failure to create channel.");
		//On Failure: print error message and EXIT_FAILURE
		exit(EXIT_FAILURE);
	}
	//Print display's PID; controller needs to know this PID

	printf("\ndisplay PID: %ld", getpid());
	fflush(stdout);

	/* Phase II  */

	//while( TRUE )
	while (go) {

		//Call MsgReceive() to receive Display object from controller
		rcvid = MsgReceive(chid, &display, sizeof(display), NULL);
		//Call MsgReply(), sending EOK back to the controller
		MsgReply(rcvid, EOK, &x, sizeof(x));

		switch (display.message_t) {
		//Print person has been scanned entering (or leaving) the building and display the person's ID
		//IF message == ID_SCAN THEN
		case WEIGHT_SCAN:

			printf("%s : %d\n", display.message, display.data);
			fflush(stdout);
			//ELSE IF message = WEIGHED THEN
			break;
		case ID_SCAN:
			printf("%d : %s\n", display.data, display.message);
			fflush(stdout);
			break;

		case REG_MSG:
			printf("%s\n", display.message);
			fflush(stdout);
			break;
		case EXIT_MSG:
			go = 0;
			break;

		default:
			break;
		}

		/* Phase III  */
	}
	//Call ChannelDestroy() to destroy the channel that controller attaches to
	ChannelDestroy(chid);
	puts("EXIT_SUCCESS");
	return EXIT_SUCCESS;
}
