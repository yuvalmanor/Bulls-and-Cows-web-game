#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef SERVICETHREAD_H
#define SERVICETHREAD_H
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

DWORD ServiceThread(SOCKET* t_socket);

#endif // !SERVICETHREAD_H