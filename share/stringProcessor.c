#include "stringProcessor.h"

int strToInt(char* stringNum) {
	int result;
	if (strcmp(stringNum, "0\n") == 0 || strcmp(stringNum, "0") == 0) //If the number read is 0, don't use atoi
		result = 0;
	else if (0 == (result = atoi(stringNum))) { //use atoi and check if it failed
		printf("atoi failed\n");
		return -1;
	}
	return result;
}

//Not done - need to decide how to implement it.
int processAndCopy(char* string) {
	char* tmpString = NULL;
	int capacity = PAGE_SIZE;

	if (NULL == (tmpString = malloc(strlen(string )))) {
		printf("Fatal error: memory allocation for tmpString failed.\n");
		return NOT_SUCCESS;
	}
	
}
