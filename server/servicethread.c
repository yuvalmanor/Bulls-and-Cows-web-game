#include "servicethread.h"

int mainMenu(socket) {
	char* p_msg = "SERVER_MAIN_MENU\n";
	TransferResult_t transResult;

	transResult = SendString(&p_msg, socket);
	if (transResult == TRNS_SUCCEEDED) {
		return 1;
	}
	else if (transResult == TRNS_FAILED) {
		printf("transfer failed\n");
		//do something
		return -1;
	}
	else {
		printf("transfer disconnected\n");
		//do something
		return -1;
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
	printf("%d\n", GetLastError());
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
	transResult = ReceiveString(&p_msg, socket);
	if (transResult != TRNS_SUCCEEDED) { //If string not received properly
		if (transResult == TRNS_FAILED) {
			printf("transfer failed\n"); 
			//do something
		}
		else {
			printf("transfer disconnected\n");
			//do something
		}
	}

	while (1) {
		printf("got this message: %s", p_msg);
		message = messageDecoder(p_msg);
		if (message == NULL) {
			printf("There was a problem with processing the message\n");
			//free things
			return -1;
		}
		username = message->username;
		printf("Username is %s\n", username);
		//Go into critical zone
		sharedFile = openOrCreateFile(&p_playerOne);
		if (sharedFile == INVALID_HANDLE_VALUE) {
			//do stuff cuz this thread is going down
			return -1;
		}
		printf("PlayerOne: %d\n", p_playerOne);
		//Write the username to the file and increment the (global) number of players
		printf("client %d username is %s\n", offset, username);
			mainMenu(socket);
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

