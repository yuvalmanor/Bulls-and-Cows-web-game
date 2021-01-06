#include "Part2.h"

int startGame(SOCKET socket, HANDLE h_sharedFile, HANDLE lockEvent, HANDLE syncEvent, int playerOne, int* p_players, char* username, char* opponentName) {
	int status;
	char* p_serverMsg = NULL, * p_opponentGuess = NULL, * p_userNum = NULL, *p_opponentNum=NULL, *p_userGuess=NULL;
	Message* p_clientMsg = NULL;

	status = opponentLeftGame(socket, p_players, lockEvent); 
	if (GAME_STILL_ON != status) return status;
	//<---send SERVER_SETUP_REQUSET--->
	p_serverMsg = prepareMsg("SERVER_SETUP_REQUSET", NULL);
	if (NULL == p_serverMsg) return NOT_SUCCESS;
	status = SendString(p_serverMsg, socket);
	free(p_serverMsg);
	if (TRNS_DISCONNECTED == status || TRNS_TIMEOUT == status) return DISCONNECTED; //what happan if send/recv disconnect\timeout?
	else if (TRNS_FAILED == status) return NOT_SUCCESS;
	//<---recive message from client--->
	status = getMessage(socket, &p_clientMsg, 30000); //need to check if waitTime is correct, shouldnt be 10MIN?
	if (TRNS_DISCONNECTED == status || TRNS_TIMEOUT == status) return DISCONNECTED; 
	else if (TRNS_FAILED == status) return NOT_SUCCESS;
	if (strcmp(p_clientMsg->type, "CLIENT_SETUP")) { 
		free(p_clientMsg);
		return NOT_SUCCESS;
	}
	p_userNum = p_clientMsg->guess;
	free(p_clientMsg);
	//<---write to shared file the user secret number--->
	status = writeToFile(h_sharedFile, SECRETNUM_OFFSET, p_userNum, playerOne, 0);
	if (NOT_SUCCESS == status) {
		free(p_userNum);
		return NOT_SUCCESS;
	}
	status = opponentLeftGame(socket, p_players, lockEvent);
	if (GAME_STILL_ON != status) {
		free(p_userNum);
		return status;
	}
	//<---wait that opponent thread write his secret number to shared file--->
	status = SyncTwoThreads(p_players,lockEvent,syncEvent,USER_WAITTIME);
	if (GAME_STILL_ON != status) {
		free(p_userNum);
		return status;
	}
	//<---read opponent secret number from shared file--->
	if (NOT_SUCCESS == readFromFile(h_sharedFile, SECRETNUM_OFFSET, &p_opponentNum, playerOne, 0)) {
		free(p_userNum);
		return NOT_SUCCESS;
	}
	while (1) {
		status = opponentLeftGame(socket, p_players, lockEvent);
		if (GAME_STILL_ON != status) {
			freeMemory(p_userNum, p_opponentNum, p_userGuess, p_opponentGuess);
			return status;
		}
		//<---send SERVER_PLAYER_MOVE_REQUEST--->
		p_serverMsg = prepareMsg("SERVER_PLAYER_MOVE_REQUEST", NULL);
		if (NULL == p_serverMsg) {
			freeMemory(p_userNum, p_opponentNum, p_userGuess, p_opponentGuess);
			return NOT_SUCCESS;
		}
		status = SendString(p_serverMsg, socket);
		free(p_serverMsg);
		if (TRNS_SUCCEEDED != status) {
			freeMemory(p_userNum, p_opponentNum, NULL, NULL);
			if (TRNS_DISCONNECTED == status || TRNS_TIMEOUT == status) return DISCONNECTED;
			else if (TRNS_FAILED == status) return NOT_SUCCESS;
		}
		//<---recive message from client (CLIENT_PLAYER_MOVE)--->
		status = getMessage(socket, &p_clientMsg, USER_WAITTIME); //need to check if waitTime is correct, shouldnt be 10MIN?
		if (TRNS_SUCCEEDED != status) {
			freeMemory(p_userNum, p_opponentNum, NULL, NULL);
			if (TRNS_DISCONNECTED == status || TRNS_TIMEOUT == status) return DISCONNECTED;
			else if (TRNS_FAILED == status) return NOT_SUCCESS;
		}
		if (strcmp(p_clientMsg->type, "CLIENT_PLAYER_MOVE")) {
			free(p_clientMsg->guess);
			free(p_clientMsg);
			return NOT_SUCCESS;
		}
		if(NULL!=p_userGuess)
			p_userGuess = p_clientMsg->guess;
		free(p_clientMsg);
		//<---write user guess to shared file--->
		status = writeToFile(h_sharedFile, GUESS_OFFSET, p_userGuess, playerOne, 0);
		if (NOT_SUCCESS == status) {
			freeMemory(p_userNum, p_opponentNum, p_userGuess, p_opponentGuess);
			return NOT_SUCCESS;
		}
		//<---wait that opponent thread write his guess to shared file--->
		status = SyncTwoThreads(p_players, lockEvent, syncEvent, USER_WAITTIME);
		if (GAME_STILL_ON != status) {
			freeMemory(p_userNum, p_opponentNum, p_userGuess, p_opponentGuess);
			return status;
		}
		//<---read opponent guess from shared file--->
		if (NOT_SUCCESS == readFromFile(h_sharedFile, GUESS_OFFSET, &p_opponentGuess, playerOne, 0)) {
			freeMemory(p_userNum, p_opponentNum, p_userGuess, p_opponentGuess);
			return NOT_SUCCESS;
		}
		//<---calculate game results--->
		status = getResults(&p_serverMsg, username, opponentName, p_userNum, p_opponentNum, p_userGuess, p_opponentGuess);
		if (NOT_SUCCESS == status) {
			freeMemory(p_userNum, p_opponentNum, p_userGuess, p_opponentGuess);
			return NOT_SUCCESS;
		}
		//<---if there is no winner or tie--->
		if (GAME_STILL_ON == status)
			continue;
		//<---if someone won or its a tie--->
		if (MAIN_MENU == status) {
			freeMemory(p_userNum, p_opponentNum, p_userGuess, p_opponentGuess);
			return MAIN_MENU;
		}
	}
}


int getResults(char** resultMsg, char* username, char* opponentName, char* userNum, char* opponentNum,char* userGuess, char* opponentGuess) {
	char* p_resultMsg = NULL, c_bulls, c_cows;
	int messageLen = 0, bulls = 0, cows = 0, i = 0, indexDiff, userStatus=-1, opponentStatus=-1;

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
		p_resultMsg = winMsg(userNum, opponentName);
		if (NULL == p_resultMsg) return NOT_SUCCESS;
		*resultMsg = p_resultMsg;
		return MAIN_MENU;
	}
	else //In case no winner yet
	{
		for (i = 0; i < 4; i++) { //calculate number of bulls and cows
			char* cur = strchr(userNum, opponentGuess[i]);
			if (NULL == cur) continue;
			indexDiff = (int)(cur - userNum);
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
		strncat_s(p_resultMsg, messageLen, &c_bulls, 1);
		strcat_s(p_resultMsg, messageLen, ";");
		strcat_s(p_resultMsg, messageLen, opponentName);
		strcat_s(p_resultMsg, messageLen, ";");
		strcat_s(p_resultMsg, messageLen, opponentGuess);
		strcat_s(p_resultMsg, messageLen, "\n");
		*resultMsg = p_resultMsg;
		return GAME_STILL_ON;
	}
}
//need to handle TIMEOUT situation inside opponentLeftGame.
int opponentLeftGame(SOCKET socket, int* p_players, HANDLE lockEvent) {
	DWORD waitCode;
	char* p_serverMsg = NULL;
	int status;

	waitCode = WaitForSingleObject(lockEvent, LOCKEVENT_WAITTIME);
	if (WAIT_OBJECT_0 != waitCode) {
		if (WAIT_FAILED == waitCode) {
			printf("waitForSingleObject error(opponentLeftGame). Error code: %d.\n", GetLastError());
			return NOT_SUCCESS;
		}
		//if(WAIT_TIMEOUT==waitCode)?
	}
	if (*p_players != 2) {
		p_serverMsg = prepareMsg("SERVER_NO_OPPONENTS", NULL);
		if (NULL == p_serverMsg) return NOT_SUCCESS;
		status = SendString(p_serverMsg, socket);
		free(p_serverMsg);
		if (TRNS_DISCONNECTED == status || TRNS_TIMEOUT == status) return DISCONNECTED; //what happan if send/recv disconnect\timeout?
		else if (TRNS_FAILED == status) return NOT_SUCCESS;
		if (0 == SetEvent(lockEvent)) {
			printf("Error when setEvent (opponentLeftGame). Error code: %d.\n", GetLastError());
			return NOT_SUCCESS;
		}
		return MAIN_MENU;
	}
	if (0 == SetEvent(lockEvent)) {
		printf("Error when setEvent (opponentLeftGame). Error code: %d.\n", GetLastError());
		return NOT_SUCCESS;
	}
	return GAME_STILL_ON;
}
void freeMemory(char* userNum, char* opponentNum, char* userGuess, char* opponentGuess) {
	
	if (NULL != userNum)
		free(userNum);
	if (NULL != userGuess)
		free(userGuess);
	if (NULL != opponentGuess)
		free(opponentGuess);
	if (NULL != opponentNum)
		free(opponentNum);
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
/*< -------------- - PART II--------------->
#	There are two players and they know each other names
	if (otherPlayerLeftGame()) {
		send SERVER_OPPONENT_QUIT
			go back to main menu
	}
#send SERVER_SETUP_REQUSET
#validate message sent -> if not, leaveGame() break;
#getMessage()
#validate message received -> if not, leaveGame() break;
#if message->type != CLIENT_SETUP->leaveGame() break;
#secretNum = message->guess
#retVal = writeSecretNumToFile(h_file, playerOne, secretNum);--->Why we need function for every writing?
#check if retVal suceeded										why not one function for all writes?
#if (otherPlayerLeftGame()) {
	send SERVER_OPPONENT_QUIT
		go back to main menu
}
#WAIT FOR BOTH OPPONENTS TO WRITE NUM AND GET THE OTHER SECRETNUM ? ?*/
/*
game on!
At this point, both players have secretNumand otherSecretNum
while (1) {
	#if (otherPlayerLeftGame()) {
		send SERVER_OPPONENT_QUIT
			gameStatus = MAIN_MENU
			break;
	}
#	send SERVER_PLAYER_MOVE_REQUEST
#		validate message sent -> if not, gameStatus = FAILED, break;
	getMessage()
#		validate message received -> if not, gameStatus = FAILED, break;
#	if message->type != CLIENT_PLAYER_MOVE->gameStatus = FAILED, break;
#	guess = message->guess
	
		#IWin = getResult(guess, otherSecretNum, &result)
		retVal = writeGuessAndResultToFile(h_file, playerOne, guess, result)<---MAYBE DO IT INSIDE getResults().
		validate retVal is good -> if not, gameStatus = FAILED, break;
	if (IWin) {
		waitcode = waitForSingleObject(lockEvent);
		check waitcode if there was a problem->release(lockEvent), gameStatus = FAILED, break;
		*(p_playerWins)++;
		release(lockEvent);

		if (otherPlayerLeftGame()) {
			send SERVER_OPPONENT_QUIT
				gameStatus = MAIN_MENU
				break;
		}
		WAIT FOR BOTH PLAYERS TO WRITE RESULT AND GUESS TO FILE < -- -

			getOtherPlayerGuessAndResult(h_file, &otherGuess, &otherResult)
			gameStatus = GameStatus(socket, username, othersernaUme, guess, otherGuess, otherResult, IWin)
			if (gameStatus != GAME_STILL_ON) { //No need for more guesses (Someone won or there was an error)
				break;
			}
	}// end of the guessing while loop
	if (gameStatus == FAILED) {
		threadRetVal = -1;
		break
	}
}
//If you are here, you're leaving the game
*(p_players)--;
closesocket(socket);
return threadRetVal;
*/
/*
		int otherPlayerLeftGame(SOCKET socket, int* p_players, Message* message){
			waitcode = waitForSingleObject(lockEvent, waittime):
				checkwaitcode() -> avoid deadlocks. if waitcode is not 0, game is off
			if (*(p_players) != 2 || message->type == "SERVER_OPPONENT_QUIT"){
				
				return 0;
			}
			release(lockEvent);
			return 1;
		}


		GameStatus(socket, username, otherUsername, guess, otherGuess, otherResult, IWin){
		 int theyWon = 0;
			if (otherResult[0] == 4){ //  other player has 4 bulls
				theyWon = 1;
			}
			if (IWin || theyWin){
				if (IWin && theyWin){
					send SERVER_DRAW
					validate message sent -> if not return FAILED
				}
				if (IWin){
					prepareMessage( SERVER_WIN, username, otherGuess)
					send SERVER_WIN with username and other user's guess
					validate message sent -> if not return FAILED
					}
				else{ //theyWin
					prepareMessage( SERVER_WIN, otherUsername, guess)
					send SERVER_WIN with username and other user's guess
					validate message sent -> if not return FAILED
				}
				return MAIN_MENU;
			}
			//If no one wins
			prepareMessage(SERVER_GAME_RESULTS, result[0], result[1], otherUsername, otherGuess)
			send the message
			validate message sent -> if not return FAILED
			return GAME_STILL_ON;
		}
		*/