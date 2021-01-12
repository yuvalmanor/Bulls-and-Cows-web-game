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
#include "hardcodeddata.h"
#include "WinsockFunc.h"

typedef enum { MAIN, FAILURE, DENIED } menuStatus;
typedef enum { MID_GAME, WIN, TIE } gameStatus;

typedef struct ThreadParam {
	SOCKET socket;
	int* p_players;
	int* p_PlayersCount;
	HANDLE* threadHandles;

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
int SyncTwoThreads(int* p_PlayersCount, HANDLE lockEvent, HANDLE syncEvent, int waitTime);
int startGame(SOCKET socket, HANDLE h_sharedFile, HANDLE lockEvent, HANDLE syncEvent, int playerOne, int* p_players, char* username, char* opponentName, int* p_playersCount);
int getResults(char** resultMsg, char* username, char* opponentName, char* userNum, char* opponentNum, char* userGuess, char* opponentGuess);
int opponentLeftGame(SOCKET socket, int* p_players, HANDLE lockEvent);
void freeMemory(char* userNum, char* opponentNum, char* userGuess, char* opponentGuess);
char* winMsg(char* opponentNum, char* winnerName);
#endif // !SERVICETHREAD_H