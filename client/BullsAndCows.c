#include "BullsAndCows.h"

int playGame(char* username, SOCKET c_socket, SOCKADDR_IN clientService, char* ip, int portNumber) {
	char* clientRequest = NULL, * recvMsg = NULL;
	int res, choice;
	Message* serverMsg = NULL;

	while (1) {
		/*<---send server username--->*/
		if (SUCCESS != setup(username, c_socket, clientService, ip, portNumber)) {
			resourcesManager(c_socket, CLEAN);
			return EXIT;
		}
		res = ReceiveString(&recvMsg, c_socket,15);
		/*HERE SHOULD CHECK IF TRNS_FAILED OR NOT AND TRNS_DISCONNECTED AND
		HANDLE IT, ALSO WHAT ABOUT TIMEOUT*/
		if (TRNS_SUCCEEDED != res) {
			if (SUCCESS != checkTRNSCode(res, ip, portNumber, c_socket, clientService))
				return NOT_SUCCESS;
			else
				continue;
		}
		serverMsg = messageDecoder(recvMsg);
		if (NULL == serverMsg) {
			free(serverMsg);
			return NOT_SUCCESS;
		}
		if (strcmp(serverMsg->type, "SERVER_MAIN_MENU")) {
			printf("Message invalid. This is the message recived: %s", serverMsg->type);
			free(serverMsg);
			return NOT_SUCCESS;
		}

		while (1) {
			choice = menu(MAIN, ip, 0);
		}
		/*
		while(1)
			choice = menu(MAIN, ip, 0);*/
		/*
		if (1==choice){
			start();
			#client send CLIENT_VERSUS
			#client recive message from server - need to wait 30 sec
			#if (serverMsg==SERVER_INVITE){ 
				printf("Game is on !\n")
				server send SERVER_SETUP_REQUEST ---->need to check if the message is SERVER_OPPONENT_QUIT
				printf("Choose your 4 digits:\n")
				player need to choose 4 digits
				client send to server CLIENT_SETUP
				while(message->type!=SERVER_WIN || message->type!=SERVER_DRAW)
					server sent SERVER_PLAYER_MOVE_REQUEST---->need to check if the message is SERVER_OPPONENT_QUIT
					printf("Choose your guess:\n")
					client send to server CLIENT_PLAYER_MOVE
					server send SERVER_GAME_RESULTS ---->need to check if the message is SERVER_OPPONENT_QUIT
					printf("Bulls: <bulls>\n
							Cows: <cows>\n
							<opponenr_username> played: <opponent_move>\n")
				if (serverMsg==SERVER_WIN) ->print winner message to user, continue; ---->need to check if the message is SERVER_OPPONENT_QUIT
				if (serverMsg==SERVER_DRAW) ->print draw message to user, continue; ---->need to check if the message is SERVER_OPPONENT_QUIT
			}
			if (serverMsg==SERVER_NO_OPPONENTS){ 
				continue;
			}
		}
		if (2==choice){
			client send to server CLIENT_DISCONNECT
			client disconnect from server
		}
		*/
		//continue;

	}


	

	
	return 0;
}

int start(SOCKET c_socket, SOCKADDR_IN clientService, char* ip, int portNumber) {
	char* clientRequest = NULL, * recvMsg = NULL;
	int res;
	Message* serverMsg = NULL;

	clientRequest = prepareMsg("CLIENT_VERSUS", NULL);
	if (NULL == clientRequest) return NOT_SUCCESS;
	res = SendString(clientRequest, c_socket);
	/*HERE SHOULD CHECK IF TRNS_FAILED OR NOT AND HANDLE IT, ALSO WHAT ABOUT TIMEOUT
	if (TRNS_FAILED == res) {
	free(clientRequest);
	return NOT_SUCCESS;
	}	*/
	//after send, should free(clientRequest)?
	res = ReceiveString(&recvMsg, c_socket, 30);
	if (TRNS_SUCCEEDED != res) {
		if (SUCCESS != checkTRNSCode(res, ip, portNumber, c_socket, clientService))
			return NOT_SUCCESS;
		else
			return START_AGAIN;
	}
	serverMsg = messageDecoder(recvMsg);
	if (NULL == serverMsg) {
		free(serverMsg);
		return NOT_SUCCESS;
	}
	if (!strcmp(serverMsg->type, "SERVER_INVITE")) {
		//GameIsOn();
		return SUCCESS;
	}
	else if (!strcmp(serverMsg->type, "SERVER_NO_OPPONENTS")) {
		printf("No opponents were found.\n");
		return START_AGAIN;
	}
	

}
int GameIsOn(SOCKET c_socket, SOCKADDR_IN clientService, char* ip, int portNumber) {
	char* clientRequest = NULL, * recvMsg = NULL;
	int res;
	Message* serverMsg = NULL;

	printf("Game is on !\n");
	res = ReceiveString(&recvMsg, c_socket, 15);
	if (TRNS_SUCCEEDED != res) {
		if (SUCCESS != checkTRNSCode(res, ip, portNumber, c_socket, clientService))
			return NOT_SUCCESS;
		else
			return START_AGAIN;
	}
	serverMsg = messageDecoder(recvMsg);
	if (NULL == serverMsg) {
		free(serverMsg);
		return NOT_SUCCESS;
	}
	if (!strcmp(serverMsg->type, "SERVER_OPPONENT_QUIT")) {
		printf("Opponent quit.\n");
		free(serverMsg);
		return START_AGAIN;
	}
	//<--- if message is SERVER_SETUP_REQUEST --->
	printf("Choose your 4 digits:\n");
	//takeGuess();

	/*printf("Game is on !\n")
		#server send SERVER_SETUP_REQUEST---- > need to check if the message is SERVER_OPPONENT_QUIT
		printf("Choose your 4 digits:\n")
		player need to choose 4 digits
		client send to server CLIENT_SETUP
		while (message->type != SERVER_WIN || message->type != SERVER_DRAW)
			server sent SERVER_PLAYER_MOVE_REQUEST---- > need to check if the message is SERVER_OPPONENT_QUIT
			printf("Choose your guess:\n")
			client send to server CLIENT_PLAYER_MOVE
			server send SERVER_GAME_RESULTS---- > need to check if the message is SERVER_OPPONENT_QUIT
			printf("Bulls: <bulls>\n
				Cows: <cows>\n
				<opponenr_username> played: <opponent_move>\n")
				if (serverMsg == SERVER_WIN)->print winner message to user, continue; ---- > need to check if the message is SERVER_OPPONENT_QUIT
					if (serverMsg == SERVER_DRAW)->print draw message to user, continue; ---- > need to check if the message is SERVER_OPPONENT_QUIT
		}*/
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
	if (NULL!=str)
		strcat_s(message, messageLen, str);
	strcat_s(message, messageLen, "\n");
	return message;

}
int setup(char* username, SOCKET c_socket, SOCKADDR_IN clientService, char* ip, int portNumber) {
	char* clientRequest = NULL, * recvMsg = NULL;
	int res, choice;
	Message* serverMsg = NULL;

	clientRequest = prepareMsg("CLIENT_REQUEST:", username);
	if (NULL == clientRequest) return NOT_SUCCESS;
	while (1)
	{

		res = SendString(clientRequest, c_socket);
		/*HERE SHOULD CHECK IF TRNS_FAILED OR NOT AND HANDLE IT, ALSO WHAT ABOUT TIMEOUT
		if (TRNS_FAILED == res) {
		free(clientRequest);
		return NOT_SUCCESS;
		}	*/
		//after send, should free(clientRequest)?
		res = ReceiveString(&recvMsg, c_socket, 15);
		if (TRNS_SUCCEEDED != res) {
			if (SUCCESS != checkTRNSCode(res, ip, portNumber, c_socket, clientService))
				return NOT_SUCCESS;
			else
				continue;
		}
		serverMsg = messageDecoder(recvMsg);
		if (NULL == serverMsg) {
			free(serverMsg);
			return NOT_SUCCESS;
		}
		if (!strcmp(serverMsg->type, "SERVER_APROVED")) {
			free(serverMsg);
			return SUCCESS;
		}
		else if (!strcmp(serverMsg, "SERVER_DENIED")) {
			free(serverMsg);
			choice = menu(DENIED, ip, portNumber);
			if (1 == choice) {
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
int checkTRNSCode(int TRNSCode, char* ip, int portNumber, SOCKET c_socket, SOCKADDR_IN clientService) {
	int choice;

	if (TRNSCode == TRNS_FAILED) {
		printf("Socket error while trying to write data to socket\n");
		return NOT_SUCCESS;
	}
	else if (TRNSCode == TRNS_DISCONNECTED || TRNSCode == TRNS_TIMEOUT) {
		choice = menu(FAILURE, ip, portNumber);
		if (1 == choice) {
			if (EXIT == makeConnection(c_socket, clientService, ip, portNumber))
				return EXIT;
			else
				return SUCCESS;
		}
		else if (2 == choice)
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
char* takeGuess() {
	char* guess = NULL;
	int i = 0;

	if (NULL == (guess = malloc(PAGE_SIZE))) {
		printf("Fatal error: memory allocation failed (prepareMsg).\n");
		return NULL;
	}
	
	while (1) {
		fgets(guess, PAGE_SIZE, stdin);
		if (4 != strlen(guess)) {
			printf("Too many digits. Try again.\n");
			continue;
		}
		for (i = 0; i < 4; i++) {
			if (!isdigit(guess[i])) {
				printf("Please enter only digits. Try again.\n");
				continue;
			}
		}
	}
	while (4 != strlen(guess)) {
		printf("Too many digits. Try again.\n");
		fgets(guess, PAGE_SIZE, stdin);
	}
	

}