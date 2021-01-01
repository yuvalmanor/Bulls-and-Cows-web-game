#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <stdio.h>
#include <string.h>
#include "WinsockFunc.h"
#include "stringProcessor.h"
#include "BullsAndCows.h"
#include "sharedMessagesProcess.h"



int clientManager(char* ip, int portNumber, char* username);
int makeConnection(SOCKET c_socket, SOCKADDR_IN clientService, char* ip, int portNumber);
void resourcesManager(SOCKET clientSocket, int WSACleanFlag);
int checkTRNSCode(int TRNSCode, char* ip, int portNumber, SOCKET c_socket, SOCKADDR_IN clientService);
int setup(char* username, SOCKET c_socket, SOCKADDR_IN clientService, char* ip, int portNumber);
int menu(int menu, char* ip, int portNumber);
#endif // !CLIENTMANAGER_H