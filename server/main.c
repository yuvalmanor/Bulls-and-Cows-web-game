#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "main.h"

int main(int argc, char* argv[]) {
	//convert argv1 to int, check it's a valid port number
	int portNumber = 2345; //should be argv1 after convertion
	serverMain(portNumber);
	return 0;
}

