#include "Part2.h"

int startGame(SOCKET socket, HANDLE h_sharedFile) {
	int status;
	char* p_serverMsg = NULL;
	Message* p_clientMsg = NULL;

	/*if (otherPlayerLeftGame()) {
		send SERVER_OPPONENT_QUIT	<--------Not sure if this "if" is necessary because thread isnt listening now. 
			go back to main menu
	}*/
	p_serverMsg = prepareMsg("SERVER_SETUP_REQUSET", NULL);
	if (NULL == p_serverMsg) return NOT_SUCCESS;
	status = SendString(p_serverMsg, socket);
	free(p_serverMsg);
	if (TRNS_DISCONNECTED == status || TRNS_TIMEOUT == status) return START_AGAIN; //what happan if send/recv disconnect\timeout?
	else if (TRNS_FAILED == status) return NOT_SUCCESS;
	
}

/*< -------------- - PART II--------------->
#	There are two players and they know each other names
	if (otherPlayerLeftGame()) {
		send SERVER_OPPONENT_QUIT
			go back to main menu
	}
send SERVER_SETUP_REQUSET
validate message sent -> if not, leaveGame() break;
getMessage()
validate message received -> if not, leaveGame() break;
if message->type != CLIENT_SETUP->leaveGame() break;
secretNum = message->guess
retVal = writeSecretNumToFile(h_file, playerOne, secretNum);
check if retVal suceeded
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
/*
		int otherPlayerLeftGame(SOCKET socket, int* p_players, Message* message){
			waitcode = waitForSingleObject(lockEvent, waittime):
				checkwaitcode() -> avoid deadlocks. if waitcode is not 0, game is off
			if (*(p_players) != 2 || message->type == "SERVER_OPPONENT_QUIT"){
				release(lockEvent);
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