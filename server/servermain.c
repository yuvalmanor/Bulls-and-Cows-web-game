#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "servermain.h"

int ServerMainFreeResources(SOCKET MainSocket) {
	if (NULL != MainSocket) {
		if (closesocket(MainSocket) == SOCKET_ERROR)
			printf("Failed to close MainSocket in ServerMainFreeResources(). error %ld\n", WSAGetLastError());
	}
	if (WSACleanup() == SOCKET_ERROR)
		printf("Failed to close Winsocket in ServerMainFreeResources(). error %ld\n", WSAGetLastError());

}

serverManager(int portNumber){
	SOCKET MainSocket = INVALID_SOCKET;
	SOCKET threadInputs[MAX_NUM_OF_PLAYERS+1] = { INVALID_SOCKET, INVALID_SOCKET, INVALID_SOCKET };
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;
	int index;
	HANDLE threadHandles[MAX_NUM_OF_PLAYERS+1] = { NULL, NULL, NULL };

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
		ServerMainFreeResources(NULL);
		return -1;
	}

	//<------- Create a sockaddr_in object and set its values ----->
	Address = inet_addr(LOCALHOST); //------>Is this an unsafe function?
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			LOCALHOST);
		ServerMainFreeResources(MainSocket);
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
		//Free resources: close socket, free address struct(?), WASCleanup, return -1
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		if (closesocket(MainSocket) == SOCKET_ERROR)
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
		if (WSACleanup() == SOCKET_ERROR)
			printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
		return -1;
	}
	printf("socket bounded.\n");

	// <-------Listen on the Socket------->
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{		
		//Free resources: close socket, free address struct(?), WASCleanup, return -1
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		if (closesocket(MainSocket) == SOCKET_ERROR)
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
		if (WSACleanup() == SOCKET_ERROR)
			printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
		return -1;
	}
	printf("listening to IP: %s port %d\n", LOCALHOST, portNumber);

	//Start polling for "quit" message - if it will happen, all threads must be closed, resources freed and program will end

	printf("Waiting for a client to connect\n");

	//<------ Wait for clients to connect ------>
	while (1) {
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			TerminateServiceThreads(threadHandles, threadInputs); //We need to add termination of the other thread

		}

		printf("Client Connected.\n");

		index = FindFirstUnusedThreadSlot(threadHandles); //Doesn't seem to work - WFSO returns 0 and doesn't timeout

		if (index == MAX_NUM_OF_PLAYERS) //maximum number of clients are connected
		{
			threadInputs[index] = AcceptSocket; //The serverIsFUll thread is responisble for closing this socket
			threadHandles[index] = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)ServerFullThread,
				&(threadInputs[index]),
				0,
				NULL
			);
		}
		else
		{
			threadInputs[index] = AcceptSocket; //The service thread is responisble for closing this socket
			threadHandles[index] = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)ServiceThread,
				&(threadInputs[index]),
				0,
				NULL
			);
		}
	} //!while(1)

	//How do we ensure that the program will end nicely?
	if (closesocket(MainSocket) == SOCKET_ERROR)
		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
	//free address struct? anything else to free?
	if (WSACleanup() == SOCKET_ERROR)
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
}


void TerminateServiceThreads(HANDLE* threadHandles, SOCKET* threadInputs)
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
				closesocket(threadInputs[index]);
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