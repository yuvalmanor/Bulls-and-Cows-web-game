#ifndef BULLSANDCOWS_H
#define BULLSANDCOWS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hardcodeddata.h"
#include "SocketSendRecvTools.h"
#include "sharedMessagesProcess.h"
#include "WinsockFunc.h"

#define CLIENT_MSG_NUM 5
#define REQUEST_LEN 14
#define CLEAN 1
#define EXIT 0
#define START_AGAIN 2
#define CONTINUE 4

typedef enum { MAIN, FAILURE, DENIED } menuStatus;
typedef enum { MID_GAME, WIN, TIE } gameStatus;

int playGame(char* username, SOCKET c_socket, SOCKADDR_IN clientService, char* ip, int portNumber);
int setup(char* username, SOCKET c_socket, SOCKADDR_IN clientService, char* ip, int portNumber);
int start(SOCKET c_socket);
int GameIsOn(SOCKET c_socket);
int playerChoice();
int checkTRNSCode(int TRNSCode, char* ip, int portNumber, SOCKET c_socket, SOCKADDR_IN clientService);
int menu(int menu, char* ip, int portNumber);
char* chooseNumber();
int opponentQuit(char* message);
void gameResults(Message* message, int status);
int makeConnection(SOCKET c_socket, SOCKADDR_IN clientService, char* ip, int portNumber);
void resourcesManager(SOCKET clientSocket, int WSACleanFlag);
#endif // !BULLSANDCOWS_H
