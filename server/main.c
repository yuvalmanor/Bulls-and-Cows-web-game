/*
Authors –Oded Rinsky - 304973357, Yuval Manor - 203537162
Project – Server
Description – The main c file of the server.
*/
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "main.h"


int main(int argc, char* argv[]) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	if (argc != 2) {
		printf("Invalid number of arguments\n");
		return -1;
	}
	int portNumber = strToInt(argv[1]);
	if (portNumber < 0 || portNumber >9999) {
		printf("Invalid port number\n");
		return -1;
	}
	serverManager(portNumber);
	return 0;

}

