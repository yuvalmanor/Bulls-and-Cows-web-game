#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "serverManager.h"

int serverManager(int portNumber) {
	SOCKET MainSocket = INVALID_SOCKET;
	//TODO - who will free the parameters?
	ThreadParam* threadParams[MAX_NUM_OF_PLAYERS+ NUM_OF_OVERHEAD_THREADS] = { NULL, NULL, NULL, NULL, NULL};
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes, ListenRes, index, numOfPlayersInGame = 0, numOfPlayersSyncing = 0;;
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
		ServerMainFreeResources(NULL, NULL, NULL, NULL); 
		return NOT_SUCCESS;
	}

	//<------- Create a sockaddr_in object and set its values ----->
	Address = inet_addr(LOCALHOST); 
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			LOCALHOST);
		ServerMainFreeResources(MainSocket, NULL, NULL, NULL);
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
		ServerMainFreeResources(MainSocket, NULL, NULL, NULL);
		return NOT_SUCCESS;
	}
	printf("socket bounded.\n");

	// <-------Listen on the Socket------->
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{		
		//Free resources: close socket, free address struct(?), WASCleanup, return -1
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		ServerMainFreeResources(MainSocket, NULL, NULL, NULL); 
		return NOT_SUCCESS;
	}
	printf("listening to IP: %s port %d\n", LOCALHOST, portNumber);

	//Create syncronization mechanisms
	if (NOT_SUCCESS == getEvents(&lockEvent, &syncEvent, &FailureEvent)) {
		ServerMainFreeResources(MainSocket, NULL, NULL, NULL);
		return NOT_SUCCESS;
	}
	//Create Failure thread and Exit Thread
	if (NOT_SUCCESS == createFailureAndExitThreads(threadParams, threadHandles, &MainSocket)) {
		ServerMainFreeResources(MainSocket, lockEvent, syncEvent, FailureEvent);
		return NOT_SUCCESS;
	}
	printf("Waiting for a client to connect\n");

	//<------ Wait for clients to connect ------>
	while (1) {
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET)
		{	
			if (WSAEINTR == GetLastError()) {//The MainSocket was closed by FailureThread or exitThread
				break; //Free resources and shut down program
			}
			else {
				printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
				continue;
			}
		}
		printf("Client Connected.\n");

		//Get the index of the first unused slot
		index = FindFirstUnusedThreadSlot(threadHandles, threadParams); 

		threadParams[index] = initThreadParam(AcceptSocket, index, &numOfPlayersInGame, &numOfPlayersSyncing, NULL); //initialize parameters for this thread
		if (threadParams[index] == NULL) { //If this fails, close the socket and wait for another client
			closesocket(AcceptSocket);
			break;
		}
		if (index == MAX_NUM_OF_PLAYERS) //maximum number of clients are connected
		{
			threadHandles[index] = CreateThread( //Create a thread that will let client know server is full and disconnect from it.
				NULL, 0, (LPTHREAD_START_ROUTINE)ServerFullThread, threadParams[index], 0, NULL);
		}
		else //If there is room for the client in the game
		{
			threadHandles[index] = CreateThread( //Create a serviceThread for the client to enter the game
				NULL, 0, (LPTHREAD_START_ROUTINE)ServiceThread, threadParams[index], 0, NULL);
		}
	} //!while(1)

	clearThreadsAndParameters(threadHandles, threadParams);
	ServerMainFreeResources(NULL, lockEvent, syncEvent, FailureEvent);
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
	p_threadparams->p_numOfPlayersInGame = players;
	p_threadparams->p_numOfPlayersSyncing = PlayersCount;
	p_threadparams->p_socket = p_socket;
	return p_threadparams;
}

//TODO make sure this is doing the right things
int ServerMainFreeResources(SOCKET MainSocket, HANDLE lockEvent, HANDLE syncEvent, HANDLE FailureEvent) { //Add all events and close handles
	if (NULL != MainSocket) {
		if (closesocket(MainSocket) == SOCKET_ERROR)
			printf("Failed to close MainSocket in ServerMainFreeResources(). error %ld\n", WSAGetLastError());
	}
	if (lockEvent != NULL) {
		CloseHandle(lockEvent);
	}
	if (syncEvent != NULL) {
		CloseHandle(syncEvent);
	}
	if (FailureEvent != NULL) {
		CloseHandle(FailureEvent);
	}
	if (WSACleanup() == SOCKET_ERROR) {
		printf("Failed to close Winsocket in ServerMainFreeResources(). error %ld\n", WSAGetLastError());
		return NOT_SUCCESS;
	}
	return SUCCESS;
}

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
	if (threadParams[EXIT_THREAD_INDEX] == NULL) {
		free(threadParams[FAILURE_THREAD_INDEX]);
		threadParams[FAILURE_THREAD_INDEX] = NULL;
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
	SOCKET* p_socket = p_param->p_socket;
	HANDLE lockEvent = NULL, syncEvent = NULL, FailureEvent = NULL;
	DWORD waitcode;
	//<-----Open FailureEvent handle------>
	if (1 != getEvents(&lockEvent, &syncEvent, &FailureEvent)) { 
		printf("error. can't getEvents (FailureThread)\n");
		if (closesocket(*p_socket) == SOCKET_ERROR)
			printf("Failed to close MainSocket in FailureThread. error %ld\n", WSAGetLastError());
		return;
	}
	//<-------Wait for failure----->
	waitcode = WaitForSingleObject(FailureEvent, INFINITE); //Wait for failure event to be set
	if (waitcode != WAIT_OBJECT_0) { //If FailureEvent was not set, don't close the main socket
		printf("FailureThread finishing without failureEvent being set\n");
		return;
	}
	//If failureEvent was set, close the main socket
	if (closesocket(*p_socket) == SOCKET_ERROR)
		printf("Failed to close MainSocket in FailureThread. error %ld\n", WSAGetLastError());
}

void exitThread(ThreadParam* lpParam) {
	ThreadParam* p_param;
	if (NULL == lpParam) {
		printf("exit thread can't work with NULL as parameters\n");
		return NOT_SUCCESS;
	}
	p_param = (ThreadParam*)lpParam;
	SOCKET* p_socket = p_param->p_socket;
	char str[5];
	while (1) {
		if (scanf_s("%s", str,  5)){
			str[4] = '\0';
			if (!strcmp(str, "exit")) {
				if (closesocket(*p_socket) == SOCKET_ERROR) //Close the main socket
					printf("Failed to close MainSocket in exitThread. error %ld\n", WSAGetLastError());
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
			if (waitcode != WAIT_OBJECT_0) { //If thread is active, free its parameters and terminate it
				if (i <= MAX_NUM_OF_PLAYERS) {
					if (closesocket(threadParams[i]->socket) == SOCKET_ERROR)
						printf("Failed to close MainSocket in ServerMainFreeResources(). error %ld\n", WSAGetLastError());
				}
				TerminateThread(threadHandles[i] != NULL, -1);
			}
			CloseHandle(threadHandles[i]);
		}
		if (threadParams[i] != NULL) { //In case parameters were created but the thread was not created
			free(threadParams[i]);
			threadParams[i] = NULL;
		}
	}
	
	return SUCCESS;
}