#include "sharedMessagesProcess.h"

int getMessage(SOCKET socket, Message** message, int waitTime) {
	TransferResult_t transResult;
	char* p_rawMessage = NULL;
	transResult = ReceiveString(&p_rawMessage, socket, waitTime);
	if (transResult == TRNS_FAILED) {
		printf("Transfer failed\n");
		return TRNS_FAILED;
	}
	else if (transResult == TRNS_DISCONNECTED) {
		printf("Transfer disconnected\n");
		if (NULL != p_rawMessage) {
			free(p_rawMessage);
		}
		return TRNS_DISCONNECTED;
	}
	else if (transResult == TRNS_TIMEOUT) {
		printf("Transfer timed out\n");
		return TRNS_TIMEOUT;
		}
	(*message) = messageDecoder(p_rawMessage);
	if (*message == NULL) {
		printf("There was a problem with processing the message\n");
		free(p_rawMessage);
		return TRNS_FAILED; //We might need to change this 
	}
	free(p_rawMessage);
	return TRNS_SUCCEEDED;
}

int sendMessage(SOCKET socket, char* rawMessage) {
	char* p_rawMessage = rawMessage;
	TransferResult_t transResult;

	transResult = SendString(&p_rawMessage, socket);
	if (transResult == TRNS_SUCCEEDED) {
		return 1;
	}
	if (transResult == TRNS_FAILED) {
		printf("transfer %s failed\n", p_rawMessage);
		return 0;
	}
	else if (transResult == TRNS_DISCONNECTED) {
		printf("transfer disconnected\n");
		return -1;
	}

}

Message* messageDecoder(char* messageStr){
	char* p_messageCpy = NULL, * messageType = NULL, * p_restOfMessage = NULL;
	int msgTypeInt=-1, numOfParams=-1;
	Message* message = NULL;

	//p_messageCpy memory allocation
	if (NULL == (p_messageCpy = malloc(strlen(messageStr) + 1))) {
		printf("Fatal error: memory allocation for tmpString failed.\n");
		return NULL;
	}

	//copy messageStr to p_messageCpy
	if (0 != (strcpy_s(p_messageCpy, strlen(messageStr) + 1, messageStr))) {
		printf("strcpy_s failed (messageDecoder)\n");
		//free p_messageCpy
		return NULL;
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
	if (NULL == message) return NULL;
	msgTypeInt = getMessageType(messageType);
	//numOfParams = getParamsNum(msgTypeInt);		//Replaced the function call to if-elseif-else
	if (msgTypeInt == SERVER_WIN) numOfParams = 2;	//comment here for the case its not working for some reason.
	else if (msgTypeInt == SERVER_GAME_RESULTS) numOfParams = 4;
	else
		numOfParams = 1;
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
		printf("strcpy_s failed (messageDecoder), message type might be invalid\n");
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
	printf("invalid message type\n");
	return NOT_SUCCESS;
}

int getField(int msgType) {
	int field = -1;

	if (msgType == CLIENT_REQUEST || msgType == SERVER_INVITE) field = USERNAME;
	else if (msgType == CLIENT_SETUP || msgType == CLIENT_PLAYER_MOVE) field = GUESS;
	else if (msgType == SERVER_DENIED) field = DENIED_REASON;
	else if (msgType == SERVER_GAME_RESULTS) field = ALL_FIELDS;
	else if (msgType == SERVER_WIN) field = USER_AND_GUESS;
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
char* prepareMsg(const char* msgType, char* str) {
	char* message = NULL;
	int messageLen = -1;
	if (NULL != str) {
		messageLen = strlen(msgType) + strlen(str) + 2; //+2 for \n and \0
	}
	else
		messageLen = strlen(msgType) + 2; //+2 for \n and \0

	if (NULL == (message = malloc(messageLen))) {
		printf("Fatal error: memory allocation failed (prepareMsg).\n");
		return NULL;
	}
	strcpy_s(message, messageLen, msgType);
	if (NULL != str) {
		strcat_s(message, messageLen, str);
	}
	strcat_s(message, messageLen, "\n");
	return message;

}
int strToInt(char* stringNum) {
	int result;
	if (strcmp(stringNum, "0\n") == 0 || strcmp(stringNum, "0") == 0) //If the number read is 0, don't use atoi
		result = 0;
	else if (0 == (result = atoi(stringNum))) { //use atoi and check if it failed
		printf("atoi failed\n");
		return -1;
	}
	return result;
}