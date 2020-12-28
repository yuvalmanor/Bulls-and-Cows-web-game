#include "sharedMessagesProcess.h"

Message* messageDecoder(char* messageStr) {
	
	char* p_messageCpy = NULL, * messageType = NULL, * p_restOfMessage = NULL;
	
	Message* message = NULL;
	
	//p_messageCpy memory allocation
	if (NULL == (p_messageCpy = malloc(strlen(messageStr) + 1))) {
		printf("Fatal error: memory allocation for tmpString failed.\n");
		return NOT_SUCCESS;
	}

	//copy messageStr to p_messageCpy
	if (0 != (strcpy_s(p_messageCpy, strlen(messageStr)+1, messageStr))) {
		printf("strcpy_s failed (messageDecoder)\n");
		return NOT_SUCCESS;
	}

	//<---Get message type--->
	//in case the message is without parameters
	if (NULL == (strchr(p_messageCpy, ':')))
	{
		messageType = strtok_s(p_messageCpy, "\n", &p_restOfMessage);
		message = initMessage(messageType);
		printf("type:%s", message->type);
		free(p_messageCpy);
		return message;
	}
	//in case the message is with parameters
	messageType = strtok_s(p_messageCpy, ":", &p_restOfMessage);
	if (NULL == messageType) {
		printf("strtok_s failed(messageDecoder)\n");
		free(p_messageCpy);
		return NULL;
	}
	//initialize invalid message parameters.
	message = initMessage(messageType);
	printf("type: %s", messageType);
	
	//initialize Message fields by message type
	setMessageParams(p_restOfMessage, message);
	printf("denied reason: %s\n", message->deniedReason);

	free(message->deniedReason);
	free(p_messageCpy);
	free(message);
	return SUCCESS;

	
	
	
	


}

int setMessageParams(char* p_restOfMessage, Message* message) {
	
	const char* messageTypeArr[MSG_NUM] = { "CLIENT_REQUEST", "CLIENT_SETUP", "CLIENT_PLAYER_MOVE","SERVER_DENIED","SERVER_INVITE", "SERVER_GAME_RESULTS", "SERVER_WIN" };
	char* p_currentRest = NULL, *token = NULL;
	int fieldToInit = -1,i=0;
	

	for (i; i < MSG_NUM; i++) {
		if (!strcmp(message->type, messageTypeArr[i])) 
			break;
	}
	if (i == CLIENT_REQUEST || i==SERVER_INVITE) fieldToInit = USERNAME;
	else if (i == CLIENT_SETUP || i == CLIENT_PLAYER_MOVE) fieldToInit = GUESS;
	else if (i == SERVER_DENIED) fieldToInit = DENIED_REASON;
	else if (i == SERVER_GAME_RESULTS) fieldToInit = ALL_FIELDS;
	else if (i == SERVER_WIN) fieldToInit == USER_AND_GUESS;
 	switch (fieldToInit) {
	case USERNAME:
		processAndCopy(&message->username, p_restOfMessage, "\n");
		break;
	case GUESS:
		processAndCopy(&message->guess, p_restOfMessage, "\n");
		break;
	case DENIED_REASON:
		processAndCopy(&message->deniedReason, p_restOfMessage, "\n");
		break;
	case ALL_FIELDS:
		token = strtok_s(p_restOfMessage, ";", &p_currentRest);
		message->bulls = *token;
		token= strtok_s(NULL, ";", &p_currentRest);
		message->cows = *token;
		//p_currentRest=oded;3456\n"
		printf("current1:%s\n", p_currentRest);
		processAndCopy(&message->username, p_currentRest, ";");
		printf("current2:%s\n", p_currentRest);
		printf("p_restof: %s\n", p_restOfMessage);
		processAndCopy(&message->guess, NULL, "; \n");
		break;
	}
	i = 0;

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