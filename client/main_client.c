#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "clientManager.h"



int main(int argc, char* argv[]) {
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