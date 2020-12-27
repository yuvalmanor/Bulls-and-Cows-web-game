
#ifndef SHAREDMESSAGESPROCESS_H
#define SHAREDMESSAGESPROCESS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hardcodeddata.h"

#define MSG_TYPE_LEN 28
#define GUESS_LEN 5
#define MSG_NUM 7

typedef enum { CLIENT_REQUEST, CLIENT_SETUP, CLIENT_PLAYER_MOVE, SERVER_DENIED, SERVER_INVITE,
SERVER_GAME_RESULTS, SERVER_WIN};
typedef struct Message {

	char type[MSG_TYPE_LEN];
	char* username;
	char* deniedReason;
	char guess[GUESS_LEN];
	int bulls;
	int cows;



}Message;


int messageDecoder(char* messageStr);
int setMessageParams(char* messageType, char* p_restOfMessage, Message** message);
#endif // SHAREDMESSAGESPROCESS_H