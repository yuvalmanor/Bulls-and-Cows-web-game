/*
Description – A module that enables communication and syncronization between different threads
*/
#include "ThreadsCommunication.h"

int getEvents(HANDLE* lockEvent, HANDLE* syncEvent, HANDLE* FailureEvent) 
{
	/* Get handle to event by name. If the event doesn't exist, create it */
	(*lockEvent) = CreateEvent(
		NULL, /* default security attributes */
		FALSE,       /* auto-reset event */
		TRUE,      /* initial state is signaled */
		lockEvent_name);         /* name */
	
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
		TRUE,       /* manual-reset event */
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

HANDLE openOrCreateFile(int* p_playerOne) {
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
		(*p_playerOne) = 1;

	}
	else if (INVALID_HANDLE_VALUE == hFile) {
		printf("Can't open %s file Error: %d\n", sharedFile_name, GetLastError());
		return INVALID_HANDLE_VALUE;
	}
	else {
		(*p_playerOne) = 0;

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
	DWORD filePointer, dwBytesRead;
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

int SyncTwoThreads(SOCKET socket, int* p_numOfPlayersSyncing, int* p_numOfPlayersInGame, HANDLE lockEvent, HANDLE syncEvent) {
	DWORD waitcode;
	int retVal;
	waitcode = WaitForSingleObject(lockEvent, LOCKEVENT_WAITTIME);
	if (waitcode != WAIT_OBJECT_0) {
		if (waitcode == WAIT_TIMEOUT) { return DISCONNECTED; }
		else { return NOT_SUCCESS; }
	}
	// < ------- safe zone-------> 
	(*p_numOfPlayersSyncing)++;
	if (*p_numOfPlayersSyncing == 2) { //If this is the second client entering the safe zone, set syncEvent
		if (!SetEvent(syncEvent)) {
			printf("SetEvent failed(SyncTwoThreads) %d\n", GetLastError());
			return NOT_SUCCESS;
		}
	}
	if (!SetEvent(lockEvent)) {
		printf("SetEvent failed(SyncTwoThreads) %d\n", GetLastError());
		return NOT_SUCCESS;
	}
	//<--------end of safe zone------>
	while (1) { //Wait for the other thread to set syncEvent
		waitcode = WaitForSingleObject(syncEvent, POLLING_TIME);//Check if other player was disconnected every POLLING_TIME ms
		if (waitcode != WAIT_OBJECT_0) { //If event was not yet set - check status
			if (waitcode == WAIT_TIMEOUT) {//If POLLING_TIME passed, check if opponent left the game
				retVal = opponentLeftGame(socket, p_numOfPlayersInGame, lockEvent);
				if (GAME_STILL_ON == retVal)//If opponent is stil playing, keep waiting for event
					continue;
				(*p_numOfPlayersSyncing)--;
				return retVal;
			}
			else return NOT_SUCCESS;
		}
		else { //if Event was set by the other thread, continue with the game
			break;
		}
	}
	WaitForSingleObject(lockEvent, LOCKEVENT_WAITTIME);
	if (waitcode != WAIT_OBJECT_0) {
		if (waitcode == WAIT_TIMEOUT) { return DISCONNECTED; }
		else { return NOT_SUCCESS; }
	}

	//<------- safe zone ------->
	(*p_numOfPlayersSyncing)--;
	if (*p_numOfPlayersSyncing == 0) { //If this is the second client entering the safe zone, reset syncEvent
		if (!ResetEvent(syncEvent)) {
			printf("ResetEvent failed(SyncTwoThreads) %d\n", GetLastError());
			return NOT_SUCCESS;
		}
	}
	if (!SetEvent(lockEvent)) {
		printf("SetEvent failed(SyncTwoThreads) %d\n", GetLastError());
		return NOT_SUCCESS;
	}
	//<-------- end of safe zone------>

	return GAME_STILL_ON;
}

int opponentLeftGame(SOCKET socket, int* p_numOfPlayersInGame, HANDLE lockEvent) {
	DWORD waitCode;
	char* p_serverMsg = NULL;
	int status;

	waitCode = WaitForSingleObject(lockEvent, LOCKEVENT_WAITTIME);
	if (WAIT_OBJECT_0 != waitCode) {
		printf("waitForSingleObject error(opponentLeftGame). Error code: %d.\n", GetLastError());
		return NOT_SUCCESS;
	}
	if ((*p_numOfPlayersInGame) != 2) {
		p_serverMsg = prepareMsg("SERVER_OPPONENT_QUIT", NULL);
		if (NULL == p_serverMsg) {
			if (!SetEvent(lockEvent))
				printf("Error when setEvent (opponentLeftGame). Error code: %d.\n", GetLastError());
			return NOT_SUCCESS;
		}
		status = SendString(p_serverMsg, socket);
		free(p_serverMsg);
		if (TRNS_FAILED == status) {
			if (!SetEvent(lockEvent)) {
				printf("Error when setEvent (opponentLeftGame). Error code: %d.\n", GetLastError());
				return NOT_SUCCESS;
			}
			return DISCONNECTED;
		}
		if (!SetEvent(lockEvent)) {
			printf("Error when setEvent (opponentLeftGame). Error code: %d.\n", GetLastError());
			return NOT_SUCCESS;
		}
		return MAIN_MENU;
	}
	if (!SetEvent(lockEvent)) {
		printf("Error when setEvent (opponentLeftGame). Error code: %d.\n", GetLastError());
		return NOT_SUCCESS;
	}
	return GAME_STILL_ON;
}
