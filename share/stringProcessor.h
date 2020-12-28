#ifndef STRINGPROCESSOR_H
#define STRINGPROCESSOR_H
#include <string.h>
#include <stdio.h>
#include "hardcodeddata.h"


int strToInt(char* stringNum);
int processAndCopy(char** dest, char* source, char* delim);
#endif // !STRINGPROCESSOR_H

