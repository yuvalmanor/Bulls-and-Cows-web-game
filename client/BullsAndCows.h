#ifndef BULLSANDCOWS_H
#define BULLSANDCOWS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hardcodeddata.h"
#include "SocketSendRecvTools.h"
#include "sharedMessagesProcess.h"

#define CLIENT_MSG_NUM 5
#define REQUEST_LEN 14
#define CLEAN 1
#define EXIT 0
#define START_AGAIN 2

typedef enum { MAIN, FAILURE, DENIED } ;

int playGame(char* username, SOCKET c_socket, SOCKADDR_IN clientService, char* ip, int portNumber);
int playerChoice();
char* prepareMsg(const char* msgType, char* str);
int setup(char* username, SOCKET c_socket, SOCKADDR_IN clientService, char* ip, int portNumber);
int checkTRNSCode(int TRNSCode, char* ip, int portNumber, SOCKET c_socket, SOCKADDR_IN clientService);
int menu(int menu, char* ip, int portNumber);


#endif // !BULLSANDCOWS_H
