#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "main.h"

static DWORD ServiceThread(SOCKET* t_socket) {
	
	printf("Im the server thread.\n");
	//communicate with client.
	closesocket(*t_socket);
	return 0;


}
int main() {

	SOCKET MainSocket = INVALID_SOCKET, inputThread=INVALID_SOCKET;
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;
	HANDLE threadHandle = NULL;

	// Initialize Winsock.
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		// Tell the user that we could not find a usable WinSock DLL.                                  
		return;
	}


	/* The WinSock DLL is acceptable. Proceed. */
 // Create a socket.    
	MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	printf("Socket created.\n");
	if (MainSocket == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		if (WSACleanup() == SOCKET_ERROR)
			printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
		return 1;
	}

	// Bind the socket.
	/*
		For a server to accept client connections, it must be bound to a network address within the system.
		The following code demonstrates how to bind a socket that has already been created to an IP address
		and port.
		Client applications use the IP address and port to connect to the host network.
		The sockaddr structure holds information regarding the address family, IP address, and port number.
		sockaddr_in is a subset of sockaddr and is used for IP version 4 applications.
   */
   // Create a sockaddr_in object and set its values.
   // Declare variables
	Address = inet_addr("127.0.0.1");
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			"127.0.0.1");
		if (closesocket(MainSocket) == SOCKET_ERROR)
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
		return 1;
	}

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address;
	service.sin_port = htons(2345); //The htons function converts a u_short from host to TCP/IP network byte order 
									   //( which is big-endian ).
	/*
		The three lines following the declaration of sockaddr_in service are used to set up
		the sockaddr structure:
		AF_INET is the Internet address family.
		"127.0.0.1" is the local IP address to which the socket will be bound.
		2345 is the port number to which the socket will be bound.
	*/
	// Call the bind function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
	bindRes = bind(MainSocket, (SOCKADDR*)&service, sizeof(service));
	if (bindRes == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		if (closesocket(MainSocket) == SOCKET_ERROR)
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
		return 1;
	}
	printf("socket bounded.\n");
	// Listen on the Socket.
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		if (closesocket(MainSocket) == SOCKET_ERROR)
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
		return 1;
	}
	printf("listening to....\n");
	printf("Waiting for a client to connect...\n");
	SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
	if (AcceptSocket == INVALID_SOCKET)
	{
		printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
		if (threadHandle != NULL) {
			DWORD Res = WaitForSingleObject(threadHandle, 0);
			if (Res == WAIT_OBJECT_0)
			{
				closesocket(threadHandle);
				CloseHandle(threadHandle);
				threadHandle = NULL;
				return 1;
				
			}
			else
			{
				printf("Waiting for thread failed. Ending program\n");
				return 1;
			}
		}
	}

	printf("Client Connected.\n");
	inputThread = AcceptSocket;
	threadHandle = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)ServiceThread,
		&inputThread,
		0,
		NULL
	);
	if (AcceptSocket == INVALID_SOCKET)
	{
		printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
		if (threadHandle != NULL) {
			DWORD Res = WaitForSingleObject(threadHandle, INFINITE);
			if (Res == WAIT_OBJECT_0)
			{
				closesocket(threadHandle);
				CloseHandle(threadHandle);
				threadHandle = NULL;
				return 1;

			}
			else
			{
				printf("Waiting for thread failed. Ending program\n");
				return 1;
			}
		}
	}
	if (closesocket(MainSocket) == SOCKET_ERROR)
		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
	if (WSACleanup() == SOCKET_ERROR)
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
}

