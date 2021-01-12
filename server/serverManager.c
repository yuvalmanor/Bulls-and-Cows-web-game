#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "serverManager.h"

int serverManager(int portNumber) {
	SOCKET MainSocket = INVALID_SOCKET;
	//TODO - who will free the parameters?
	ThreadParam* threadParams[MAX_NUM_OF_PLAYERS+ NUM_OF_OVERHEAD_THREADS] = { NULL, NULL, NULL, NULL, NULL};
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;
	int index;
	int players = 0, PlayersCount = 0;
	HANDLE threadHandles[MAX_NUM_OF_PLAYERS+ NUM_OF_OVERHEAD_THREADS] = { NULL, NULL, NULL, NULL, NULL};
	HANDLE lockEvent = NULL, syncEvent = NULL, FailureEvent = NULL;
	//<--------Initialize Winsock------->
	if (-1 == InitializeWinsock()) { 
		return NOT_SUCCESS;
	}
	// <-----Create a socket----->   
	MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	printf("Socket created.\n");
	if (MainSocket == INVALID_SOCKET)
	{
		//Free resources, WSACleanup and end program with -1
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		ServerMainFreeResources(NULL, threadParams, NULL, NULL, NULL);
		return NOT_SUCCESS;
	}

	//<------- Create a sockaddr_in object and set its values ----->
	Address = inet_addr(LOCALHOST); 
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			LOCALHOST);
		ServerMainFreeResources(MainSocket, threadParams, NULL, NULL, NULL);
		return NOT_SUCCESS;
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
		ServerMainFreeResources(MainSocket, threadParams, NULL, NULL, NULL);
		return NOT_SUCCESS;
	}
	printf("socket bounded.\n");

	// <-------Listen on the Socket------->
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{		
		//Free resources: close socket, free address struct(?), WASCleanup, return -1
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		ServerMainFreeResources(MainSocket, threadParams, NULL, NULL, NULL);
		return NOT_SUCCESS;
	}
	printf("listening to IP: %s port %d\n", LOCALHOST, portNumber);

	//Create syncronization mechanisms
	if (NOT_SUCCESS == getEvents(&lockEvent, &syncEvent, &FailureEvent)) {
		ServerMainFreeResources(MainSocket, threadParams, NULL, NULL, NULL);
		return NOT_SUCCESS;
	}
	//Create Failure thread and Exit Thread
	if (NOT_SUCCESS == createFailureAndExitThreads(threadParams, threadHandles, &MainSocket)) {
		ServerMainFreeResources(MainSocket, threadParams, lockEvent, syncEvent, FailureEvent);
		return NOT_SUCCESS;
	}
	printf("Waiting for a client to connect\n");

	//<------ Wait for clients to connect ------>
	while (1) {
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		printf("manager done waiting\n");
		if (AcceptSocket == INVALID_SOCKET) //How should we handle this? close the program or continue as usual?
		{

			if (WSAENOTSOCK == GetLastError()) {
				break;
			}
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			continue;
		}
		printf("Client Connected.\n");

		//Get the index of the first unused slot
		index = FindFirstUnusedThreadSlot(threadHandles, threadParams); //Doesn't seem to work - WFSO returns 0 and doesn't timeout

		threadParams[index] = initThreadParam(AcceptSocket, index, &players, &PlayersCount, NULL); //initialize parameters for this thread
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
	clearThreadsAndParameters(threadHandles, threadParams);
	ServerMainFreeResources(MainSocket, threadParams, lockEvent, syncEvent, FailureEvent);
	printf("ServerManager is quitting\n");
	return 0;

}

ThreadParam* initThreadParam(SOCKET socket, int index, int* players, int* PlayersCount, SOCKET* p_socket) {
	ThreadParam* p_threadparams = NULL;
	if (NULL == (p_threadparams = (ThreadParam*)malloc(sizeof(ThreadParam)))) {
		printf("Fatal error: memory allocation failed (ThreadParam).\n");
		return NULL;
	}
	p_threadparams->socket = socket;
	p_threadparams->p_players = players;
	p_threadparams->p_PlayersCount = PlayersCount;
	p_threadparams->p_socket = p_socket;
	return p_threadparams;
}

//TODO make sure this is doing the right things
int ServerMainFreeResources(SOCKET MainSocket, ThreadParam** threadParams, HANDLE lockEvent, HANDLE syncEvent, HANDLE FailureEvent) { //Add all events and close handles
	if (NULL != MainSocket) {
		if (closesocket(MainSocket) == SOCKET_ERROR)
			printf("Failed to close MainSocket in ServerMainFreeResources(). error %ld\n", WSAGetLastError());
	}
	if (WSACleanup() == SOCKET_ERROR) {
		printf("Failed to close Winsocket in ServerMainFreeResources(). error %ld\n", WSAGetLastError());
	}
	for (int i = 0; i < 5; i++) {
		if (threadParams[i] != NULL) {
			if (closesocket(threadParams[i]->socket) == SOCKET_ERROR) {
				printf("Can't close socket, it might alraedy be closed\n");
			}
			free(threadParams[i]);
		}
	}
	if (lockEvent != NULL && syncEvent != NULL && FailureEvent != NULL) {
		CloseHandle(lockEvent);
		CloseHandle(syncEvent);
		CloseHandle(FailureEvent);
	}
	return 1;
}
//
//void TerminateServiceThreads(HANDLE* threadHandles, SOCKET* threadParams) //Needed?
//{
//	int index;
//
//	for (index = 0; index < MAX_NUM_OF_PLAYERS; index++)
//	{
//		if (threadHandles[index] != NULL)
//		{ 
//			//<------change this to terminate thread!! ----->
//			// poll to check if thread finished running:
//			DWORD Res = WaitForSingleObject(threadHandles[index], INFINITE); //--->can't be infinite
//			if (Res == WAIT_OBJECT_0)
//			{
//				closesocket(threadParams[index]);
//				CloseHandle(threadHandles[index]);
//				threadHandles[index] = NULL;
//				break;
//			}
//			else
//			{
//				printf("CleanupServiceThreads: Waiting for thread failed\n");
//				return;
//			}
//		}
//	}
//}

int FindFirstUnusedThreadSlot(HANDLE* threadHandles, ThreadParam** threadParams){
	DWORD waitcode;
	int index;

	for (index = 0; index < MAX_NUM_OF_PLAYERS; index++)
	{
		if (threadHandles[index] == NULL)
			break;
		else
		{
			// poll to check if thread finished running:
			waitcode = WaitForSingleObject(threadHandles[index], 1);

			if (waitcode == WAIT_OBJECT_0) // this thread finished running
			{
				CloseHandle(threadHandles[index]);
				free(threadParams[index]);
				threadParams[index] = NULL;
				threadHandles[index] = NULL;
				break;
			}
		}
	}

	return index;
}

int createFailureAndExitThreads(ThreadParam** threadParams, HANDLE* threadHandles, SOCKET* p_socket) {
	threadParams[FAILURE_THREAD_INDEX] = initThreadParam(NULL, NULL, NULL, NULL, p_socket);
	if (threadParams[FAILURE_THREAD_INDEX] == NULL) {
		return NOT_SUCCESS;
	}
	threadParams[EXIT_THREAD_INDEX] = initThreadParam(NULL, NULL, NULL, NULL, p_socket);
	if (threadParams[FAILURE_THREAD_INDEX] == NULL) {
		return NOT_SUCCESS;
	}
	threadHandles[FAILURE_THREAD_INDEX] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FailureThread, threadParams[FAILURE_THREAD_INDEX], 0, NULL);
	threadHandles[EXIT_THREAD_INDEX] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)exitThread, threadParams[EXIT_THREAD_INDEX], 0, NULL);
	return SUCCESS;
}


void FailureThread(ThreadParam* lpParam) {
	ThreadParam* p_param;
	if (NULL == lpParam) {
		printf("Service thread can't work with NULL as parameters\n");
		return NOT_SUCCESS; 
	}
	p_param = (ThreadParam*)lpParam;
	//SOCKET* p_socket = p_param->p_socket; TODO
	HANDLE lockEvent = NULL, syncEvent = NULL, FailureEvent = NULL;
	DWORD waitcode;
	if (1 != getEvents(&lockEvent, &syncEvent, &FailureEvent)) {
		printf("error. can't getEvents (FailureThread)\n");
		//TerminateAllThread(threadHandles); //TODO change this to close MainSocket
	}
	waitcode = WaitForSingleObject(FailureEvent, INFINITE);
	if (waitcode != WAIT_OBJECT_0) {
		printf("An error occured when waiting for FailureThread\n");
	}
		printf("FailureThread: terminating all threads and shutting program down\n");
		//TerminateAllThread(threadHandles); //TODO change this to close MainSocket
}

void TerminateAllThread(HANDLE* threadHandles) {
	for (int i = 0; i < MAX_NUM_OF_PLAYERS + 1; i++) {
		if (!TerminateThread(threadHandles[i], NOT_SUCCESS)) {
			printf("Thread terminated\n");
		}
		else
			printf("Thread not terminated\n");
	}
	printf("Telling parent ServerManager to stop waiting for accept()");
	//make parent thread not wait for accept()
}

void exitThread(ThreadParam* lpParam) {
	ThreadParam* p_param;
	int retval;
	u_long iMode = 1;
	if (NULL == lpParam) {
		printf("exit thread can't work with NULL as parameters\n");
		return NOT_SUCCESS;
	}
	p_param = (ThreadParam*)lpParam;
	SOCKET* p_socket = p_param->p_socket;
	char str[5];
	while (1) {
		if (scanf_s("%s", str,  5)){
			if (!strcmp(str, "exit") || !strcmp(str, "exit\n")) {
				printf("Telling parent ServerManager to stop waiting for accept()\n");
				closesocket(*p_socket);
				break;
			}
		}
	}
}

int clearThreadsAndParameters(HANDLE* threadHandles, ThreadParam** threadParams) {
	DWORD waitcode;
	for (int i = 0; i < MAX_NUM_OF_PLAYERS+ NUM_OF_OVERHEAD_THREADS; i++) {
		if (threadHandles[i] != NULL) {
			waitcode = WaitForSingleObject(threadHandles[i], 1);
			if (waitcode != WAIT_OBJECT_0) {
				if (i <= MAX_NUM_OF_PLAYERS) {
					closesocket(threadParams[i]->socket); //TODO - should we check threadparams[i] != NULL?
				}
				TerminateThread(threadHandles[i] != NULL, -1);
			}
			CloseHandle(threadHandles[i]);
		}
		if (threadParams[i] != NULL) {
			free(threadParams[i]);
			threadParams[i] = NULL;
		}
	}
	
	return SUCCESS;
}