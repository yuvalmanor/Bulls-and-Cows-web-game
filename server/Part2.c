#include "Part2.h"

int startGame(SOCKET socket, HANDLE h_sharedFile, int playerOne) {
	int status;
	char* p_serverMsg = NULL, * p_opponentGuess = NULL, * p_userNum = NULL;
	Message* p_clientMsg = NULL;

	/*if (otherPlayerLeftGame()) {
		send SERVER_OPPONENT_QUIT	<--------Not sure if this "if" is necessary because thread isnt listening now. 
			go back to main menu
	}*/
	p_serverMsg = prepareMsg("SERVER_SETUP_REQUSET", NULL);
	if (NULL == p_serverMsg) return NOT_SUCCESS;
	status = SendString(p_serverMsg, socket);
	free(p_serverMsg);
	if (TRNS_DISCONNECTED == status || TRNS_TIMEOUT == status) return DISCONNECTED; //what happan if send/recv disconnect\timeout?
	else if (TRNS_FAILED == status) return NOT_SUCCESS;
	status = getMessage(socket, &p_clientMsg, 30000); //need to check if waitTime is correct
	if (TRNS_DISCONNECTED == status || TRNS_TIMEOUT == status) return DISCONNECTED;
	else if (TRNS_FAILED == status) return NOT_SUCCESS;
	if (strcmp(p_clientMsg->type, "CLIENT_SETUP")) {
		free(p_clientMsg);
		return NOT_SUCCESS;
	}
	p_userNum = p_clientMsg->guess;
	status = writeToFile(h_sharedFile, SECRETNUM_OFFSET, p_userNum, playerOne, 0);
	if (NOT_SUCCESS == status) return NOT_SUCCESS;

}


char* getResults(char* username, char* opponentName, char* userNum, char* opponentGuess, char* opponentNum) {
	char* p_resultMsg = NULL, c_bulls, c_cows;
	int messageLen = 0, bulls = 0, cows = 0, i = 0, indexDiff;

	//call to function which will check if user won - need to read it from file

	//<---In case of opponent WIN--->
	if (!strcmp(userNum, opponentGuess)) {
		messageLen = strlen("SERVER_WIN:") + strlen(opponentName) + strlen(opponentNum) + 3; //+3 for ;,\n,\0
		if (NULL == (p_resultMsg = malloc(messageLen))) {
			printf("Fatal error: memory allocation failed (getResults).\n");
			return NULL;
		}
		strcpy_s(p_resultMsg, messageLen, "SERVER_WIN:");
		strcat_s(p_resultMsg, messageLen, opponentName);
		strcat_s(p_resultMsg, messageLen, ";");
		strcat_s(p_resultMsg, messageLen, opponentNum);
		strcat_s(p_resultMsg, messageLen, "\n");
		return p_resultMsg;
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
			return NULL;
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
		return p_resultMsg;
	}
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
if (otherPlayerLeftGame()) {
	send SERVER_OPPONENT_QUIT
		go back to main menu
}
WAIT FOR BOTH OPPONENTS TO WRITE NUM AND GET THE OTHER SECRETNUM ? ?

game on!
At this point, both players have secretNumand otherSecretNum
while (1) {
	if (otherPlayerLeftGame()) {
		send SERVER_OPPONENT_QUIT
			gameStatus = MAIN_MENU
			break;
	}
	send SERVER_PLAYER_MOVE_REQUEST
		validate message sent -> if not, gameStatus = FAILED, break;
	getMessage()
		validate message received -> if not, gameStatus = FAILED, break;
	if message->type != CLIENT_PLAYER_MOVE->gameStatus = FAILED, break;
	guess = message->guess

		IWin = getResult(guess, otherSecretNum, &result)
		retVal = writeGuessAndResultToFile(h_file, playerOne, guess, result)
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
int opponentLeftGame(SOCKET socket, int* p_players, HANDLE lockEvent) {
	DWORD waitCode;
	char* p_serverMsg = NULL;
	int status;

	waitCode = waitForSingleObject(lockEvent, LOCKEVENT_WAITTIME);
	if (WAIT_OBJECT_0 != waitCode) {
		int x = 5;
	}
	if (*p_players != 2) {
		p_serverMsg = prepareMsg("SERVER_NO_OPPONENTS", NULL);
		if (NULL == p_serverMsg) return NOT_SUCCESS;
		status = SendString(p_serverMsg, socket);
		free(p_serverMsg);
		if (TRNS_DISCONNECTED == status || TRNS_TIMEOUT == status) return DISCONNECTED; //what happan if send/recv disconnect\timeout?
		else if (TRNS_FAILED == status) return NOT_SUCCESS;
		SetEvent(lockEvent);

	}
}
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