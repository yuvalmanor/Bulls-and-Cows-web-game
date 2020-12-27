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
#include "hardcodeddata.h"
#include "servicethread.h"

serverMain(int portNumber);

#endif // !SERVERMAIN_H