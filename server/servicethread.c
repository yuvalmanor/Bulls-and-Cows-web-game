#include "servicethread.h"

typedef const char* LPCSTR;

static LPCSTR lockEvent_name = "lockEvent";
static LPCSTR syncEvent_name = "syncEvent";
static LPCSTR failureEvent_name = "Failure";
static LPCSTR sharedFile_name = "GameSession.txt";

DWORD ServiceThread(void* lpParam) {
	//get thread parameters
	ThreadParam* p_param;
	if (NULL == lpParam) {
		printf("Service thread can't work with NULL as parameters\n");
		return NOT_SUCCESS;
	}
	p_param = (ThreadParam*)lpParam;
	DWORD waitcode;
	SOCKET socket = p_param->socket;
	int playerOne, retVal, threadRetVal = 0, gameStatus = MAIN_MENU;
	int *p_players = p_param->p_players;
	int* p_PlayersCount = p_param->p_PlayersCount;
	char* username = NULL, * otherUsername = NULL, * secretNum = NULL;
	char* otherSecretNum = NULL, *guess, *otherGuess=NULL, p_msg = NULL;
	Message* message = NULL;
	HANDLE h_sharedFile = NULL, lockEvent = NULL, syncEvent = NULL, failureEvent = NULL;
	TransferResult_t transResult;

	if (NOT_SUCCESS == getEvents(&lockEvent, &syncEvent, &failureEvent)) {
		return NOT_SUCCESS;
	}

	retVal = getUserNameAndApproveClient(socket, &username);
	if (retVal == NOT_SUCCESS) {
		leaveGame(socket, lockEvent, NULL, NULL, NULL);
		return NOT_SUCCESS;
	}
	 // TODO: Add checkings of return values to the next 4 lines
	waitcode = WaitForSingleObject(lockEvent, LOCKEVENT_WAITTIME);
	(*p_players)++;
	SetEvent(lockEvent);

	// <-------- entering game loop ------->
	while (1) {
		retVal = Main_menu(socket, lockEvent, syncEvent, p_players, &playerOne, username, &otherUsername);
		if (retVal != GAME_STILL_ON) {
			if (retVal == MAIN_MENU) {
				if (playerOne) {
					DeleteFileA(sharedFile_name);
				}
				continue;
			}
			if (retVal == QUIT) {
				printf("Player opted to quit\n");
			}
			if (retVal == DISCONNECTED) {
				printf("Client disconnected abruptly\n\n");
			}
			if (retVal == NOT_SUCCESS) {
				printf("Main_menu failed\n");
			}
			break;
		}
		printf("I am %s\nOther player is %s\n", username, otherUsername);
		
		// <--------- PART 2 --------->
		h_sharedFile = CreateFileA(sharedFile_name,
			GENERIC_ALL,
			(FILE_SHARE_READ|FILE_SHARE_DELETE|FILE_SHARE_WRITE),
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if (INVALID_HANDLE_VALUE == h_sharedFile) {
			printf("Can't open %s file Error: %d\n", sharedFile_name, GetLastError());
			break; //Leave game
		}
		retVal = startGame(socket, h_sharedFile, lockEvent, syncEvent, playerOne, p_players, username, otherUsername, p_PlayersCount);
		CloseHandle(h_sharedFile);
		if (playerOne) {
			DeleteFileA(sharedFile_name);
		}
		if (retVal == NOT_SUCCESS) {
			printf("The game ended with a failure\n");
			break;
		}
	} // !while(1)
	if (playerOne) {
		DeleteFileA(sharedFile_name);
	}
	free(username);
	free(otherUsername);
	free(otherGuess);
	free(otherSecretNum);
	waitcode = WaitForSingleObject(lockEvent, LOCKEVENT_WAITTIME); //TODO add validation of waitcode
	(*p_players)--;
	SetEvent(lockEvent);//TODO add validation of setEvent
	if (retVal == QUIT)
		confirmShutdown(socket);
	else {
		closesocket(socket);
		if (retVal == NOT_SUCCESS) {
			if (!SetEvent(failureEvent)) {
				printf("Error in SetEvent(failureEvent). Other threads will not be terminated\n");
			}
		}
	}
	printf("serviceThread finished.\n");
	return 0;
}

int Main_menu(SOCKET socket, HANDLE lockEvent, HANDLE syncEvent, int* p_players, int* playerOne, char* username, char** otherUsername) {
	TransferResult_t transResult;
	int retVal, offset = 0;
	DWORD waitcode;
	Message* message = NULL;
	HANDLE h_sharedFile = NULL;

	printf("sending SERVER_MAIN_MENU\n");
	char* p_rawMessage = "SERVER_MAIN_MENU\n";
	transResult = SendString(p_rawMessage, socket); //Send client MAIN_MENU
	if (transResult != TRNS_SUCCEEDED) {
		if (transResult == TRNS_DISCONNECTED)
			return DISCONNECTED;
		else
			return NOT_SUCCESS;
	}
	retVal = getMessage(socket, &message, INFINITE); //Get response from client
	if (retVal != TRNS_SUCCEEDED) {
		if (retVal == TRNS_DISCONNECTED)
			return DISCONNECTED;
		else
			return NOT_SUCCESS;
	}
	if (!strcmp(message->type, "CLIENT_DISCONNECT")) { //If client chose to quit
		free(message);
		return QUIT;
	}
	else if (strcmp(message->type, "CLIENT_VERSUS")) {//If an invalid response
		free(message);
		return NOT_SUCCESS;
	}
	//else- client chose CLIENT_VERSUS
	free(message); message = NULL;
	//----> Go to critical section
	//mainMenuCriticalSection(lockEvent, p_players)
	waitcode = WaitForSingleObject(lockEvent, LOCKEVENT_WAITTIME);
	if (waitcode != WAIT_OBJECT_0) {
		printf("Waitcode is %d\nError code %d while waiting for lockEvent\n", waitcode, GetLastError());
		return NOT_SUCCESS;
	}
	if (*p_players != 2) {
		if (*p_players > 2) {
			printf("Error: incorrect number of players\n");
			if (!SetEvent(lockEvent))  //release lockEvent
				printf("SetEvent failed %d\n", GetLastError());
			return NOT_SUCCESS;
		}
		printf("There are only %d players\n", *p_players);
		if (!SetEvent(lockEvent)) { //release lockEvent
			printf("SetEvent failed %d\n", GetLastError());
			return NOT_SUCCESS;
		}//Send client SERVER_NO_OPPONENTS
		printf("sending SERVER_NO_OPPONENTS\n");
		char* p_rawMessage = "SERVER_NO_OPPONENTS\n";
		transResult = SendString(p_rawMessage, socket);
		if (transResult != TRNS_SUCCEEDED) {
			if (transResult == TRNS_DISCONNECTED)
				return DISCONNECTED;
			else
				return NOT_SUCCESS;
		}
		return MAIN_MENU;
	}
	//if there are 2 players
	h_sharedFile = openOrCreateFile(playerOne); //why is playerOne always 0?
	printf("%s: playerOne = %d\n", username, *playerOne);
	if (h_sharedFile == INVALID_HANDLE_VALUE) {
		if (!SetEvent(lockEvent)) { //release lockEvent
			printf("SetEvent failed %d\n", GetLastError());
		}
		return NOT_SUCCESS;
	}

	if (!*playerOne) { //If this is player 2
		//get player 1's name
		if (NOT_SUCCESS == readFromFile(h_sharedFile, 0, otherUsername, *playerOne, 1)) {
			CloseHandle(h_sharedFile);
			SetEvent(lockEvent);
			return NOT_SUCCESS;
		}
		offset = strlen(*otherUsername) + 1;
	}
	//Write username to the file
	if (NOT_SUCCESS == writeToFile(h_sharedFile, offset, username, *playerOne, 1)) {
		CloseHandle(h_sharedFile);
		SetEvent(lockEvent);
		return NOT_SUCCESS;
	}
	//if Player 2: username to the file, release syncEvent for player 1 to read their username from the file.
	if (!*playerOne) {
		if (!SetEvent(syncEvent)) { //release syncEvent
			printf("SetEvent failed %d\n", GetLastError());
			CloseHandle(h_sharedFile);
			SetEvent(lockEvent);
			return NOT_SUCCESS;
		}
	}

	if (!SetEvent(lockEvent)) { //release lockEvent
		printf("SetEvent failed %d\n", GetLastError());
		CloseHandle(h_sharedFile);
		return NOT_SUCCESS;
	}
	//---->out of the critical section
	if (*playerOne) {
		waitcode = WaitForSingleObject(syncEvent, DOUBLE_RESPONSE_WAITTIME);
		if (!ResetEvent(syncEvent)) { //lock syncEvent
			printf("ResetEvent failed %d\n", GetLastError());
			CloseHandle(h_sharedFile);
			return NOT_SUCCESS;
		}
		if (waitcode != WAIT_OBJECT_0) {
			CloseHandle(h_sharedFile);
			if (waitcode == WAIT_TIMEOUT) { //The other player did not respond within 30 seconds
				printf("sending SERVER_NO_OPPONENTS\n");
				char* p_rawMessage = "SERVER_NO_OPPONENTS\n";
				transResult = SendString(p_rawMessage, socket);
				if (transResult != TRNS_SUCCEEDED) { //message was not sent
					if (transResult == TRNS_DISCONNECTED)
						return DISCONNECTED;
					else
						return NOT_SUCCESS;
				}
				else {
					return MAIN_MENU; //message was sent. go back to main menu
				}
			}
			else {
				printf("There was a problem with waiting for syncEvent\n");
				return NOT_SUCCESS;
			}
		}
		offset = strlen(username) + 1;
		if (NOT_SUCCESS == readFromFile(h_sharedFile, offset, otherUsername, *playerOne, 1)) {//get player 2's name
			CloseHandle(h_sharedFile);
			return NOT_SUCCESS;
		}
	}
	CloseHandle(h_sharedFile);
	return GAME_STILL_ON;
}


//I should probably create a function that takes the result of a message 
//and stuff to free and frees things according to the need.

/*
threadRetVal = 0;
gameStatus = GAME_STILL_ON;
get first message from client
validate the message
username = message->username
send client SERVER_APPROVED

down(lockEvent)
(*p_players)++
release(lockEvent)

LOOP:{
	Send client MAIN_MENU
	getMessage()
	if message is not CLIENT_VERSUS -> break;
	down(lockEvent) (go into danger zone)
	if (*p_players != 2){
		release(lockEvent);
		sendMessage(SERVER_NO_OPPONENTS)
		continue; //go back to main_menu
		}
	openOrCreateFile();
	validate opening of file
	if created new -> playerOne = 1
	else if opened an existing file -> playerOne = 0
	if not playerOne{
		read otherUsername from the file
		release(syncEvent)
	}
	write the username into the file
	CloseHandle(file)
	release(lockEvent)
	if (playerOne){
		waitcode = waitForSingleObject(syncEvent, 30 sec)
		if waitcode != 0 (didn't work){
				deleteFile(file)
			if waitcode is TIMEDOUT:
				send client SERVER_NO_OPPONENTS
				validate the message
				continue; //go back to main menu
			else{
				gameStatus = NOT_SUCCESS;
				break; //leave program
			}
		}
		else{ //both clients sent CLIENT_VERSUS
			getPlayer2Name() //This function uses lockEvent so as long as player2 is not finished writing, player1 will wait
		}
	}
	if (otherPlayerLeftGame){
		send SERVER_OPPONENT_QUIT 
		go back to main menu
	}
	send client SERVER_INVITE with the otherUsername
	validate the message sent

	<--------------- PART II --------------->
	There are two players and they know each other names
	if (otherPlayerLeftGame()){
		send SERVER_OPPONENT_QUIT 
		go back to main menu
	}
	send SERVER_SETUP_REQUSET
	validate message sent -> if not, leaveGame() break;
	getMessage()
	validate message received -> if not, leaveGame() break;
	if message->type != CLIENT_SETUP -> leaveGame() break;
	secretNum = message->guess
	retVal = writeSecretNumToFile(h_file, playerOne, secretNum);
	check if retVal suceeded
	if (otherPlayerLeftGame()){
		send SERVER_OPPONENT_QUIT
		go back to main menu
	}
	WAIT FOR BOTH OPPONENTS TO WRITE NUM AND GET THE OTHER SECRETNUM??

	game on!
	At this point, both players have secretNum and otherSecretNum
	while(1){
		if (otherPlayerLeftGame()){
			send SERVER_OPPONENT_QUIT
			gameStatus = MAIN_MENU
			break;
		}
		send SERVER_PLAYER_MOVE_REQUEST
		validate message sent -> if not, gameStatus = FAILED, break;
		getMessage()
		validate message received -> if not, gameStatus = FAILED, break;
		if message->type != CLIENT_PLAYER_MOVE -> gameStatus = FAILED, break;
		guess = message->guess

		IWin = getResult(guess, otherSecretNum, &result)
		retVal = writeGuessAndResultToFile(h_file, playerOne, guess, result)
		validate retVal is good -> if not, gameStatus = FAILED, break;
		if (IWin){
			waitcode = waitForSingleObject(lockEvent);
			check waitcode if there was a problem -> release(lockEvent), gameStatus = FAILED, break;
			*(p_playerWins)++;
			release(lockEvent);

		if (otherPlayerLeftGame()){
			send SERVER_OPPONENT_QUIT
			gameStatus = MAIN_MENU
			break;
		}
		WAIT FOR BOTH PLAYERS TO WRITE RESULT AND GUESS TO FILE <---

		getOtherPlayerGuessAndResult(h_file, &otherGuess, &otherResult)
		gameStatus = GameStatus(socket, username, othersernaUme, guess, otherGuess, otherResult, IWin)
		if (gameStatus != GAME_STILL_ON){ //No need for more guesses (Someone won or there was an error)
			break;
		}
	}// end of the guessing while loop
	if (gameStatus == FAILED){
		threadRetVal = -1;
		break;
	}
}	
	//If you are here, you're leaving the game
	down(lockEvent);
	*(p_players)--;
	release(lockEvent);
	closesocket(socket);
	return threadRetVal;
		*/
/*
		int otherPlayerLeftGame(SOCKET socket, int* p_players, Message* message){
			waitcode = waitForSingleObject(lockEvent, waittime):
			if (waitcode != WAIT_OBJECT_0)
			if (*(p_players) != 2){
				SetEvent(lockEvent);
				return 0;
			}
			SetEvent(lockEvent);
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
			return GAME_STILL_ON
		}



	Manager:
			lockEvent- Auto -> Signaled
			syncEvent - Manual->  Non-Signaled
			PlayerCount = 0

<-------------------HOW TO USE SYNCTWOTHREADS():---------------------->
	ServiceThread:
		Do something

		retVal = syncTwoThreads(p_PlayerCount, lockEvent, syncEvent);
		if (retVal == NOT_SUCCESS){
			return NOT_SUCCESS;
		}
		else if (retVal == DISCONNECTED){
			ResetEvent(syncEvent);
			SetEvent(lockEvent)
			Playersount = 0;
			return DISCONNECTED;
		}
<-------------------!HOW TO USE SYNCTWOTHREADS():---------------------->

		*/

int getUserNameAndApproveClient(SOCKET socket, char** username) {
	int retVal;
	Message* message = NULL;
	printf("Waiting for username from client\n");
	
	retVal = getMessage(socket, &message, 15000); //Change waitTime to a DEFINED number 
	if (retVal != TRNS_SUCCEEDED) {
		printf("couldn't get username from client. Quitting\n");
		return NOT_SUCCESS;
	}
	if (strcmp((message->type), "CLIENT_REQUEST") != 0) {
		printf("message type is invalid, %s instead of CLIENT_REQUEST\n", message->type);
		free(message);
		return NOT_SUCCESS;
	}
	(*username) = message->username;
	free(message);
	printf("Username is %s\n", *username);
	printf("sending SERVER_APPROVED\n");
	char* p_rawMessage = "SERVER_APPROVED\n";
	retVal = SendString(p_rawMessage, socket);
	if (retVal != TRNS_SUCCEEDED) {
		printf("Transfer failed when sending %s\n", p_rawMessage);
		free(*username);
		return NOT_SUCCESS;
	}

	printf("SERVER_APPROVED sent\n");
	return SUCCESS;
}

HANDLE openOrCreateFile(int* playerOne) {
	DWORD dwDesiredAccess = 0, dwShareMode = 0, dwCreationDisposition = 0;
	HANDLE hFile;
	dwDesiredAccess = GENERIC_ALL;
	dwShareMode = (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE);
	//Try to open existing file:
	dwCreationDisposition = OPEN_EXISTING;
	hFile = CreateFileA(sharedFile_name,
		dwDesiredAccess,
		dwShareMode,
		NULL,
		dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	//if it doesn't exist, create a new file and set playerOne to 1
	if (GetLastError() == ERROR_FILE_NOT_FOUND) {
		dwCreationDisposition = CREATE_NEW;
		hFile = CreateFileA(sharedFile_name,
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
		printf("Can't open %s file Error: %d\n", sharedFile_name, GetLastError());
		return INVALID_HANDLE_VALUE;
	}
	else {
		printf("An existing file was opened\nPlayerOne: 0\n ");
		(*playerOne) = 0;

	}
	return hFile;
}

int writeToFile(HANDLE h_file, int offset, char* data, int playerOne, int writeUsername) {
	DWORD dwBytesWritten, filePointer;
	int numOfBytesToWrite = (int)strlen(data);
	char* buffer = NULL;
	if (writeUsername) { //Write to offset +32
		offset += 2 * DATABLOCKSIZE;
	}
	else {
		offset += playerOne * DATABLOCKSIZE;
	}

	filePointer = SetFilePointer(h_file, offset, NULL, FILE_BEGIN);
	if (INVALID_SET_FILE_POINTER == filePointer) {
		printf("File pointer failed to move\nError code:%d\n", GetLastError());
		return NOT_SUCCESS;
	}
	if (FALSE == WriteFile(h_file, data, numOfBytesToWrite, &dwBytesWritten, NULL)) {
		printf("File write failed.\nError code:%d\n", GetLastError());
		return NOT_SUCCESS;
	}
	if (FALSE == WriteFile(h_file, "\r\n", 1, &dwBytesWritten, NULL)) {
		printf("File write failed.\nError code:%d\n", GetLastError());
		return NOT_SUCCESS;
	}
	return SUCCESS;
}

int readFromFile(HANDLE h_sharedFile, int offset, char** data, int playerOne, int readUsername) {
	DWORD dwBytesWritten, filePointer, dwBytesRead;
	int numOfBytesToRead = SECRETNUMBER_LEN + 2;
	char* buffer = NULL;
	if (readUsername) {
		offset += 2 * DATABLOCKSIZE;
		numOfBytesToRead = MAX_USERNAME_LEN;
	}
	else {
		offset += (!playerOne) * DATABLOCKSIZE;
	}

	filePointer = SetFilePointer(h_sharedFile, offset, NULL, FILE_BEGIN);
	if (INVALID_SET_FILE_POINTER == filePointer) {
		printf("File pointer failed to move\nError code:%d\n", GetLastError());
		return NOT_SUCCESS;
	}
	if (NULL == (buffer = (char*)malloc(numOfBytesToRead))) {
		printf("Fatal error: memory allocation failed (ReadFromFile).\n");
		return NOT_SUCCESS;
	}
	if (FALSE == ReadFile(h_sharedFile, buffer, numOfBytesToRead, &dwBytesRead, NULL)) {
		printf("File read failed.\nError code:%d\n", GetLastError());
		free(buffer);
		return NOT_SUCCESS;
	}
	//Place \0 at the right place
	for (int i = 0; i < numOfBytesToRead; i++) {
		if (buffer[i] == '\r') {
			buffer[i] = '\0';
			break;
		}
	}
	*data = buffer;
	return SUCCESS;
}

void leaveGame(SOCKET socket, HANDLE lockEvent, int* p_players, HANDLE h_sharedFile, Message* message) {
	//TODO
	int x = 1;
}


int getEvents(HANDLE* lockEvent, HANDLE* syncEvent, HANDLE* FailureEvent) // CHECK THIS
{
	/* Get handle to event by name. If the event doesn't exist, create it */
	(*lockEvent) = CreateEvent(
		NULL, /* default security attributes */
		FALSE,       /* auto-reset event */
		TRUE,      /* initial state is signaled */
		lockEvent_name);         /* name */
	/* Check if succeeded and handle errors */
	if (*lockEvent == NULL) {
		printf("Counldn't create Event. Error: %d\n", GetLastError());
		return NOT_SUCCESS;
	}
	(*syncEvent) = CreateEvent(
		NULL, /* default security attributes */
		TRUE,       /* manual-reset event */
		FALSE,      /* initial state is non-signaled */
		syncEvent_name);         /* name */
	if (*syncEvent == NULL) {
		printf("Counldn't create Event. Error: %d\n", GetLastError());
		CloseHandle(*lockEvent);
		return NOT_SUCCESS;
	}
	(*FailureEvent) = CreateEvent(
		NULL, /* default security attributes */
		FALSE,       /* auto-reset event */
		FALSE,      /* initial state is non-signaled */
		failureEvent_name);         /* name */
	if (*FailureEvent == NULL) {
		printf("Counldn't create Event. Error: %d\n", GetLastError());
		CloseHandle(*lockEvent);
		CloseHandle(*syncEvent);
		return NOT_SUCCESS;
	}

	return SUCCESS;
}

int SyncTwoThreads(int* p_PlayersCount, HANDLE lockEvent, HANDLE syncEvent, int waitTime) {
	DWORD waitcode;
	waitcode = WaitForSingleObject(lockEvent, waitTime);
	if (waitcode != WAIT_OBJECT_0) {
		if (waitcode == WAIT_TIMEOUT) { return DISCONNECTED; }
		else { return NOT_SUCCESS; }
	}
	// < ------ - safe zone-------> ////CHECK SetEvent ERROR CODE!
		(*p_PlayersCount)++;
		if (*p_PlayersCount == 2) {
			SetEvent(syncEvent);
		}
		SetEvent(lockEvent);
		//<--------end of safe zone------>

		WaitForSingleObject(syncEvent, waitTime);
		if (waitcode != WAIT_OBJECT_0) {
			if (waitcode == WAIT_TIMEOUT) { return DISCONNECTED; }
			else { return NOT_SUCCESS; }
		}

		WaitForSingleObject(lockEvent, waitTime);
		if (waitcode != WAIT_OBJECT_0) {
			if (waitcode == WAIT_TIMEOUT) { return DISCONNECTED; }
			else { return NOT_SUCCESS; }
		}

		//<------- safe zone ------->
		(*p_PlayersCount)--;
		if (*p_PlayersCount == 0) {
		ResetEvent(syncEvent);
	}
	SetEvent(lockEvent);
	//<-------- end of safe zone------>

	return GAME_STILL_ON;
}