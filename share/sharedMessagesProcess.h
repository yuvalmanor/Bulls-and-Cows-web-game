
#ifndef SHAREDMESSAGESPROCESS_H
#define SHAREDMESSAGESPROCESS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hardcodeddata.h"
#include "stringProcessor.h"
#include "SocketSendRecvTools.h"

#define MSG_TYPE_LEN 28
#define GUESS_LEN 5
#define MSG_NUM 7

typedef enum { CLIENT_REQUEST, CLIENT_SETUP, CLIENT_PLAYER_MOVE, SERVER_DENIED, SERVER_INVITE,
SERVER_GAME_RESULTS, SERVER_WIN}messageTypesWithParams;
typedef enum {USERNAME,GUESS,DENIED_REASON,ALL_FIELDS,USER_AND_GUESS}fieldsParams;
typedef struct Message {

	char type[MSG_TYPE_LEN];
	char* username;
	char* deniedReason;
	char* guess;
	char bulls;
	char cows;



}Message;

int getMessage(SOCKET socket, Message** message, int waitTime);
int sendMessage(SOCKET socket, char* rawMessage);
Message* messageDecoder(char* messageStr);
int setMessageParams(char* p_restOfMessage, int numOfParams, int msgType, Message* message);
Message* initMessage(char* messageType);
int getMessageType(char* messageType);
int getField(int msgType);
int getParamsNum(int type);
int initOneParam(char* restOfMessage, int msgType, Message* message);
int initTwoParams(char* restOfMessage, Message* message);
int initFourParams(char* restOfMessage, Message* message);
#endif // SHAREDMESSAGESPROCESS_H