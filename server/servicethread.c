/*
Description – An implementation of the thread that gives service to the clients
				that should be accepted to the game.
*/

#include "servicethread.h"

DWORD ServiceThread(void* lpParam) {
	ThreadParam* p_param;
	if (NULL == lpParam) {
		printf("Service thread can't work with NULL as parameters\n");
		return NOT_SUCCESS;
	}
	p_param = (ThreadParam*)lpParam;
	DWORD waitcode;
	SOCKET socket = p_param->socket;
	int playerOne=0, retVal, threadRetVal = 0, gameStatus = MAIN_MENU;
	int *p_numOfPlayersInGame = p_param->p_numOfPlayersInGame, * p_numOfPlayersSyncing = p_param->p_numOfPlayersSyncing;
	char* p_username = NULL, * p_opponentUsername = NULL;
	Message* message = NULL;
	HANDLE h_sharedFile = NULL, lockEvent = NULL, syncEvent = NULL, failureEvent = NULL;

	if (NOT_SUCCESS == getEvents(&lockEvent, &syncEvent, &failureEvent)) 
		return NOT_SUCCESS;
	retVal = getUserNameAndApproveClient(socket, &p_username);
	if (retVal == NOT_SUCCESS) {
		freeServiceThreadResources(socket, lockEvent, syncEvent, failureEvent, NULL);
		return NOT_SUCCESS;
	}
	waitcode = WaitForSingleObject(lockEvent, LOCKEVENT_WAITTIME);
	if (WAIT_OBJECT_0 != waitcode) {
		freeServiceThreadResources(socket, lockEvent, syncEvent, failureEvent, p_username);
		return NOT_SUCCESS;
	}
	(*p_numOfPlayersInGame)++;
	if (!SetEvent(lockEvent)) {
		printf("Error in SetEvent(servicethread). Other threads will not be terminated\n");
		freeServiceThreadResources(socket, lockEvent, syncEvent, failureEvent, p_username);
		return NOT_SUCCESS;
	}
	// <-------- Entering game loop ------->
	while (1) {
		retVal = Main_menu(socket, lockEvent, syncEvent, p_numOfPlayersInGame, &playerOne, p_username, &p_opponentUsername);
		if (retVal != GAME_STILL_ON) {
			if (playerOne) {
				if (!DeleteFileA(sharedFile_name)) {
					printf("DeleteFileA failed. Error code: %d\n", GetLastError());
					retVal = NOT_SUCCESS;
					break;
				}
			}
			if (retVal == MAIN_MENU) continue;
			break;
		}
		h_sharedFile = CreateFileA(sharedFile_name, GENERIC_ALL, (FILE_SHARE_READ|FILE_SHARE_DELETE|FILE_SHARE_WRITE),
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == h_sharedFile) {
			printf("Can't open %s file Error: %d\n", sharedFile_name, GetLastError());
			if (playerOne) {
				if (!DeleteFileA(sharedFile_name)) {
					printf("DeleteFileA failed. Error code: %d\n", GetLastError());
					retVal = NOT_SUCCESS;
				}
			}
			break; //Leave game
		}
		//<---Both players are registered to the game, know each other names and start to play--->
		retVal = startGame(socket, h_sharedFile, lockEvent, syncEvent, playerOne, p_numOfPlayersInGame, p_username, p_opponentUsername, p_numOfPlayersSyncing);
		free(p_opponentUsername);
		CloseHandle(h_sharedFile);
		if (playerOne) {
			if (!DeleteFileA(sharedFile_name)) {
				printf("DeleteFileA failed. Error code: %d\n", GetLastError());
				retVal = NOT_SUCCESS;
			}
		}
		if (retVal == NOT_SUCCESS || retVal == DISCONNECTED) break;
	} // !while(1)
	waitcode = WaitForSingleObject(lockEvent, LOCKEVENT_WAITTIME);
	if (WAIT_OBJECT_0 != waitcode) {
		printf("WaitForSingleObject failed. Error code: %d\n", GetLastError());
		retVal = NOT_SUCCESS;
	}
	(*p_numOfPlayersInGame)--;
	if (!SetEvent(lockEvent)) {
		printf("SetEvent failed. Error code: %d\n", GetLastError());
		retVal = NOT_SUCCESS;
	}
	freeServiceThreadResources(INVALID_SOCKET, lockEvent, syncEvent, NULL, p_username);
	if (retVal == QUIT)
		confirmShutdown(socket);
	else {
		if (closesocket(socket))
			printf("closesocket failed. Error code: %d\n", GetLastError());
		if (retVal == NOT_SUCCESS) {
			if (!SetEvent(failureEvent)) {
				printf("Error in SetEvent(failureEvent). Other threads will not be terminated\n");
			}
		}
		CloseHandle(failureEvent);
	}
	return 0;
}

int getUserNameAndApproveClient(SOCKET socket, char** p_username) {
	int retVal;
	Message* message = NULL;
	
	retVal = getMessage(socket, &message, RESPONSE_WAITTIME);
	if (retVal != TRNS_SUCCEEDED) {
		printf("couldn't get username from client. Quitting\n");
		return NOT_SUCCESS;
	}
	if (strcmp((message->type), "CLIENT_REQUEST") != 0) {
		printf("message type is invalid, %s instead of CLIENT_REQUEST\n", message->type);
		free(message);
		return NOT_SUCCESS;
	}
	(*p_username) = message->username;
	free(message);
	char* p_rawMessage = "SERVER_APPROVED\n";
	retVal = SendString(p_rawMessage, socket);
	if (retVal != TRNS_SUCCEEDED) {
		printf("Transfer failed when sending %s\n", p_rawMessage);
		free(*p_username);
		return DISCONNECTED;
	}

	return SUCCESS;
}

int Main_menu(SOCKET socket, HANDLE lockEvent, HANDLE syncEvent, int* p_numOfPlayersInGame, int* p_playerOne, char* username, char** p_opponentUsername) {
	TransferResult_t transResult;
	int retVal, offset = 0;
	Message* message = NULL;
	HANDLE h_sharedFile = NULL;

	//<---Send SERVER_MAIN_MENU to client--->
	*p_playerOne = 0;
	char* p_rawMessage = "SERVER_MAIN_MENU\n";
	transResult = SendString(p_rawMessage, socket);
	if (transResult != TRNS_SUCCEEDED)
		return DISCONNECTED;
	//<---Get response from client--->
	retVal = getMessage(socket, &message, INFINITE);
	if (retVal != TRNS_SUCCEEDED) {
		if (retVal == TRNS_DISCONNECTED)
			return DISCONNECTED;
		else
			return NOT_SUCCESS;
	}
	//<---In case of client disconnect--->
	if (!strcmp(message->type, "CLIENT_DISCONNECT")) {
		free(message);
		return QUIT;
	}
	else if (strcmp(message->type, "CLIENT_VERSUS")) {//If an invalid response
		free(message);
		return NOT_SUCCESS;
	}
	//else- client chose CLIENT_VERSUS
	free(message);
	//----> Go to critical section
	retVal = ExchangeClientsNames(socket, lockEvent, syncEvent, p_numOfPlayersInGame, p_playerOne, username, p_opponentUsername);
	return retVal;
}

int ExchangeClientsNames(SOCKET socket, HANDLE lockEvent, HANDLE syncEvent, int* p_numOfPlayersInGame, int* p_playerOne, char* username, char** p_opponentUsername) {
	DWORD waitcode;
	TransferResult_t transResult;
	HANDLE h_sharedFile = NULL;
	int offset = 0;
	waitcode = WaitForSingleObject(lockEvent, LOCKEVENT_WAITTIME);
	if (waitcode != WAIT_OBJECT_0) {
		printf("Waitcode is %d\nError code %d while waiting for lockEvent\n", waitcode, GetLastError());
		return NOT_SUCCESS;
	}
	//<---In case there are less than 2 players--->
	if (*p_numOfPlayersInGame != 2) {
		if (*p_numOfPlayersInGame > 2) {
			printf("Error: incorrect number of players\n");
			if (!SetEvent(lockEvent))  //release lockEvent
				printf("SetEvent failed %d\n", GetLastError());
			return NOT_SUCCESS;
		}
		if (!SetEvent(lockEvent)) { //release lockEvent
			printf("SetEvent failed %d\n", GetLastError());
			return NOT_SUCCESS;
		}
		//<---Send client SERVER_NO_OPPONENTS--->
		char* p_rawMessage = "SERVER_NO_OPPONENTS\n";
		transResult = SendString(p_rawMessage, socket);
		if (transResult != TRNS_SUCCEEDED)
			return DISCONNECTED;
		return MAIN_MENU;
	}
	//<---In case there are 2 players--->
	h_sharedFile = openOrCreateFile(p_playerOne);
	if (h_sharedFile == INVALID_HANDLE_VALUE) {
		if (!SetEvent(lockEvent)) { //release lockEvent
			printf("SetEvent failed %d\n", GetLastError());
		}
		return NOT_SUCCESS;
	}
	if (!*p_playerOne) { //If this is player 2
		//get player 1's name
		if (NOT_SUCCESS == readFromFile(h_sharedFile, 0, p_opponentUsername, *p_playerOne, 1)) {
			CloseHandle(h_sharedFile);
			SetEvent(lockEvent);
			return NOT_SUCCESS;
		}
		offset = strlen(*p_opponentUsername) + 1;
	}
	//Write username to the file
	if (NOT_SUCCESS == writeToFile(h_sharedFile, offset, username, *p_playerOne, 1)) {
		CloseHandle(h_sharedFile);
		SetEvent(lockEvent);
		return NOT_SUCCESS;
	}
	//if Player 2: username to the file, release syncEvent for player 1 to read their username from the file.
	if (!*p_playerOne) {
		if (!SetEvent(syncEvent)) { //release syncEvent
			printf("SetEvent failed %d\n", GetLastError());
			CloseHandle(h_sharedFile);
			if (!SetEvent(lockEvent))
				printf("SetEvent failed %d\n", GetLastError());
			return NOT_SUCCESS;
		}
	}
	if (!SetEvent(lockEvent)) { //release lockEvent
		printf("SetEvent failed %d\n", GetLastError());
		CloseHandle(h_sharedFile);
		return NOT_SUCCESS;
	}
	//<-------Out of the critical section------>
	if (*p_playerOne) {
		waitcode = WaitForSingleObject(syncEvent, USER_WAITTIME);
		if (!ResetEvent(syncEvent)) { //lock syncEvent
			printf("ResetEvent failed %d\n", GetLastError());
			CloseHandle(h_sharedFile);
			return NOT_SUCCESS;
		}
		if (waitcode != WAIT_OBJECT_0) {
			CloseHandle(h_sharedFile);
			if (waitcode == WAIT_TIMEOUT) { //The other player did not respond within 10 minutes
				char* p_rawMessage = "SERVER_NO_OPPONENTS\n";
				transResult = SendString(p_rawMessage, socket);
				if (transResult != TRNS_SUCCEEDED)
					return DISCONNECTED;
				else
					return MAIN_MENU; //message was sent. go back to main menu
			}
			else {
				printf("There was a problem with waiting for syncEvent\n");
				return NOT_SUCCESS;
			}
		}
		offset = strlen(username) + 1;
		if (NOT_SUCCESS == readFromFile(h_sharedFile, offset, p_opponentUsername, *p_playerOne, 1)) {//get player 2's name
			CloseHandle(h_sharedFile);
			return NOT_SUCCESS;
		}
	}
	CloseHandle(h_sharedFile);
	return GAME_STILL_ON;
}

void freeServiceThreadResources(SOCKET socket, HANDLE lockEvent, HANDLE syncEvent, HANDLE failureEvent, char* username) {
	if (INVALID_SOCKET != socket) {
		if (closesocket(socket))
			printf("closesocket error (freeServiceThreadResources), error %ld.\n", WSAGetLastError());
	}
	if (lockEvent != NULL) 
		CloseHandle(lockEvent);
	if (syncEvent != NULL) 
		CloseHandle(syncEvent);
	if (failureEvent != NULL)
		CloseHandle(failureEvent);
	if (username != NULL)
		free(username);
}

int startGame(SOCKET socket, HANDLE h_sharedFile, HANDLE lockEvent, HANDLE syncEvent, int playerOne, int* p_numOfPlayersInGame, char* username, char* opponentName, int* p_numOfPlayersSyncing) {
	int status, results;
	char* p_serverMsg = NULL, * p_opponentGuess = NULL, * p_userNum = NULL, * p_opponentNum = NULL, * p_userGuess = NULL;
	Message* p_clientMsg = NULL;
	status = secretNumInit(socket, h_sharedFile, lockEvent, syncEvent, playerOne, p_numOfPlayersInGame,
		opponentName, p_numOfPlayersSyncing, &p_userNum, &p_opponentNum);
	if (status != SUCCESS) return status;
	// while (1)
	while (1) {
		status = opponentLeftGame(socket, p_numOfPlayersInGame, lockEvent);
		if (GAME_STILL_ON != status) {
			freeSingleGameMemory(p_userNum, p_opponentNum, p_userGuess, p_opponentGuess);
			return status;
		} 
		//<---send SERVER_PLAYER_MOVE_REQUEST--->
		p_serverMsg = prepareMsg("SERVER_PLAYER_MOVE_REQUEST", NULL);
		if (NULL == p_serverMsg) {
			freeSingleGameMemory(p_userNum, p_opponentNum, p_userGuess, p_opponentGuess);
			return NOT_SUCCESS;
		}
		status = SendString(p_serverMsg, socket);
		free(p_serverMsg);
		if (TRNS_FAILED == status) {
			freeSingleGameMemory(p_userNum, p_opponentNum, NULL, NULL);
			return DISCONNECTED;
		}
		//<---recive message from client (CLIENT_PLAYER_MOVE)--->
		status = getMessage(socket, &p_clientMsg, USER_WAITTIME);
		if (TRNS_SUCCEEDED != status) {
			freeSingleGameMemory(p_userNum, p_opponentNum, NULL, NULL);
			if (TRNS_DISCONNECTED == status || TRNS_TIMEOUT == status) return DISCONNECTED;
			else if (TRNS_FAILED == status) return NOT_SUCCESS;
		}
		if (strcmp(p_clientMsg->type, "CLIENT_PLAYER_MOVE")) {
			freeSingleGameMemory(p_userNum, p_opponentNum, NULL, NULL);
			free(p_clientMsg);
			return NOT_SUCCESS;
		}
		if (NULL != p_clientMsg)
			p_userGuess = p_clientMsg->guess;
		free(p_clientMsg);
		//<---write user guess to shared file--->
		status = writeToFile(h_sharedFile, GUESS_OFFSET, p_userGuess, playerOne, 0);
		if (NOT_SUCCESS == status) {
			freeSingleGameMemory(p_userNum, p_opponentNum, p_userGuess, NULL);
			return NOT_SUCCESS;
		}
		//<---wait that opponent thread write his guess to shared file--->
		status = SyncTwoThreads(socket, p_numOfPlayersSyncing, p_numOfPlayersInGame, lockEvent, syncEvent);
		if (GAME_STILL_ON != status) {
			freeSingleGameMemory(p_userNum, p_opponentNum, p_userGuess, NULL);
			return status;
		}
		//<---read opponent guess from shared file--->
		if (NOT_SUCCESS == readFromFile(h_sharedFile, GUESS_OFFSET, &p_opponentGuess, playerOne, 0)) {
			freeSingleGameMemory(p_userNum, p_opponentNum, p_userGuess, p_opponentGuess);
			return NOT_SUCCESS;
		}
		//<---calculate game results--->
		results = getResults(&p_serverMsg, username, opponentName, p_userNum, p_opponentNum, p_userGuess, p_opponentGuess);
		if (NOT_SUCCESS == results) {
			freeSingleGameMemory(p_userNum, p_opponentNum, p_userGuess, p_opponentGuess);
			return NOT_SUCCESS;
		}
		//<------ Send the results to the client------>
		status = SendString(p_serverMsg, socket);
		free(p_serverMsg);
		if (TRNS_FAILED == status) {
			freeSingleGameMemory(p_userNum, p_opponentNum, p_userGuess, p_opponentGuess);
			return DISCONNECTED;
		}
		//<---Continue according to the game results--->
		if (GAME_STILL_ON == results) { //If no one wins
			freeSingleGameMemory(NULL, NULL, p_userGuess, p_opponentGuess);
			continue;
		}
		if (MAIN_MENU == results) {//If one or both players win
			freeSingleGameMemory(p_userNum, p_opponentNum, p_userGuess, p_opponentGuess);
			return MAIN_MENU;
		}
	}
}

int secretNumInit(SOCKET socket, HANDLE h_sharedFile, HANDLE lockEvent, HANDLE syncEvent, int playerOne,
	int* p_numOfPlayersInGame, char* opponentName, int* p_numOfPlayersSyncing, char** p_userNum, char** p_opponentNum) {
	int status;
	char* p_serverMsg = NULL, *p_opponentNumTmp = NULL;
	Message* p_clientMsg = NULL;
	status = opponentLeftGame(socket, p_numOfPlayersInGame, lockEvent);
	if (GAME_STILL_ON != status) return status;
	//<---send SERVER_INVITE--->
	p_serverMsg = prepareMsg("SERVER_INVITE:", opponentName);
	if (NULL == p_serverMsg) return NOT_SUCCESS;
	status = SendString(p_serverMsg, socket);
	free(p_serverMsg);
	if (TRNS_FAILED == status) return DISCONNECTED;
	//<---send SERVER_SETUP_REQUSET--->
	p_serverMsg = prepareMsg("SERVER_SETUP_REQUSET", NULL);
	if (NULL == p_serverMsg) return NOT_SUCCESS;
	status = SendString(p_serverMsg, socket);
	free(p_serverMsg);
	if (TRNS_FAILED == status) return DISCONNECTED;
	//<---recive message from client--->
	status = getMessage(socket, &p_clientMsg, USER_WAITTIME);
	if (TRNS_DISCONNECTED == status || TRNS_TIMEOUT == status) return DISCONNECTED;
	else if (TRNS_FAILED == status) return NOT_SUCCESS;
	if (strcmp(p_clientMsg->type, "CLIENT_SETUP")) {
		free(p_clientMsg);
		return NOT_SUCCESS;
	}
	*p_userNum = p_clientMsg->guess;
	free(p_clientMsg);
	//<---write to shared file the user secret number--->
	status = writeToFile(h_sharedFile, SECRETNUM_OFFSET, *p_userNum, playerOne, 0);
	if (NOT_SUCCESS == status) {
		free(*p_userNum);
		return NOT_SUCCESS;
	}
	status = opponentLeftGame(socket, p_numOfPlayersInGame, lockEvent);
	if (GAME_STILL_ON != status) {
		free(*p_userNum);
		return status;
	}
	//<---wait that opponent thread write his secret number to shared file--->
	status = SyncTwoThreads(socket, p_numOfPlayersSyncing, p_numOfPlayersInGame, lockEvent, syncEvent);
	if (GAME_STILL_ON != status) {
		free(*p_userNum);
		return status;
	}
	//<---read opponent secret number from shared file--->
	if (NOT_SUCCESS == readFromFile(h_sharedFile, SECRETNUM_OFFSET, &p_opponentNumTmp, playerOne, 0)) {
		free(*p_userNum);
		return NOT_SUCCESS;
	}
	*p_opponentNum = p_opponentNumTmp;
	return SUCCESS;
}

int getResults(char** resultMsg, char* username, char* opponentName, char* userNum, char* opponentNum, char* userGuess, char* opponentGuess) {
	char* p_resultMsg = NULL, c_bulls, c_cows;
	int messageLen = 0, bulls = 0, cows = 0, i = 0, indexDiff, userStatus = -1, opponentStatus = -1;

	//call to function which will check if user won - need to read it from file
	//<---check if user have 4 bulls--->
	if (!strcmp(opponentNum, userGuess))
		userStatus = WIN;
	//<---check if opponent have 4 bulls--->
	if (!strcmp(userNum, opponentGuess))
		opponentStatus = WIN;
	//<---check if TIE--->
	if (WIN == userStatus && WIN == opponentStatus) {
		p_resultMsg = prepareMsg("SERVER_DRAW", NULL);
		if (NULL == p_resultMsg) return NOT_SUCCESS;
		*resultMsg = p_resultMsg;
		return MAIN_MENU;
	}
	//<---in case of user WIN--->
	else if (WIN == userStatus && WIN != opponentStatus) {
		p_resultMsg = winMsg(opponentNum, username);
		if (NULL == p_resultMsg) return NOT_SUCCESS;
		*resultMsg = p_resultMsg;
		return MAIN_MENU;
	}
	//<---in case of opponent WIN--->
	else if (WIN != userStatus && WIN == opponentStatus) {
		p_resultMsg = winMsg(opponentNum, opponentName);
		if (NULL == p_resultMsg) return NOT_SUCCESS;
		*resultMsg = p_resultMsg;
		return MAIN_MENU;
	}
	else //In case no winner yet
	{
		for (i = 0; i < 4; i++) { //calculate number of bulls and cows
			char* cur = strchr(opponentNum, userGuess[i]);
			if (NULL == cur) continue;
			indexDiff = (int)(cur - opponentNum);
			if (i == indexDiff) {
				bulls += 1;
				continue;
			}
			else
				cows += 1;
		}
		c_bulls = bulls + '0';
		c_cows = cows + '0';
		messageLen = strlen("SERVER_GAME_RESULTS:") + strlen(opponentName) + strlen(opponentGuess) + 7; /*7 for bulls,cows,3*;,\n,\0*/
		if (NULL == (p_resultMsg = malloc(messageLen))) {
			printf("Fatal error: memory allocation failed (getResults).\n");
			return NOT_SUCCESS;
		}
		strcpy_s(p_resultMsg, messageLen, "SERVER_GAME_RESULTS:");
		strncat_s(p_resultMsg, messageLen, &c_bulls, 1);
		strcat_s(p_resultMsg, messageLen, ";");
		strncat_s(p_resultMsg, messageLen, &c_cows, 1);
		strcat_s(p_resultMsg, messageLen, ";");
		strcat_s(p_resultMsg, messageLen, opponentName);
		strcat_s(p_resultMsg, messageLen, ";");
		strcat_s(p_resultMsg, messageLen, opponentGuess);
		strcat_s(p_resultMsg, messageLen, "\n");
		p_resultMsg[messageLen - 1] = '\0';
		*resultMsg = p_resultMsg;
		return GAME_STILL_ON;
	}
}

void freeSingleGameMemory(char* userNum, char* opponentNum, char* userGuess, char* opponentGuess) {

	if (NULL != userNum) {
		free(userNum);
		userNum = NULL;
	}
	if (NULL != userGuess) {
		free(userGuess);
		userGuess = NULL;
	}
	if (NULL != opponentGuess) {
		free(opponentGuess);
		opponentGuess = NULL;
	}
	if (NULL != opponentNum) {
		free(opponentNum);
		opponentNum = NULL;
	}
}

char* winMsg(char* opponentNum, char* winnerName) {
	char* p_resultMsg = NULL;
	int messageLen;

	messageLen = strlen("SERVER_WIN:") + strlen(winnerName) + strlen(opponentNum) + 3; //+3 for ;,\n,\0
	if (NULL == (p_resultMsg = malloc(messageLen))) {
		printf("Fatal error: memory allocation failed (winnerCheck).\n");
		return NULL;
	}
	strcpy_s(p_resultMsg, messageLen, "SERVER_WIN:");
	strcat_s(p_resultMsg, messageLen, winnerName);
	strcat_s(p_resultMsg, messageLen, ";");
	strcat_s(p_resultMsg, messageLen, opponentNum);
	strcat_s(p_resultMsg, messageLen, "\n");
	return p_resultMsg;

}
