#include "BullsAndCows.h"

int playGame(char* username, SOCKET c_socket) {
	int retVal, choice;
	
	choice = playerChoice();
	if (1 == choice) {
		if (NOT_SUCCESS == setup(username, c_socket))
			return NOT_SUCCESS;
	}
	else if (2 == choice) {
		//quit();
		return 1;
	}


	

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
int setup(char* username, SOCKET m_socket) {
	char *clientRequest=NULL;
	int sendRes;
	clientRequest = prepareMsg("CLIENT_REQUEST:", username);
	if (NULL == clientRequest) return NOT_SUCCESS;
	sendRes = SendString(clientRequest, m_socket);
	if (TRNS_FAILED == sendRes) {
		free(clientRequest);
		return NOT_SUCCESS;
	}
	//after send, should free(clientRequest)?
	return SUCCESS;
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