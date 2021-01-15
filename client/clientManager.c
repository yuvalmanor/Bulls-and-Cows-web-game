#include "clientManager.h"

int clientManager(char* ip, int portNumber, char* username) {
	SOCKADDR_IN clientService;
	SOCKET c_socket;

	//<--------Initialize Winsock------->
	if (NOT_SUCCESS == InitializeWinsock()) {
		return NOT_SUCCESS;
	}
	// <-----Create a socket----->   
	c_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (c_socket == INVALID_SOCKET)
	{
		//Free resources, WSACleanup and end program with -1
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		resourcesManager(INVALID_SOCKET, CLEAN);
		return NOT_SUCCESS;
	}
	//<------- Create a sockaddr_in object and set its values ----->
	if (SUCCESS != initAddress(ip, portNumber, &clientService)) {
		printf("Error at when initializing address: %ld\n", WSAGetLastError());
		resourcesManager(c_socket, CLEAN);
		return NOT_SUCCESS;
	}
	//<---connect client to server--->
	if (EXIT == makeConnection(c_socket, clientService, ip, portNumber)) {
		resourcesManager(c_socket, CLEAN);
		return EXIT;
	}
	printf("Connected to %s:%d\n", ip, portNumber);
	
	playGame(username, c_socket, clientService, ip, portNumber);

	resourcesManager(INVALID_SOCKET,CLEAN);
	return 1;
}



