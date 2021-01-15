/*
Description – A module that enables communication and syncronization between different threads
*/

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef THREADSCOMMUNICATION_H
#define THREADSCOMMUNICATION_H
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sharedMessagesProcess.h"
#include "hardcodeddata.h"

static LPCWSTR lockEvent_name = L"lockEvent";
static LPCWSTR syncEvent_name = L"syncEvent";
static LPCWSTR failureEvent_name = L"Failure";
static LPCSTR sharedFile_name = "GameSession.txt";

/*
Description: A wrapper function for CreateEvent - Create 3 events if don't exist, get events handles if already exist
Parameters:
	1. HANDLE* lockEvent - a pointer to a handle where lockEvent handle will be
	2. HANDLE* syncEvent - a pointer to a handle where syncEvent handle will be
	3. HANDLE* FailureEvent - a pointer to a handle where FailureEvent handle will be
Returns: SUCCESS, NOT_SUCCESS
*/
int getEvents(HANDLE* lockEvent, HANDLE* syncEvent, HANDLE* FailureEvent);

/*
Description: If communication file doensn't exist - create it and set playerOne to 1
			Otherwise - open the existing file and set playerOne to 0 (this is player 2)
Parameters:
	1. int* p_playerOne - a poniter to an int that indicates if this is player 1 or not
Returns: A file handle if successful, o.w NULL
*/
HANDLE openOrCreateFile(int* p_playerOne);
/*
Description: write data to the communication file
Parameters:
	1. HANDLE h_file - a handle to the open communication file
	2. int offset - the offset where the data should be written to
	3. char* data - the data to write to the file
	4. int playerOne - 1 if this is player 1, 0 if this is player 2
	5. int writeUsername - 1 if the data is a username, otherwise - 0.
Returns: SUCCESS or NOT_SUCCESS
*/
int writeToFile(HANDLE h_file, int offset, char* data, int playerOne, int writeUsername);

/*
Description: read data from the communication file
Parameters:
	1. HANDLE h_file - a handle to the open communication file
	2. int offset - the offset where the data should be read from
	3. char** data - a pointer to the buffer where the data will be read to
	4. int playerOne - 1 if this is player 1, 0 if this is player 2
	5. int readUsername - 1 if reading a username, otherwise - 0.
Returns: SUCCESS or NOT_SUCCESS
*/
int readFromFile(HANDLE h_sharedFile, int offset, char** data, int playerOne, int readUsername);

/*
Description: A function that syncs the two clients. Will only return if other client also called this function
			or if the other client disconnected.
Parameters:
	1. SOCKET socket - the socket to close or NULL if no need to close the socket
	2. int* p_numOfPlayersSyncing - a pointer to an int indicating how many players user this function in the current sync
	3. int* p_numOfPlayersInGame -a pointer to an int indicating how many players are in the game
	4. HANDLE lockEvent - an event to protect shared resources - Auto reset and signaled
	5. HANDLE syncEvent - an event to sync the two threads - Maual reset and non-signaled
Returns: MAIN_MENU if other client disconnected, NOT_SUCCESS if an error occured or GAME_STILL_ON
*/
int SyncTwoThreads(SOCKET socket, int* p_numOfPlayersSyncing, int* p_numOfPlayersInGame, HANDLE lockEvent, HANDLE syncEvent);

/*
Description: opponentLeftGame checks if the opponent left the game, if he is, send SERVER_NO_OPPONENTS
		message to the client.
Arguments:
	1. SOCKET socket
	2. int* p_players - Pointer to an integer that counts the number of clients are connected to the server.
	3. HANDLE lockEvent - an event to protect shared resources - Auto reset and signaled
returns: NOT_SUCCESS or MAIN_MENU
*/
int opponentLeftGame(SOCKET socket, int* p_players, HANDLE lockEvent);

#endif //!THREADSCOMMUNICATION_H