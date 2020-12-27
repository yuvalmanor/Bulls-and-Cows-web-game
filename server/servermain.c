#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "servermain.h"



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

int FindFirstUnusedThreadSlot(HANDLE* threadHandles)
{
	int index;

	for (index = 0; index < MAX_NUM_OF_PLAYERS; index++)
	{
		if (threadHandles[index] == NULL)
			break;
		else
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(threadHandles[index], 0);

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

serverMain(int portNumber){
	SOCKET MainSocket = INVALID_SOCKET;
	SOCKET threadInputs[MAX_NUM_OF_PLAYERS] = { INVALID_SOCKET, INVALID_SOCKET };
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;
	int index;
	HANDLE threadHandles[MAX_NUM_OF_PLAYERS] = { NULL, NULL };

	// Initialize Winsock.
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		// Tell the user that we could not find a usable WinSock DLL.                                  
		return;
	}/* The WinSock DLL is acceptable. Proceed. */


	// <-----Create a socket----->   
	MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	printf("Socket created.\n");
	if (MainSocket == INVALID_SOCKET)
	{
		//Free resources, WSACleanup and end program with -1
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		if (WSACleanup() == SOCKET_ERROR)
			printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
		return -1;
	}

	// <------Bind the socket----->
	/*
		For a server to accept client connections, it must be bound to a network address within the system.
		The following code demonstrates how to bind a socket that has already been created to an IP address
		and port.
		Client applications use the IP address and port to connect to the host network.
		The sockaddr structure holds information regarding the address family, IP address, and port number.
		sockaddr_in is a subset of sockaddr and is used for IP version 4 applications.
	*/
	//<------- Create a sockaddr_in object and set its values ----->
	// Declare variables
	Address = inet_addr("127.0.0.1"); //------>Is this an unsafe function?
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			"127.0.0.1");
		//Free resources: close socket, WASCleanup, return -1
		if (closesocket(MainSocket) == SOCKET_ERROR)
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
		if (WSACleanup() == SOCKET_ERROR)
			printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
		return -1;
	}

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address;
	service.sin_port = htons(portNumber); //The htons function converts a u_short from host to TCP/IP network byte order 
									   //( which is big-endian ).
	/*
		The three lines following the declaration of sockaddr_in service are used to set up
		the sockaddr structure:
		AF_INET is the Internet address family.
		"127.0.0.1" is the local IP address to which the socket will be bound.
		2345 is the port number to which the socket will be bound.
	*/

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

		index = FindFirstUnusedThreadSlot(threadHandles); //Doesn't seem to work

		if (index == MAX_NUM_OF_PLAYERS) //maximum number of clients are connected
		{
			printf("No slots available for client, dropping the connection.\n");
			closesocket(AcceptSocket); //Closing the socket, dropping the connection.
		}
		else
		{
			threadInputs[index] = AcceptSocket; //The service thread is responisble for closing the socket
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
