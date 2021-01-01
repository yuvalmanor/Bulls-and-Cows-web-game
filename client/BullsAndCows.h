#ifndef BULLSANDCOWS_H
#define BULLSANDCOWS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hardcodeddata.h"
#include "SocketSendRecvTools.h"

#define CLIENT_MSG_NUM 5
#define REQUEST_LEN 14

typedef enum { MAIN, FAILURE, DENIED } ;

int playGame(char* username, SOCKET c_socket);
int playerChoice();
char* prepareMsg(const char* msgType, char* str);


#endif // !BULLSANDCOWS_H
