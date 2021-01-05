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

int startGame(SOCKET socket, HANDLE h_sharedFile, HANDLE lockEvent, HANDLE syncEvent, int playerOne, int* p_players, char* username, char* opponentName);
char* getResults(char* username, char* opponentName, char* userNum, char* opponentNum, char* userGuess, char* opponentGuess);
int opponentLeftGame(SOCKET socket, int* p_players, HANDLE lockEvent);
void freeMemory(char* userNum, char* opponentNum, char* userGuess, char* opponentGuess);
int winnerCheck(char** resultMsg, char* winnerSecretNum, char* loserSecretNum, char* Guess, char* name);
#endif // !PART2_H
