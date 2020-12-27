#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "main.h"

int main(int argc, char* argv[]) {
	/*if (argc != 2) {
		printf("Invalid number of arguments\n");
		return -1;
	}
	int portNumber = strToInt(argv[1]);
	if (portNumber < 0 || portNumber >9999) {
		printf("Invalid port number\n");
		return -1;
	}
	serverMain(portNumber);
	return 0;*/
	char string1[] =
		"A string\tof ,,tokens\nand some  more tokens";
	char string2[] =
		"Another string\n\tparsed at the same time.";
	char seps[] = ":";
	char* token1 = NULL;
	char* token2 = NULL;
	char* next_token1 = NULL;
	char* next_token2 = NULL;
	token1 = strtok_s(string1, seps, &next_token1);
	printf(" %s\n", token1);
	if (NULL != next_token1)
		printf("len=%d\n", strlen(next_token1));
	return 0;
}

