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


int processAndCopy(char** dest, char* source, char* delim) {
	char* token = NULL, * tmp_restOfSource = NULL, * tmpString = NULL;
	int i=0;

	token = strtok_s(source, delim, &tmp_restOfSource);
	if (NULL == token) {
		printf("strtok_s failed(processAndCopy)\n");
		//free(tmpString);
		return NOT_SUCCESS;
	}
	
	if (NULL == (tmpString = malloc(strlen(token) + 1))) {
		printf("Fatal error: memory allocation for tmpString failed.\n");
		return NOT_SUCCESS;
	}
	strcpy_s(tmpString, strlen(token) + 1, token);
	*dest = tmpString;
	return SUCCESS;

}
