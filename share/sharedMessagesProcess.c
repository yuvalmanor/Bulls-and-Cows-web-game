#include "sharedMessagesProcess.h"

Message* messageDecoder(char* messageStr){
	char* p_messageCpy = NULL, * messageType = NULL, * p_restOfMessage = NULL;
	int msgTypeInt=-1, numOfParams=-1;
	Message* message = NULL;

	//p_messageCpy memory allocation
	if (NULL == (p_messageCpy = malloc(strlen(messageStr) + 1))) {
		printf("Fatal error: memory allocation for tmpString failed.\n");
		return NOT_SUCCESS;
	}

	//copy messageStr to p_messageCpy
	if (0 != (strcpy_s(p_messageCpy, strlen(messageStr) + 1, messageStr))) {
		printf("strcpy_s failed (messageDecoder)\n");
		return NOT_SUCCESS;
	}
	//<---Get message type--->
	//in case the message is without parameters
	if (NULL == (strchr(p_messageCpy, ':')))
	{
		messageType = strtok_s(p_messageCpy, "\n", &p_restOfMessage);
		if (NULL == messageType) {
			printf("strtok_s failed(messageDecoder)\n");
			free(p_messageCpy);
			return NULL;
		}
		message = initMessage(messageType);
		printf("type:%s", message->type);
		free(p_messageCpy);
		return message;
	}
	messageType = strtok_s(p_messageCpy, ":", &p_restOfMessage);
	if (NULL == messageType) {
		printf("strtok_s failed(messageDecoder)\n");
		free(p_messageCpy);
		return NULL;
	}
	//in case the message is with parameters
	message = initMessage(messageType);
	if (NULL == message) /*free stuff*/return NULL;
	msgTypeInt = getMessageType(messageType);
	numOfParams = getParamsNum(msgTypeInt);
	setMessageParams(p_restOfMessage, numOfParams, msgTypeInt, message);
	//free p_restOfMessage?
	free(p_messageCpy);
	return message;
}

int setMessageParams(char* p_restOfMessage, int numOfParams, int msgType, Message* message) {
	
	
 	switch (numOfParams) {
	case 1:
		if (NOT_SUCCESS == (initOneParam(p_restOfMessage, msgType, message)))
			return NOT_SUCCESS;
		break;
	case 2:
		if (NOT_SUCCESS == (initTwoParams(p_restOfMessage, message)))
			return NOT_SUCCESS;
		break;
	case 4:
		if (NOT_SUCCESS == (initFourParams(p_restOfMessage, message)))
			return NOT_SUCCESS;
		break;
	
	}
	
	return SUCCESS;
}

Message* initMessage(char* messageType) {

	Message* message = NULL;

	if (NULL == (message = (Message*)malloc(sizeof(Message)))) {
		printf("Fatal error: memory allocation failed (Message).\n");
		return NULL;
	}
	if (0 != (strcpy_s(message->type, strlen(messageType)+1, messageType))) {
		printf("strcpy_s failed (messageDecoder)\n");
		free(message);
		return NULL;
	}
	
	//initialize Message fields to invalid values
	message->bulls = '-';
	message->cows = '-';
	message->deniedReason = NULL;
	message->username = NULL;
	message->guess = NULL;

	return message;
}
int getParamsNum(int type) {
	
	if (type == SERVER_WIN) return 2;
	else if (type == SERVER_GAME_RESULTS) return 4;
	else
		return 1;
}
int getMessageType(char* messageType) {
	const char* messageTypeArr[MSG_NUM] = { "CLIENT_REQUEST", "CLIENT_SETUP", "CLIENT_PLAYER_MOVE","SERVER_DENIED","SERVER_INVITE", "SERVER_GAME_RESULTS", "SERVER_WIN" };
	int type = -1, i=0;

	for (i; i < MSG_NUM; i++) {
		if (!strcmp(messageType, messageTypeArr[i]))
			return i;
	}
}

int getField(int msgType) {
	int field = -1;

	if (msgType == CLIENT_REQUEST || msgType == SERVER_INVITE) field = USERNAME;
	else if (msgType == CLIENT_SETUP || msgType == CLIENT_PLAYER_MOVE) field = GUESS;
	else if (msgType == SERVER_DENIED) field = DENIED_REASON;
	else if (msgType == SERVER_GAME_RESULTS) field = ALL_FIELDS;
	else if (msgType == SERVER_WIN) field == USER_AND_GUESS;
	return field;
}
int initOneParam(char* restOfMessage, int msgType, Message* message) {
	int field = getField(msgType);
	char* token = NULL, * buffer = NULL, * currentRest = NULL;

	token = strtok_s(restOfMessage, "\n", &currentRest);
	if (NULL == (buffer = malloc(strlen(token) + 1))) {
		printf("Fatal error: memory allocation failed (initOneParam).\n");
		return NOT_SUCCESS;
	}
	strcpy_s(buffer, strlen(token) + 1, token);
	if (USERNAME == field)
		message->username = buffer;
	else if (GUESS == field)
		message->guess = buffer;
	else if (DENIED_REASON == field)
		message->deniedReason=buffer;
	return SUCCESS;
}
int initTwoParams(char* restOfMessage, Message* message) {
	char* token = NULL, * currentRest = NULL, * p_user = NULL, *p_guess = NULL;

	token = strtok_s(restOfMessage, ";", &currentRest);
	if (NULL == (p_user = malloc(strlen(token) + 1))) {
		printf("Fatal error: memory allocation failed (initTwoParam).\n");
		return NOT_SUCCESS;
	}
	strcpy_s(p_user, strlen(token) + 1, token);
	message->username = p_user;
	token = strtok_s(NULL, "\n", &currentRest);
	if (NULL == (p_guess = malloc(strlen(token) + 1))) {
		printf("Fatal error: memory allocation failed (initTwoParam).\n");
		return NOT_SUCCESS;
	}
	strcpy_s(p_guess, strlen(token) + 1, token);
	message->guess = p_guess;
	return SUCCESS;
}
int initFourParams(char* restOfMessage, Message* message) {
	char* token = NULL, * currentRest = NULL, * p_user = NULL, * p_guess = NULL;
	
	message->bulls = restOfMessage[0];
	restOfMessage += 2;
	message->cows = restOfMessage[0];
	restOfMessage += 2;
	if (NOT_SUCCESS == (initTwoParams(restOfMessage, message)))
		return NOT_SUCCESS;
	return SUCCESS;
}
