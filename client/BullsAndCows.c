#include "BullsAndCows.h"

int playGame(char* username, SOCKET c_socket) {
	int retVal, choice;
	
	


	

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

char* prepareMsg(const char* msgType, char* str) {
	char* message = NULL;
	int messageLen = strlen(msgType) + strlen(str) + 2; //+2 for \n and \0
	if (NULL == (message = malloc(messageLen))) {
		printf("Fatal error: memory allocation failed (prepareMsg).\n");
		return NULL;
	}
	strcpy_s(message, messageLen, msgType);
	strcat_s(message, messageLen, str);
	strcat_s(message, messageLen, "\n");
	return message;

}