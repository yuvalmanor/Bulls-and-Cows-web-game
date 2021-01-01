#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "clientManager.h"

int clientManager(char* ip, int portNumber, char* username) {
	SOCKADDR_IN clientService;
	SOCKET c_socket;
	unsigned long Address;
	

	//<--------Initialize Winsock------->
	if (NOT_SUCCESS == InitializeWinsock()) {
		return NOT_SUCCESS;
	}
	// <-----Create a socket----->   
	c_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	printf("Socket created.\n");
	if (c_socket == INVALID_SOCKET)
	{
		//Free resources, WSACleanup and end program with -1
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		resourcesManager(NULL, CLEAN);
		return NOT_SUCCESS;
	}
	//<------- Create a sockaddr_in object and set its values ----->
	Address = inet_addr(ip); //------>Is this an unsafe function?
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			LOCALHOST);
		resourcesManager(c_socket, CLEAN);
		return NOT_SUCCESS;
	}
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = Address;
	clientService.sin_port = htons(portNumber);

	//<---connect client to server--->
	if (EXIT == makeConnection(c_socket, clientService, ip, portNumber)) {
		resourcesManager(c_socket, CLEAN);
		return EXIT;
	}
	/*<---send server username--->*/
	if (SUCCESS != setup(username, c_socket, clientService, ip, portNumber)) {
		resourcesManager(c_socket, CLEAN);
		return EXIT;
	}
	
	playGame(username, c_socket);

	resourcesManager(c_socket,CLEAN);
}

int makeConnection(SOCKET c_socket, SOCKADDR_IN clientService, char* ip, int portNumber) {
	int choice;

	while (1) {
		if (connect(c_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
			
			choice = menu(FAILURE, ip, portNumber);
			if (1 == choice) {
				resourcesManager(c_socket, 0);
				continue;
			}
			else if (2 == choice)
			{
				resourcesManager(c_socket, CLEAN);
				return EXIT;
			}

		}
		else
			break;

	}
	return SUCCESS;
}
int setup(char* username, SOCKET c_socket, SOCKADDR_IN clientService, char* ip, int portNumber) {
	char* clientRequest = NULL, recvMsg = NULL;
	int res, choice;
	Message* serverMsg=NULL;

	clientRequest = prepareMsg("CLIENT_REQUEST:", username);
	if (NULL == clientRequest) return NOT_SUCCESS;
	while (1)
	{
		
		res = SendString(clientRequest, c_socket);
		/*HERE SHOULD CHECK IF TRNS_FAILED OR NOT AND HANDLE IT, ALSO WHAT ABOUT TIMEOUT
		if (TRNS_FAILED == res) {
		free(clientRequest);
		return NOT_SUCCESS;
		}	*/
		//after send, should free(clientRequest)?
		res = ReceiveString(&recvMsg, c_socket);
		/*HERE SHOULD CHECK IF TRNS_FAILED OR NOT AND TRNS_DISCONNECTED AND
		HANDLE IT, ALSO WHAT ABOUT TIMEOUT*/
		if (TRNS_SUCCEEDED != res) {
			if (SUCCESS != checkTRNSCode(res, ip, portNumber, c_socket, clientService))
				return NOT_SUCCESS;
			else
				continue;
		}
		serverMsg = messageDecoder(recvMsg);
		if (NULL == serverMsg) {
			free(serverMsg);
			return NOT_SUCCESS;
		}
		if (!strcmp(serverMsg, "SERVER_APROVED")) {
			free(serverMsg);
			return SUCCESS;
		}
		else if (!strcmp(serverMsg, "SERVER_DENIED")) {
			free(serverMsg);
			choice = menu(DENIED, ip, portNumber);
			if (1 == choice) {
				if (EXIT == makeConnection(c_socket, clientService, ip, portNumber))
					return EXIT;
				else
					continue;
			}
			else if (2 == choice)
				return EXIT;
		}
	}
	

	return SUCCESS;
}
void resourcesManager(SOCKET c_socket, int WSACleanFlag) {

	if (NULL != c_socket) {
		if (closesocket(c_socket) == SOCKET_ERROR)
			printf("Failed to close clientSocket in resourcesManager. error %ld\n", WSAGetLastError());
	}
	if (CLEAN == WSACleanFlag) {
		if (WSACleanup() == SOCKET_ERROR)
			printf("Failed to close Winsocket in resourcesManager. error %ld\n", WSAGetLastError());
	}
	
	
}
int checkTRNSCode(int TRNSCode, char* ip, int portNumber, SOCKET c_socket, SOCKADDR_IN clientService) {
	int choice;

	if (TRNSCode == TRNS_FAILED) {
		printf("Socket error while trying to write data to socket\n");
		return NOT_SUCCESS;
	}
	else if (TRNSCode == TRNS_DISCONNECTED || TRNSCode == TRNS_TIMEOUT) {
		choice = menu(FAILURE, ip, portNumber);
		if (1 == choice) {
			if (EXIT == makeConnection(c_socket, clientService, ip, portNumber))
				return EXIT;
			else
				return SUCCESS;
		}
		else if (2 == choice)
			return EXIT;
	}
	
}
int menu(int menu, char* ip, int portNumber) {
	int choice;

	switch (menu) {
	case MAIN:
		printf("Choose what to do next:\n");
		printf("1. Play against another client\n");
		printf("2. Quit\n");
		break;
	case FAILURE:
		printf("Failed connecting to server on %s:%d.\n", ip, portNumber);
		printf("Choose what to do next:\n");
		printf("1. Try to reconnect\n");
		printf("2. Exit\n");
		break;
	case DENIED:
		printf("Server on %s:%d denied the connection request.\n");
		printf("Choose what to do next:\n");
		printf("1. Try to reconnect\n");
		printf("2. Exit\n");
		break;
	}

	choice = playerChoice();
	return choice;

}