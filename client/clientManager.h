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

int clientManager(char* ip, int portNumber, char* username);
void resourcesManager(SOCKET clientSocket);


#endif // !CLIENTMANAGER_H