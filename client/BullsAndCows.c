#include "BullsAndCows.h"

int playGame(char* username, SOCKET c_socket, SOCKADDR_IN clientService, char* ip, int portNumber) {
	char* p_clientMsg = NULL;
	int status, choice;
	Message* p_serverMsg = NULL;

	while (1) {
		/*<---send server username--->*/
		if (SUCCESS != setup(username, c_socket, clientService, ip, portNumber)) {
			return EXIT;
		}
		while (1) { //I changed the case of START_AGAIN to continue, is this ok?
			printf("Getting SERVER_MAIN_MENU\n");
			status = getMessage(c_socket, &p_serverMsg, RESPONSE_WAITTIME);
			if (TRNS_SUCCEEDED != status) { //Yuval checked it. comment still here for flag the place
				if (SUCCESS != checkTRNSCode(status, ip, portNumber, c_socket, clientService))
					return NOT_SUCCESS;
				else
					continue;
			}
			if (strcmp(p_serverMsg->type, "SERVER_MAIN_MENU")) {
				printf("Message invalid. This is the message recived: %s", p_serverMsg->type);
				free(p_serverMsg);
				return NOT_SUCCESS;
			}
			printf("Got SERVER_MAIN_MENU\n");
			free(p_serverMsg);
			choice = menu(MAIN, ip, 0);
			if (1 == choice) {
				status = start(c_socket);
				if (NOT_SUCCESS == status)
					return NOT_SUCCESS;
				else if (SUCCESS == status)
					continue;
				else if (START_AGAIN == status)
					continue;
			}
			else if (2 == choice) {
				p_clientMsg = prepareMsg("CLIENT_DISCONNECT", NULL);
				if (NULL == p_clientMsg) return NOT_SUCCESS;
				status = SendString(p_clientMsg, c_socket);
				if (TRNS_FAILED == status) {
					free(p_clientMsg);
					return NOT_SUCCESS;
				}
				shutdownConnection(c_socket);
				return SUCCESS;
			}
		}
	}
	
	return 0;
}
int setup(char* username, SOCKET c_socket, SOCKADDR_IN clientService, char* ip, int portNumber) {
	char* p_clientMsg = NULL;
	int status, choice;
	Message* p_serverMsg = NULL;

	p_clientMsg = prepareMsg("CLIENT_REQUEST:", username);
	if (NULL == p_clientMsg) return NOT_SUCCESS;
	while (1)
	{

		status = SendString(p_clientMsg, c_socket);
		if (TRNS_FAILED == status) {
			free(p_clientMsg);
			printf("send failed(setup)");
			return NOT_SUCCESS;
		}
		printf("CLIENT_REQUEST sent\nHoping to get SERVER_APPROVED\n");
		status = getMessage(c_socket, &p_serverMsg, RESPONSE_WAITTIME);
		if (TRNS_SUCCEEDED != status) { //fix it to right handeling
			free(p_clientMsg);
			if (SUCCESS != checkTRNSCode(status, ip, portNumber, c_socket, clientService))
				return NOT_SUCCESS;
			else
				continue;
		}
		printf("Got message from server: %s\n", p_serverMsg->type);
		if (!strcmp(p_serverMsg->type, "SERVER_APPROVED")) {
			free(p_clientMsg);
			free(p_serverMsg);
			return SUCCESS;
		}
		else if (!strcmp(p_serverMsg->type, "SERVER_DENIED")) { // TODO solve error 10038 thing
			free(p_serverMsg->deniedReason);
			free(p_serverMsg);
			setsockopt(socket, SOL_SOCKET, SO_DONTLINGER, 0, 0);
			confirmShutdown(c_socket);
			choice = menu(DENIED, ip, portNumber);
			if (1 == choice) {
				c_socket = INVALID_SOCKET;
				c_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (c_socket == INVALID_SOCKET)
				{
					printf("Error at socket( ): %ld\n", WSAGetLastError());
					return NOT_SUCCESS;
				}
				if (EXIT == makeConnection(c_socket, clientService, ip, portNumber))
					return EXIT;
				else
					continue;
			}
			else if (2 == choice)
				return EXIT;
		}
	}


	return SUCCESS;
}
int start(SOCKET c_socket) {
	char* p_clientMsg = NULL;
	int status;
	Message* p_serverMsg = NULL;

	p_clientMsg = prepareMsg("CLIENT_VERSUS", NULL);
	if (NULL == p_clientMsg) return NOT_SUCCESS;
	status = SendString(p_clientMsg, c_socket);
	free(p_clientMsg);
	if (TRNS_FAILED == status) return NOT_SUCCESS;
	status = getMessage(c_socket, &p_serverMsg, USER_WAITTIME);
	if (TRNS_DISCONNECTED == status || TRNS_TIMEOUT == status) return START_AGAIN;//need to change this to right handeling
	else if (TRNS_FAILED == status) return NOT_SUCCESS;
	if (!strcmp(p_serverMsg->type, "SERVER_INVITE")) {
		free(p_serverMsg);
		return GameIsOn(c_socket);
		
	}
	//if server message is SERVER_NO_OPPONENTS
	else {
		free(p_serverMsg);
		printf("No opponents were found.\n");
		return START_AGAIN;
	}
	

}
int GameIsOn(SOCKET c_socket) {
	char* p_clientMsg = NULL, * p_userChoice = NULL, *p_guess = NULL;
	int status;
	Message* p_serverMsg = NULL;

	printf("Game is on !\n");
	status = getMessage(c_socket, &p_serverMsg, RESPONSE_WAITTIME);
	if (TRNS_DISCONNECTED == status || TRNS_TIMEOUT == status) return START_AGAIN;//need to change this
	else if (TRNS_FAILED == status) return NOT_SUCCESS;
	if (strcmp(p_serverMsg->type, "SERVER_SETUP_REQUEST")) {
		if (START_AGAIN == opponentQuit(p_serverMsg->type, p_serverMsg, c_socket))
			return START_AGAIN;
	}
	//<--- if message is SERVER_SETUP_REQUEST --->
	printf("Choose your 4 digits:\n");
	p_userChoice=chooseNumber();
	p_clientMsg = prepareMsg("CLIENT_SETUP:", p_userChoice);
	free(p_userChoice);
	if (NULL == p_clientMsg) return NOT_SUCCESS;
	status= SendString(p_clientMsg, c_socket);
	free(p_clientMsg);
	if (TRNS_FAILED == status) return NOT_SUCCESS;
	free(p_serverMsg);

	while (1) {
		//<--------- Wait to get SERVER_PLAYER_MOVE_REQUEST from server ----->
		status = getMessage(c_socket, &p_serverMsg, RESPONSE_WAITTIME);
		if (TRNS_DISCONNECTED == status || TRNS_TIMEOUT == status) return START_AGAIN;//need to change this
		else if (TRNS_FAILED == status) return NOT_SUCCESS;
		if (START_AGAIN == opponentQuit(p_serverMsg->type,p_serverMsg,c_socket)) return START_AGAIN;
		//<--- check if message is SERVER_PLAYER_MOVE_REQUEST --->
		if (strcmp(p_serverMsg->type, "SERVER_PLAYER_MOVE_REQUEST")) {
			printf("Message invalid. This is the message recived: %s", p_serverMsg->type);
			free(p_serverMsg);
			return NOT_SUCCESS;
		}
		free(p_serverMsg); p_serverMsg = NULL;
		printf("Choose your guess:\n");
		p_guess = chooseNumber();
		p_clientMsg = prepareMsg("CLIENT_PLAYER_MOVE:", p_guess);
		free(p_guess);
		if (NULL == p_clientMsg) return NOT_SUCCESS;
		//<-------Send guess to the server---------->
		status = SendString(p_clientMsg, c_socket);
		free(p_clientMsg);
		if (TRNS_FAILED == status) return NOT_SUCCESS;
		//<-------Get round results from the server------>
		status = getMessage(c_socket, &p_serverMsg, USER_WAITTIME);//USER_WAITTIME because server waits for opponent to enter his move
		if (TRNS_DISCONNECTED == status || TRNS_TIMEOUT == status) return START_AGAIN;//need to change this
		else if (TRNS_FAILED == status) return NOT_SUCCESS;
		if (START_AGAIN == opponentQuit(p_serverMsg->type,p_serverMsg,c_socket)) return START_AGAIN;
		//<---Continue according to GAME RESULTS--->
		if (!strcmp(p_serverMsg->type, "SERVER_GAME_RESULTS")) { //If no one wins
			gameResults(p_serverMsg, MID_GAME);
			free(p_serverMsg);
			continue;
		}
		if (!strcmp(p_serverMsg->type, "SERVER_WIN")) { //If someone wins
			gameResults(p_serverMsg, WIN);
			free(p_serverMsg);
			return SUCCESS;
		}
		if (!strcmp(p_serverMsg->type, "SERVER_DRAW")) { //If it's a tie
			gameResults(p_serverMsg, TIE);
			free(p_serverMsg);
			return SUCCESS;
		}
		
		
	}
	
}
int playerChoice() {
	char* choice = NULL, option;
	int res=0;

	if (NULL == (choice = malloc(PAGE_SIZE))) {
		printf("Fatal error: memory allocation failed (playerChoice).\n");
		return NULL;
	}
	while (1) {
		scanf_s("%s", choice, PAGE_SIZE);
		if (1 != strlen(choice)) {
			printf("Invalid choice. Try again.\n");
			continue;
		}
		option = choice[0];
		if (!isdigit(option) || option != '1' && option != '2') {
			printf("Invalid choice. Try again.\n");
			continue;
		}
		break;
	}
	free(choice);
	if ('1' == option)
		return 1;
	else
		return 2;
}

int checkTRNSCode(int TRNSCode, char* ip, int portNumber, SOCKET c_socket, SOCKADDR_IN clientService) {
	int choice;
	
	if (TRNSCode == TRNS_FAILED) {
		return NOT_SUCCESS;
	}
	else // (TRNSCode == TRNS_DISCONNECTED || TRNSCode == TRNS_TIMEOUT) 
	{	
		choice = menu(FAILURE, ip, portNumber);
		if (TRNS_DISCONNECTED == TRNSCode) 
			confirmShutdown(c_socket);
		if (1 == choice) {
			c_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (c_socket == INVALID_SOCKET)
			{
				printf("Error at socket( ): %ld\n", WSAGetLastError());
				return NOT_SUCCESS;
			}
			if (EXIT == makeConnection(c_socket, clientService, ip, portNumber))
				return EXIT;
			else
				return SUCCESS;
		}
		else //choice==2
			return EXIT;
	}

}
int menu(int menu, char* ip, int portNumber) {
	int choice;

	switch (menu) {
	case MAIN:
		printf("Choose what to do next:\n");
		printf("1. Play against another client\n");
		printf("2. Quit\n");
		break;
	case FAILURE:
		printf("Failed connecting to server on %s:%d.\n", ip, portNumber);
		printf("Choose what to do next:\n");
		printf("1. Try to reconnect\n");
		printf("2. Exit\n");
		break;
	case DENIED:
		printf("Server on %s:%d denied the connection request.\n",ip,portNumber);
		printf("Choose what to do next:\n");
		printf("1. Try to reconnect\n");
		printf("2. Exit\n");
		break;
	}

	choice = playerChoice();
	return choice;

}
char* chooseNumber() {
	char* guess = NULL;
	int i = 0, flag = 1;

	if (NULL == (guess = malloc(PAGE_SIZE))) {
		printf("Fatal error: memory allocation failed (chooseNumber).\n");
		return NULL;
	}
	
	while (flag) {
		scanf_s("%s", guess, PAGE_SIZE);
		if (4 != strlen(guess)) {
			printf("Invalid number of digits. Try again.\n");
			continue;
		}
		for (i = 0; i < 4; i++) {
			if (!isdigit(guess[i])) {
				printf("Please enter only digits. Try again.\n");
				break;
			}
			char* first = strchr(guess, guess[i]);
			char* last = strrchr(guess, guess[i]);
			if (first != last) {
				printf("Please enter each digit once. Try again.\n");
				break;
			}
			
		}
		if (i == 4)
			flag = 0;
	}
	return guess;
}
int opponentQuit(char* message, Message* serverMsg, SOCKET c_socket) {
	int status;
	if (!strcmp(message, "SERVER_OPPONENT_QUIT")) {
		printf("Opponent quit.\n");
		status = getMessage(c_socket, &serverMsg, RESPONSE_WAITTIME); //need to check errors
		if (strcmp(serverMsg->type, "SERVER_MAIN_MENU")) {
			printf("Message invalid. This is the message recived: %s", serverMsg->type);
			free(serverMsg);
			return NOT_SUCCESS;
		}
		free(serverMsg);
		return START_AGAIN;
	}
	else
	{
		return CONTINUE;
	}
		
}
void gameResults(Message* message, int status) {

	if (TIE == status) {
		printf("It's a tie\n");
		return;
	}
	if (MID_GAME == status) {
		printf("Bulls: %c\n", message->bulls);
		printf("Cows: %c\n", message->cows);
		printf("%s played: %s\n", message->username, message->guess);
		
	}
	else if (WIN == status) {
		printf("%s won!\n", message->username);
		printf("Opponent number was %s\n", message->guess);
	}
	free(message->username);
	free(message->guess);
}
int makeConnection(SOCKET c_socket, SOCKADDR_IN clientService, char* ip, int portNumber) {
	int choice;

	while (1) {
		if (connect(c_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {

			choice = menu(FAILURE, ip, portNumber);
			if (1 == choice) {
				resourcesManager(c_socket, 0);
				continue;
			}
			else if (2 == choice)
			{
				resourcesManager(c_socket, CLEAN);
				return EXIT;
			}

		}
		else
			break;

	}
	return SUCCESS;
}
void resourcesManager(SOCKET c_socket, int WSACleanFlag) {

	if (NULL != c_socket) {
		if (closesocket(c_socket) == SOCKET_ERROR)
			printf("Failed to close clientSocket in resourcesManager. error %ld\n", WSAGetLastError());
	}
	if (CLEAN == WSACleanFlag) {
		if (WSACleanup() == SOCKET_ERROR)
			printf("Failed to close Winsocket in resourcesManager. error %ld\n", WSAGetLastError());
	}


}