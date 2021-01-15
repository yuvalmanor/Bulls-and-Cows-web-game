/*
Authors –Oded Rinsky - 304973357, Yuval Manor - TODO
Project – Server
Description – The main c file of the server.
*/
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
	serverManager(portNumber);
	return 0;

}

