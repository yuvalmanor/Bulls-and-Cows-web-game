#ifndef BULLSANDCOWS_H
#define BULLSANDCOWS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hardcodeddata.h"
#include "SocketSendRecvTools.h"

static char* client_request_type = "CLIENT_REQUEST";
int playGame(char* username);
int playerChoice();

#endif // !BULLSANDCOWS_H
