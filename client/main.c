/*
Authors –Oded Rinsky - 304973357, Yuval Manor - 203537162
Project – Client
Description – The main c file of the client.
*/
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "ClientManager.h"

int main(int argc, char* argv[]) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	int portNumber;
	if (argc != 4) {
		printf("Invalid number of arguments\n");
		return NOT_SUCCESS;
	}
	portNumber = strToInt(argv[2]);
	if (portNumber < 0 || portNumber >9999) {
		printf("Invalid port number\n");
		return NOT_SUCCESS;
	}
	clientManager(argv[1], portNumber, argv[3]);
	
	return 0;
 }