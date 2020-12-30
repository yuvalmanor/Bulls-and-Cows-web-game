#include "BullsAndCows.h"

int playGame(char* username) {
	int retVal;
	
	

	/*<---send server username--->*/

	// get respond 
	//if denied - quit
	//else continue
	/*while (1) {
		//get message from server
		//check message
		//act according to message
		//if MAIN_MANU:
		//main_manu();
		//else if 

	}*/
	return 0;
}

int playerChoice() {
	char option;
	int t;

	printf("Choose what to do next:\n");
	printf("1. Play against another client\n");
	printf("2. Quit\n");
	option = getchar();
	printf("option:%c\n", option);
	while (option != '1' && option != '2') {
		printf("Invalid option. Try again\n");
		option = getchar();
		option = getchar();
	}
	if ('1' == option)
		return 1;
	else if ('2' == option)
		return 2;
}