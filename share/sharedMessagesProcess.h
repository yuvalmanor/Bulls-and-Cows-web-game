// shareMessagesProccess module - Take care of message processing. This is a shared module.
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
/*Description: A wraper function for recv function. Converting the recived message string to Message type.
* Arguments:
* 1. SOCKET socket
* 2. Message** message - A pointer address to Message variable.
* 3. int waitTime - The response wait time expected.
* returns: TRNS_SUCCEEDED/TRNS_FAILED/TRNS_TIMEOUT/TRNS_DISCONNECTED
*/
int getMessage(SOCKET socket, Message** message, int waitTime);
int sendMessage(SOCKET socket, char* rawMessage);
/*Description: messageDecoder get a string and convert it to Message type variable.
* Arguments:
* 1. char* messageStr - Pointer to string whice conatins the message.
* returns: Message pointer.
*/
Message* messageDecoder(char* messageStr);
/*Description: setMessageParams use switch-case to decide which kind of initializing parameter function
* will be choose by the messge type.
* Arguments:
* 1. char* p_restOfMessage - Pointer to the message string that remains after cutting the message type.
* 2. int numOfParams = Number of parameters of the message.
* 3. int msgType - Integer (enum) that represnt the message type.
* 4. Message* message - pointer to Message variable.
* returns: SUCCESS or NOT_SUCCESS
*/
int setMessageParams(char* p_restOfMessage, int numOfParams, int msgType, Message* message);
/*Description: initMessage create the Message strucet variable and set the struct fields (except the type field)
to invalid values and the type field to valid value.
* Arguments:
* 1. char* messageType - Pointer to the message type string.
* returns: Message variable
*/
Message* initMessage(char* messageType);
/*Description: getMessageType get string message type and return an integer (enum) that represent the message type.
* Arguments:
* 1. char* messageType - Pointer to the message type string.
* returns: Integer (enum) that represent the message type or NOT_SUCCESS.
*/
int getMessageType(char* messageType);
/*Description: getField returns which fields shoulde be fill by the message type.
* Arguments:
* 1. int msgType - Integer (enum) that represent the message type.
* returns: Integer (enum) that represent the fields should be fill.
*/
int getField(int msgType);
//getParamsNum is not used anymore. Before the hagashe, delete it.
int getParamsNum(int type);
/*Description: initOneParam initialize the required parameter by its message type. This function made for all
* the message types have one parameter.
* Arguments:
* 1. char* restOfMessage - Pointer to the message string that remains after cutting the message type.
* 2. int msgType = Integer (enum) that represent the message type.
* 3. Message* message - pointer to Message variable.
* returns: SUCCESS or NOT_SUCCESS
*/
int initOneParam(char* restOfMessage, int msgType, Message* message);
/*Description: initTwoParams initialize the required parameters by its message type. This function made for all
* the message type have two parameter.
* Arguments:
* 1. char* restOfMessage - Pointer to the message string that remains after cutting the message type.
* 2. Message* message - pointer to Message variable.
* returns: SUCCESS or NOT_SUCCESS
*/
int initTwoParams(char* restOfMessage, Message* message);
/*Description: initFourParams initialize the required parameters by its message type. This function made for all
* the message type have four parameter. This function using initTwoParams function inside.
* Arguments:
* 1. char* restOfMessage - Pointer to the message string that remains after cutting the message type.
* 2. Message* message - pointer to Message variable.
* returns: SUCCESS or NOT_SUCCESS
*/
int initFourParams(char* restOfMessage, Message* message);
/*Description: prepareMsg peparing a string message with zero or one parameter to be send to the server/client.
* Arguments:
* 1. const char* msgType - The message type should be send.
* 2. char* str - NULL if no parmeter required or string pointer to the parameter.
* returns: Pointer to string which contain the message to be send.
*/
char* prepareMsg(const char* msgType, char* str);
#endif // SHAREDMESSAGESPROCESS_H
