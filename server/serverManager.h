#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef SERVERMAIN_H
#define SERVERMAIN_H
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include <stdio.h>
#include <string.h>
#include "serverFullThread.h"
#include "WinsockFunc.h"

//#define EVENT_NAME "Event_1"

int serverManager(int portNumber);

ThreadParam* initThreadParam(SOCKET socket, int index, int* players, int* PlayersCount);

int ServerMainFreeResources(SOCKET MainSocket,
	ThreadParam** threadParams,
	HANDLE lockEvent, HANDLE syncEvent, HANDLE FailureEvent);

void TerminateServiceThreads(HANDLE* threadHandles, SOCKET* threadParams);

int FindFirstUnusedThreadSlot(HANDLE* threadHandles);


#endif // !SERVERMAIN_H