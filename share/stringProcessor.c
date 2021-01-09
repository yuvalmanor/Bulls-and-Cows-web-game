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

