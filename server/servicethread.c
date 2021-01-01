#include "servicethread.h"

char* mainMenu(socket) {
	char* p_msg = "SERVER_MAIN_MENU\n";
	TransferResult_t transResult;

	transResult = SendString(&p_msg, socket);
	if (transResult == TRNS_SUCCEEDED) {
		return p_msg;
	}
	else if (transResult == TRNS_FAILED) {
		printf("transfer failed\n");
		//do something
		return NULL;
	}
	else {
		printf("transfer disconnected\n");
		//do something
		return NULL;
	}
}

HANDLE openOrCreateFile(int *playerOne) {
	DWORD dwDesiredAccess = 0, dwShareMode = 0, dwCreationDisposition = 0;
	HANDLE hFile;
	dwDesiredAccess = (GENERIC_READ | GENERIC_WRITE);
	dwShareMode = FILE_SHARE_READ;
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
	printf("Last error is %d\n", GetLastError());
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

	}
	else if (INVALID_HANDLE_VALUE == hFile) {
		printf("Can't open %s file\n", GAMESESSION_FILENAME);
		return INVALID_HANDLE_VALUE;
	}
	else {
		(*playerOne) = 0;
	
	}
	return hFile;
}

int writeUserNameToFile(HANDLE h_file, char* username, int playerOne, char** otherUsername) {
		DWORD dwBytesWritten, filePointer, dwBytesRead;
		int numOfBytesToWrite = (int)strlen(username);
		char* buffer = NULL;
		if (NULL == (buffer = (char*)malloc(MAX_USERNAME_LEN))) {
			printf("Fatal error: memory allocation failed (writeUserNameToFile).\n");
			return 0;
		}

		filePointer = SetFilePointer(h_file, 36, NULL, FILE_BEGIN); //Change 36 to a DEFINED
		if (INVALID_SET_FILE_POINTER == filePointer) {
			printf("File pointer failed to move\nError code:%d\n", GetLastError());
			free(buffer);
			return 0;
		}
		if (!playerOne) { //this is the second client. Read the first client's username and add yours afterwards
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
			free(buffer);
		} //Will this bring us to the right place in the file?
		if (FALSE == WriteFile(h_file, username, numOfBytesToWrite, &dwBytesWritten, NULL)) {
			printf("File write failed.\nError code:%d\n", GetLastError());
			return 0;
		return 1;
	}
}

DWORD ServiceThread(ThreadParam* lpParam) {
	//get thread parameters
	ThreadParam* p_param;
	if (NULL == lpParam) {
		printf("Service thread can't work with NULL as parameters\n");
		return -1;
	}
	p_param = (ThreadParam*)lpParam;
	SOCKET socket = p_param->socket;
	int offset = p_param->offset, p_playerOne;
	char* username = NULL, otherUsername = NULL;
	char* p_msg = NULL;
	Message* message = NULL;
	TransferResult_t transResult;
	HANDLE sharedFile = NULL;
	printf("Waiting for username from client\n");
		//Get username from client
	transResult = ReceiveString(&p_msg, socket);
	if (transResult == TRNS_FAILED) {
		printf("transfer failed\n"); 
		//do something
	}
	else if (transResult == TRNS_DISCONNECTED) {
		printf("transfer disconnected\n");
		//do something
	}
		printf("got this message: %s", p_msg);
		message = messageDecoder(p_msg);
		if (message == NULL) {
			printf("There was a problem with processing the message\n");
			//free things
			return -1;
		}
		if (strcmp((message->type), "CLIENT_REQUEST") != 0)  {
			printf("message type is invalid, %s instead of CLIENT_REQUEST\n", message->type);
			//free things
			return -1;
		}
		username = message->username;
		free(message); message = NULL;
		printf("Username is %s\n", username);

	while (1) {
		//Go into critical zone
		//open or create the GameSession file and check if this is player one or not
		sharedFile = openOrCreateFile(&p_playerOne);
		if (sharedFile == INVALID_HANDLE_VALUE) {
			//do stuff cuz this thread is going down
			return -1;
		}
		printf("PlayerOne: %d\n", p_playerOne);
		//Write the username to the file and increment the (global) number of players
		
		//leave critical zone
		printf("client %d username is %s\n", offset, username);
		//<------Main menu------->
		p_msg = "SERVER_MAIN_MENU\n";

		transResult = SendString(&p_msg, socket);
		if (transResult == TRNS_FAILED) {
			printf("transfer %s failed\n", p_msg);
			//handle a failed transfer (Try again)?
			return NULL;
		}
		else if (transResult == TRNS_DISCONNECTED) {
			printf("transfer disconnected\n");
			//do something
			return NULL;
		}

				
			
			//get the client's reply 
			//if 2 - decrement number of players, if flag is 1 delete file and disconnect from client gently
			//if 1 - check if there are 2 players. 
				//if only 1 player, decrement players, delete the file, send SERVER_NO_OPPONENTS 
				//and go back to the beginning of the while loop (with continue;)
			//get other player's name and send SERVER_INVITE msg
	
	} // !while(1)


	printf("Im the server thread.\n");
	//communicate with client.
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
	while()
		get the number from the client
		validate the message is CLIENT_SETUP (client should validate the value is good)
		if so break;
	write it to the file (no need for mutex)
	while()
		read value of the other player, if it's valid, save it and break (mutex?)
		otherwise continue
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