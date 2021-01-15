/*
Description – The module that is incharge of creating the Main socket, choosing whether to accept or deny
		the clients, Creating the threads that handle each client and the threads that poll for "exit" or failure
*/

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


/*
Description - The function that manages the server and its threads
Parameters - 
	*int portNumber - the port number to which the socket will be bounded
Returns - SUCCESS or NOT_SUCCESS
	*/
int serverManager(int portNumber);

/*
Description - Initialize a signle thread's parameters
Parameters -
	*SOCKET socket - a socket for the thread to use
	* int* numOfPlayersInGame - the address of an int initialized to 0
	* int* numOfPlayersSyncing - the address of an int initialized to 0
	* SOCKET* p_socket - the adress of the MainSocket
Returns - a ThreadParam instance if successful, NULL if not successful.
	*/
ThreadParam* initThreadParam(SOCKET socket, int* numOfPlayersInGame, int* numOfPlayersSyncing, SOCKET* p_socket);
/*
Description - Free manager resources
Parameters -
	* SOCKET MainSocket - The main Socket
	* HANDLE lockEvent - an event to close or NULL if event was not yet opened
	* HANDLE syncEvent - an event to close or NULL if event was not yet opened
	* HANDLE FailureEvent - an event to close or NULL if event was not yet opened
Returns - SUCCESS or NOT_SUCCESS
	*/
int ServerManagerFreeResources(SOCKET MainSocket, HANDLE lockEvent, HANDLE syncEvent, HANDLE FailureEvent);
/*
Description - Finds the first unused thread slot (out of the first 3 slots)
Parameters -
	* HANDLE* threadHandles - An array of thread handles
	* ThreadParam** threadParams - an array of thread parameters
Returns - The index of the first unused slot
	*/
int FindFirstUnusedThreadSlot(HANDLE* threadHandles, ThreadParam** threadParams);

/*
Description - the function the "Failure thread" will execute. If a failure Event was set - Make the
			main thread stop waiting for accept(), terminate all of the threads and shut the program down
Parameters -
	* ThreadParam* lpParam - the parameters for the thread
Returns - no return value
	*/
void FailureThread(ThreadParam* lpParam);

/*
Description - the function the "exit thread" will execute. If the user wrote "exit" to the console - Make the
			main thread stop waiting for accept(), terminate all of the threads and shut the program down.
			We can assume no client will be connected when exit is being called
Parameters -
	* ThreadParam* lpParam - the parameters for the thread
Returns - no return value
	*/
void exitThread(ThreadParam* lpParam);

/*
Description - close all opened thread Handles and free any allocated thread parameter. 
If Threads are running, Terminate them.
Parameters -
	* HANDLE* threadHandles - arr of thread Handles
	* ThreadParam** threadParams - arr of pointers to Thread parameters
Returns - SUCCESS or NOT_SUCCESS
	*/
int clearThreadsAndParameters(HANDLE* threadHandles, ThreadParam** threadParams);
#endif // !SERVERMAIN_H