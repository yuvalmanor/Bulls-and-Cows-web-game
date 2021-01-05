#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "serverManager.h"

int serverManager(int portNumber) {
	SOCKET MainSocket = INVALID_SOCKET;
	ThreadParam* threadParams[MAX_NUM_OF_PLAYERS+2] = { NULL, NULL, NULL, NULL};
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;
	int index;
	int players = 0;
	HANDLE threadHandles[MAX_NUM_OF_PLAYERS+2] = { NULL, NULL, NULL, NULL };
	HANDLE lockEvent = NULL, syncEvent = NULL, FailureEvent = NULL;
	//<--------Initialize Winsock------->
	if (-1 == InitializeWinsock()) { 
		return -1;
	}

	// <-----Create a socket----->   
	MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	printf("Socket created.\n");
	if (MainSocket == INVALID_SOCKET)
	{
		//Free resources, WSACleanup and end program with -1
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		ServerMainFreeResources(NULL, threadParams);
		return -1;
	}

	//<------- Create a sockaddr_in object and set its values ----->

	Address = inet_addr(LOCALHOST); 
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			LOCALHOST);
		ServerMainFreeResources(MainSocket, threadParams);
		return -1;
	}
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address;
	service.sin_port = htons(portNumber);

	//<------- Bind ------->
	// Call the bind function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
	bindRes = bind(MainSocket, (SOCKADDR*)&service, sizeof(service));
	if (bindRes == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		ServerMainFreeResources(MainSocket, threadParams);
		return -1;
	}
	printf("socket bounded.\n");

	// <-------Listen on the Socket------->
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{		
		//Free resources: close socket, free address struct(?), WASCleanup, return -1
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		ServerMainFreeResources(MainSocket, threadParams);
		return -1;
	}
	printf("listening to IP: %s port %d\n", LOCALHOST, portNumber);

	//Create syncronization mechanisms
	if (FAILING == getEvents(&lockEvent, &syncEvent, &FailureEvent)) {
		ServerMainFreeResources(MainSocket, threadParams);
		return -1;
	}
	//start polling for exit with a thread - it asks the threads to finish politly
	//make a thread that awaits failureEvent to be signaled - it kills the other threads
	printf("Waiting for a client to connect\n");

	//<------ Wait for clients to connect ------>
	while (1) {
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET) //How should we handle this? close the program or continue as usual?
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			//TerminateServiceThreads(threadHandles, threadParams); //Is this necessary?
		}
		printf("Client Connected.\n");

		//Get the index of the first unused slot
		index = FindFirstUnusedThreadSlot(threadHandles); //Doesn't seem to work - WFSO returns 0 and doesn't timeout

		threadParams[index] = initThreadParam(AcceptSocket, index, &players); //initialize parameters for this thread
		if (threadParams[index] == NULL) { //If this fails, close the socket and wait for another client
			closesocket(AcceptSocket);
			continue;
		}
		if (index == MAX_NUM_OF_PLAYERS) //maximum number of clients are connected
		{
			threadParams[index]->socket = AcceptSocket; //The serverIsFUll thread is responisble for closing this socket
			threadHandles[index] = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)ServerFullThread, //This type of thread lets the client know there's no room
														//for another thread and then disconnects from it.
				threadParams[index],
				0,
				NULL
			);
		}
		else //If there is room for the client in the game
		{
			threadParams[index]->socket = AcceptSocket; //The service thread is responisble for closing this socket
			threadHandles[index] = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)ServiceThread,
				threadParams[index],
				0,
				NULL
			);
		}
	} //!while(1)
	printf("We shouldn't be here\n");
	//How do we ensure that the program will end nicely?
	ServerMainFreeResources(MainSocket, threadParams);
}

ThreadParam* initThreadParam(SOCKET socket, int index, int* players) {
	ThreadParam* p_threadparams = NULL;
	if (NULL == (p_threadparams = (ThreadParam*)malloc(sizeof(ThreadParam)))) {
		printf("Fatal error: memory allocation failed (ThreadParam).\n");
		return NULL;
	}
	p_threadparams->socket = socket;
	p_threadparams->p_players = players;
	return p_threadparams;
}

int ServerMainFreeResources(SOCKET MainSocket, ThreadParam** threadParams) { //Add all events and close handles
	if (NULL != MainSocket) {
		if (closesocket(MainSocket) == SOCKET_ERROR)
			printf("Failed to close MainSocket in ServerMainFreeResources(). error %ld\n", WSAGetLastError());
	}
	if (WSACleanup() == SOCKET_ERROR) {
		printf("Failed to close Winsocket in ServerMainFreeResources(). error %ld\n", WSAGetLastError());
	}
	for (int i = 0; i < 4; i++) {
		if (threadParams[i] != NULL) {
			if (closesocket(threadParams[i]->socket) == SOCKET_ERROR) {
				printf("Can't close socket, it might alraedy be closed\n");
			}
			free(threadParams[i]);
		}
	}
	return 1;
}

void TerminateServiceThreads(HANDLE* threadHandles, SOCKET* threadParams) //Needed?
{
	int index;

	for (index = 0; index < MAX_NUM_OF_PLAYERS; index++)
	{
		if (threadHandles[index] != NULL)
		{ 
			//<------change this to terminate thread!! ----->
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(threadHandles[index], INFINITE); //--->can't be infinite
			if (Res == WAIT_OBJECT_0)
			{
				closesocket(threadParams[index]);
				CloseHandle(threadHandles[index]);
				threadHandles[index] = NULL;
				break;
			}
			else
			{
				printf("CleanupServiceThreads: Waiting for thread failed\n");
				return;
			}
		}
	}
}

int FindFirstUnusedThreadSlot(HANDLE* threadHandles){

	int index;

	for (index = 0; index < MAX_NUM_OF_PLAYERS; index++)
	{
		if (threadHandles[index] == NULL)
			break;
		else
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(threadHandles[index], 1);

			if (Res == WAIT_OBJECT_0) // this thread finished running
			{
				CloseHandle(threadHandles[index]);
				threadHandles[index] = NULL;
				break;
			}
		}
	}

	return index;
}