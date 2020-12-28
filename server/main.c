#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "main.h"

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("Invalid number of arguments\n");
		return -1;
	}
	int portNumber = strToInt(argv[1]);
	if (portNumber < 0 || portNumber >9999) {
		printf("Invalid port number\n");
		return -1;
	}
	serverMain(portNumber);
	return 0;
	
	
	//messageDecoder("SERVER_GAME_RESULTS:1;2;oded;3456\n");
	return 0;
}

