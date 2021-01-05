#ifndef PART2_H
#define PART2_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hardcodeddata.h"
#include "SocketSendRecvTools.h"
#include "sharedMessagesProcess.h"
#include "servicethread.h"
#include "hardcodeddata.h"




typedef enum { MAIN, FAILURE, DENIED } menuStatus;
typedef enum { MID_GAME, WIN, TIE } gameStatus;

int startGame(SOCKET socket, HANDLE h_sharedFile);
char* getResults(char* username, char* opponentName, char* userNum, char* opponentGuess, char* opponentNum);

#endif // !PART2_H
