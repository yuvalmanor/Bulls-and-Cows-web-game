#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef SERVICETHREAD_H
#define SERVICETHREAD_H
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "SocketSendRecvTools.h"
#include "sharedMessagesProcess.h"

typedef struct ThreadParam {
	SOCKET socket;
	int offset; //Not necessary
	int* p_players;

}ThreadParam;

static numOfPlayers = 0;

DWORD ServiceThread(ThreadParam* lpParam);

#endif // !SERVICETHREAD_H