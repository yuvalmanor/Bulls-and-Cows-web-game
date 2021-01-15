#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef WINSOCKFUNC_H
#define WINSOCKFUNC_H
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <stdio.h>
#include <string.h>
#include "hardcodeddata.h"
#include "SocketSendRecvTools.h"

/*
Description: Wrapper function to make the serverManager more readable
Returns: SUCCESS or NOT_SUCCESS
*/
int InitializeWinsock();

void shutdownConnection(SOCKET socket);
void confirmShutdown(SOCKET socket);
/*
Description: Wrapper function of inet_addr to make the serverManager more readable
Parameters: 
	char* ip - The IP to use for the address
	int portNumber - The port number to use for the address
	SOCKADDR_IN* service - A pointer to a SOCKADDR_IN struct
Returns: SUCCESS or NOT_SUCCESS
*/
int initAddress(char* ip, int portNumber, SOCKADDR_IN* service);
#endif // !WINSOCKFUNC_H