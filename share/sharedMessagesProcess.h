
#ifndef SHAREDMESSAGESPROCESS_H
#define SHAREDMESSAGESPROCESS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hardcodeddata.h"
#include "stringProcessor.h"

#define MSG_TYPE_LEN 28
#define GUESS_LEN 5
#define MSG_NUM 7

typedef enum { CLIENT_REQUEST, CLIENT_SETUP, CLIENT_PLAYER_MOVE, SERVER_DENIED, SERVER_INVITE,
SERVER_GAME_RESULTS, SERVER_WIN};
typedef enum {USERNAME,GUESS,DENIED_REASON,ALL_FIELDS,USER_AND_GUESS};
typedef struct Message {

	char type[MSG_TYPE_LEN];
	char* username;
	char* deniedReason;
	char* guess;
	char bulls;
	char cows;



}Message;


Message* messageDecoder(char* messageStr);
int setMessageParams(char* p_restOfMessage, Message* message);
Message* initMessage(char* messageType);
#endif // SHAREDMESSAGESPROCESS_H