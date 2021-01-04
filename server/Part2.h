#ifndef PART2_H
#define PART2_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hardcodeddata.h"
#include "SocketSendRecvTools.h"
#include "sharedMessagesProcess.h"
#include "servicethread.h"

#define EXIT 0
#define START_AGAIN 2


typedef enum { MAIN, FAILURE, DENIED } menuStatus;
typedef enum { MID_GAME, WIN, TIE } gameStatus;

int startGame(SOCKET socket, HANDLE h_sharedFile);

#endif // !PART2_H
