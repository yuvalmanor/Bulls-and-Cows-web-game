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

int InitializeWinsock();
void shutdownConnection(SOCKET socket, char* buffer);
void confirmShutdown(SOCKET socket);
//SOCKADDR_IN initAddress(char* ip, int portNumber);
#endif // !WINSOCKFUNC_H