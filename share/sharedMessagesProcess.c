#include "sharedMessagesProcess.h"

int messageDecoder(char* messageStr) {
	
	char messageCpy[MSG_TYPE_LEN],*p_messageType=NULL;
	char* p_restOfMessage = NULL;
	Message* message = NULL;
	

	if (0 == (strcpy_s(messageCpy, strlen(messageStr), messageStr))) {
		printf("strcpy_s failed (messageDecoder)\n");
		return NOT_SUCCESS;
	}
	
	if (NULL == (p_restOfMessage = malloc(strlen(messageStr)))) {
		printf("Fatal error: memory allocation for tmpString failed.\n");
		return NOT_SUCCESS;
	}
	//Get message type
	p_messageType = strtok_s(messageCpy, ":", &p_restOfMessage);
	if (NULL == p_messageType) {
		printf("strtok_s failed(messageDecoder)\n");
		free(p_restOfMessage);
		return NOT_SUCCESS;
	}
	if (NULL == (message = (Message*)malloc(sizeof(Message)))) {
		printf("Fatal error: memory allocation failed (Message).\n");
		free(p_restOfMessage);
		return NOT_SUCCESS;
	}
	if (0 == (strcpy_s(message->type, strlen(p_messageType), p_messageType))) {
		printf("strcpy_s failed (messageDecoder)\n");
		free(message);
		free(p_restOfMessage);
		return NOT_SUCCESS;
	}
	//initialize Message fields to invalid values
	message->bulls = -1;
	message->cows = -1;
	message->deniedReason = NULL;
	message->username = NULL;
	if (0 == (strcpy_s(message->guess, GUESS_LEN, "-111"))) {
		printf("strcpy_s failed (messageDecoder)\n");
		free(message);
		return NOT_SUCCESS;
	}
	//initialize Message fields by message type
	


}
//NOT DONE
int setMessageParams(char* messageType, char* p_restOfMessage, Message* message) {
	
	const char* messageTypeArr[MSG_NUM] = { "CLIENT_REQUEST", "CLIENT_SETUP", "CLIENT_PLAYER_MOVE","SERVER_DENIED","SERVER_INVITE", "SERVER_GAME_RESULTS", "SERVER_WIN" };
	int messageNum = -1,i=0;

	for (i; i < MSG_NUM; i++) {
		if (!strcmp(messageType, messageTypeArr)) {
			messageNum = i;
			break;
		}
	}
	switch (messageNum) {
	case CLIENT_REQUEST:
		func(message->username, p_restOfMessage);
		message->username = func(p_restOfMessage);

	}
}