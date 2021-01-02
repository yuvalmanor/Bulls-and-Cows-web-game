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
	
	playGame(username, c_socket, clientService, ip, portNumber);

	resourcesManager(c_socket,CLEAN);
}



