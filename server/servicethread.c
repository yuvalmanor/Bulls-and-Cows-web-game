#include "servicethread.h"

HANDLE openOrCreateFile(int *playerOne) {
	DWORD dwDesiredAccess = 0, dwShareMode = 0, dwCreationDisposition = 0;
	HANDLE hFile;
	dwDesiredAccess = (GENERIC_READ | GENERIC_WRITE);
	dwShareMode = FILE_SHARE_DELETE;
	//Try to open existing file:
	dwCreationDisposition = OPEN_EXISTING;
	hFile = CreateFileA(GAMESESSION_FILENAME,
		dwDesiredAccess,
		dwShareMode,
		NULL,
		dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	//if it doesn't exist, create a new file and set playerOne to 1
	if (GetLastError() == ERROR_FILE_NOT_FOUND) {
		dwCreationDisposition = CREATE_NEW;
		hFile = CreateFileA(GAMESESSION_FILENAME,
			dwDesiredAccess,
			dwShareMode,
			NULL,
			dwCreationDisposition,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		(*playerOne) = 1;
		printf("A new file was created\nPlayerOne: 1\n ");

	}
	else if (INVALID_HANDLE_VALUE == hFile) {
		printf("Can't open %s file\n", GAMESESSION_FILENAME);
		return INVALID_HANDLE_VALUE;
	}
	else {
		printf("An existing file was opened\nPlayerOne: 0\n ");
		(*playerOne) = 0;
	
	}
	return hFile;
}

int writeUserNameToFile(HANDLE h_file, char* username, int playerOne, char** otherUsername) {
		DWORD dwBytesWritten, filePointer, dwBytesRead;
		int numOfBytesToWrite = (int)strlen(username);
		char* buffer = NULL;

		filePointer = SetFilePointer(h_file, 36, NULL, FILE_BEGIN); //Change 36 to a DEFINED
		if (INVALID_SET_FILE_POINTER == filePointer) {
			printf("File pointer failed to move\nError code:%d\n", GetLastError());
			return 0;
		}
		if (!playerOne) { //this is the second client. Read the first client's username and add yours afterwards
			if (NULL == (buffer = (char*)malloc(MAX_USERNAME_LEN))) {
				printf("Fatal error: memory allocation failed (writeUserNameToFile).\n");
				return 0;
			}
			printf("second client\n");
			if (FALSE == ReadFile(h_file, buffer, MAX_USERNAME_LEN, &dwBytesRead, NULL)) {//Does it read until the \n?
				printf("File read failed.\nError code:%d\n", GetLastError());
				free(buffer);
				return 0;
			}
			//Place \0 at the right place
			for (int i = 0; i < MAX_USERNAME_LEN; i++) {
				if (buffer[i] == '\r') {
					buffer[i] = '\0';
					break;
				}
			}
			*otherUsername = buffer;
		} 
		if (FALSE == WriteFile(h_file, username, numOfBytesToWrite, &dwBytesWritten, NULL)) {
			printf("File write failed.\nError code:%d\n", GetLastError());
			return 0;
		}
		if (FALSE == WriteFile(h_file, "\r\n", 1, &dwBytesWritten, NULL)) {
			printf("File write failed.\nError code:%d\n", GetLastError());
			return 0;
		}
		return 1;
}

int getPlayer2Name(HANDLE h_sharedFile, char* username, char** otherUsername) {
	DWORD filePointer, dwBytesRead;
	int usernameLen = (int)strlen(username); char* buffer;

	filePointer = SetFilePointer(h_sharedFile, 36+usernameLen+2, NULL, FILE_BEGIN); //Change 36 to a DEFINED
	if (INVALID_SET_FILE_POINTER == filePointer) {
		printf("File pointer failed to move\nError code:%d\n", GetLastError());
		return NOT_SUCCESS;
	}
	if (NULL == (buffer = (char*)malloc(MAX_USERNAME_LEN))) {
		printf("Fatal error: memory allocation failed (writeUserNameToFile).\n");
		return NOT_SUCCESS;
	}
	if (FALSE == ReadFile(h_sharedFile, buffer, MAX_USERNAME_LEN, &dwBytesRead, NULL)) {//Does it read until the \n?
		printf("File read failed.\nError code:%d\n", GetLastError());
		free(buffer);
		return NOT_SUCCESS;
	}
	//Place \0 at the right place
	for (int i = 0; i < MAX_USERNAME_LEN; i++) {
		if (buffer[i] == '\r') {
			buffer[i] = '\0';
			break;
		}
	}
	*otherUsername = buffer;
	return SUCCESS;
}

void leaveGame(SOCKET socket, int* p_players, HANDLE h_sharedFile, HANDLE fileEven) {

}

DWORD ServiceThread(void* lpParam) {
	//get thread parameters
	ThreadParam* p_param;
	LPCWSTR sharedFileName = GAMESESSION_FILENAME;
	if (NULL == lpParam) {
		printf("Service thread can't work with NULL as parameters\n");
		return -1;
	}
	p_param = (ThreadParam*)lpParam;
	SOCKET socket = p_param->socket;
	int playerOne, transferred, retVal;
	int *p_players = p_param->p_players;
	char* username = NULL, * otherUsername = NULL, *secretNum=NULL, *otherSecretNum=NULL;
	char* p_msg = NULL;
	Message* message = NULL;
	HANDLE h_sharedFile = NULL;
	TransferResult_t transResult;

	printf("Waiting for username from client\n");
		//<------Get username from client----->
	transferred = getMessage(socket, &message, 15000); //Change waitTime to a DEFINED number 
	if (transferred != 1) {
		printf("couldn't get username from client. Quitting\n");
		return -1;
	}
	if (strcmp((message->type), "CLIENT_REQUEST") != 0)  {
		printf("message type is invalid, %s instead of CLIENT_REQUEST\n", message->type);
		free(message);
		return -1;
	}
	username = message->username;
	free(message); message = NULL;
	printf("Username is %s\n", username);
	printf("sending SERVER_APPROVED\n");
	char* p_rawMessage = "SERVER_APPROVED\n";
	transResult = SendString(p_rawMessage, socket);
	if (transResult != TRNS_SUCCEEDED) {
		return 1;
	}
	if (transResult == TRNS_FAILED) {
		printf("transfer %s failed\n", p_rawMessage);
		return 0;
	}
	else if (transResult == TRNS_DISCONNECTED) {
		printf("transfer disconnected\n");
		return -1;
	}
	printf("SERVER_APPROVED sent\n");
	while (1) {
		//Go into critical zone
		//open or create the GameSession file and check if this is player one or not
		h_sharedFile = openOrCreateFile(&playerOne);
		if (h_sharedFile == INVALID_HANDLE_VALUE) {
			//do stuff cuz this thread is going down

			return -1;
		}
		//Write the username to the file and increment the (global) number of players
		if (!writeUserNameToFile(h_sharedFile, username, playerOne, &otherUsername)) {
			//leaveGame()
			CloseHandle(h_sharedFile);
			break;
		}
		(*p_players)++;
		//leave critical zone
		CloseHandle(h_sharedFile);

		//<------Main menu------->
		printf("sending SERVER_MAIN_MENU\n");
		char* p_rawMessage = "SERVER_MAIN_MENU\n";
		transResult = SendString(p_rawMessage, socket);
		if (transResult != TRNS_SUCCEEDED) {
			//leaveGame()
			(*p_players)--;
			if (TRNS_SUCCEEDED) {
				//disconnected
				break;
			}//sent failed
			break; //change to continue
		}
		printf("SERVER_MAIN_MENU sent\n");
		//<----Player main menu response---->
		printf("Getting response from client\n");
		transferred = getMessage(socket, &message, 15000);
		if (transferred != 1) {
			if (transferred){
				printf("No response from user in Main menu\n");
				if (playerOne) { //If this is player one, delete the file
					//leaveGame() <- delete file if needed, close file handle decrement players.
					if (!DeleteFileW(sharedFileName)) { // Had problems deleteing the file
						printf("Trouble deleting %s file. Quitting\n", GAMESESSION_FILENAME);
						break;
					}
				}
			} //go back to main menu
			break; //should be continue;
		}
		printf("response is %\n", message->type);
		if (strcmp((message->type), "CLIENT_DISCONNECT") == 0) { //Player chose 2
			free(message);
			if (playerOne) {
				printf("Delete the file!\n");
				if (!DeleteFileW(sharedFileName)) { // Had problems deleteing the file
					printf("Trouble deleting %s file. Quitting\n", GAMESESSION_FILENAME);
				}
			}
			//leaveGame()
			(*p_players)--;
			break;
		}
		else if (strcmp((message->type), "CLIENT_VERSUS") != 0) { //invalid message type
			printf("message type %s is not relevant here \n", message->type);
			free(message); 
			if (playerOne) {
				printf("Delete the file!\n");
				if (!DeleteFileW(sharedFileName)) { // Had problems deleteing the file
					printf("Trouble deleting %s file. Quitting\n", GAMESESSION_FILENAME);
					break;
				}
			}
			//leaveGame()
			(*p_players)--;
			break; //Is this what we want?
		}
		free(message); message = NULL;
		//wait for fileEvent
		if (*p_players != 2) {//not enough players
			//release event
			//leavegame()
			(*p_players)--;
			continue;
		}
		if (playerOne) { //player 1 needs to get player 2's username
			retVal = getPlayer2Name(h_sharedFile, username, &otherUsername);
			if (retVal == NOT_SUCCESS) {
				//release event
				//leavegame()
				(*p_players)--;
				break;
			}
			//release event
		}
		printf("I am %s\nOther player is %s\n", username, otherUsername);
		// <------- Get player's secretNum ----->

		transferred = getMessage(socket, &message, 15000);
		free(message);



				
			
			//get the client's reply 
			//if 2 - decrement number of players, if flag is 1 delete file and disconnect from client gently
			//if 1 - check if there are 2 players. 
				//if only 1 player, decrement players, delete the file, send SERVER_NO_OPPONENTS 
				//and go back to the beginning of the while loop (with continue;)
			//get other player's name and send SERVER_INVITE msg
	
	} // !while(1)

	printf("leaving game.\n");
	closesocket(socket);
	return 0;
}


//I should probably create a function that takes the result of a message 
//and stuff to free and frees things according to the need.

/*
get first message from client
validate the message
username is <- message->username

LOOP:
	trying to start the game
	open file or create it
	validate opening of file
	write the username into the file, increment the number of players
	send main_menu to client
	validate message sent
	get response from client
	create Message, validate it
	if quit - free stuff and quit
	if continue -
	if 2 players, get the other player's username, send client INVITATION and otherUsername
	if 1 player, close the file, send NO_OPPONENT and go back to the beginning of the loop
	from this moment on, if the other player disconnects, close the file (and maybe free memory) and go back to the beginning of the loop
	so we should check if the messages are SERVER_OPPONENT_QUIT

	game on!
	get the number from the client
	validate the message is CLIENT_SETUP (client should validate the value is good)
	if (not client disconnected)
	if (not players<2)
	write secretNum to the file (no need for mutex), acording to playerOne or !playerOne
	wait for other player to also write their secretNum <---------- IMPORTANT
	while()
		get otherSecretNum from file

	send the client a CLIENT_PLAYER_MOVE
	validate sent
	while()
		get the move from client
		validate the message is CLIENT_PLAYER_MOVE
		calculate the bulls and cows, write results to file
		if won, raise playerWins (mutex) raise I_won to 1
		when both players are done checking:
		game_finished = checkVictory(socket, username, otherusername, guess, other guess);
		if !game_finished:	
			read other player's guess
			send SERVER_GAME_RESULTS with other player's guess and its own results
		else
		break;
	back to main manu
		*/


		//checkVictory(socket, username, otherusername, guess, other guess){
		//	if playerWins > 0:
		//		if playerWins == 1 and I_Won, send SERVER_WIN with username and other user's guess
		//		if playerWins == 1 and !I_Won send SERVER_WIN with other username and this user's guess
		//		if playerWins == 2 send SERVER_DRAW
		//}