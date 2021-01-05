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
	int* p_players;
	int* p_PlayersCount;

}ThreadParam;

static numOfPlayers = 0;

DWORD ServiceThread(void* lpParam);
int Main_menu(SOCKET socket, HANDLE lockEvent, HANDLE syncEvent, int* p_players, int* playerOne, char* username, char** otherUsername); int getUserNameAndApproveClient(SOCKET socket, char** username);
int getUserNameAndApproveClient(SOCKET socket, char** username);
HANDLE openOrCreateFile(int* playerOne);
int writeToFile(HANDLE h_file, int offset, char* data, int playerOne, int writeUsername);
int readFromFile(HANDLE h_sharedFile, int offset, char** data, int playerOne, int readUsername);
int getEvents(HANDLE* lockEvent, HANDLE* syncEvent, HANDLE* FailureEvent);
void leaveGame(SOCKET socket, HANDLE lockEvent, int* p_players, HANDLE h_sharedFile, Message* message);
int SyncTwoThreads(int* p_PlayersCount, HANDLE lockEvent, HANDLE syncEvent);

#endif // !SERVICETHREAD_H