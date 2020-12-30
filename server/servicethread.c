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

DWORD ServiceThread(ThreadParam* lpParam) {
	//get thread parameters
	ThreadParam* p_param;
	if (NULL == lpParam) {
		printf("Service thread can't work with NULL as parameters\n");
		return -1;
	}
	p_param = (ThreadParam*)lpParam;
	SOCKET socket = p_param->socket;
	int offset = p_param->offset;
	char* username = NULL;
	char* p_msg = NULL;
	TransferResult_t transResult;

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
	username = p_msg;
	//check if file exists, if it doesn't create it and set flag to 1
	//Write the username to the file and increment the (global) number of users
	printf("client %d username is %s\n", offset, username);
		mainMenu(socket);
		//get the client's reply 
		//if 2 - decrement number of players, if flag is 1 delete file and disconnect from client gently
		//if 1 - check if there are 2 players. 
			//if only 1 player, decrement players, delete the file, send SERVER_NO_OPPONENTS 
			//and go back to the beginning of the while loop (with continue;)
		//get other player's name and send SERVER_INVITE msg
		
		

	}


	printf("Im the server thread.\n");
	//communicate with client.
	closesocket(socket);
	return 0;


}

