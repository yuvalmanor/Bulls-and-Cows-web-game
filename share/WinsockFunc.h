/*
Description - This module is incharge of socket operations and related functions - shared module
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef WINSOCKFUNC_H
#define WINSOCKFUNC_H
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hardcodeddata.h"

typedef enum { TRNS_FAILED, TRNS_SUCCEEDED, TRNS_DISCONNECTED, TRNS_TIMEOUT } TransferResult_t;


/*
Description: Wrapper function to make the serverManager more readable
Returns: SUCCESS or NOT_SUCCESS
*/
int InitializeWinsock();

/*
Description: This function implements the sequence of a graceful shutdown from the side that requests to disconnect
Parameters:
	SOCKET socket
Returns: No return value
*/
void shutdownConnection(SOCKET socket);
/*
Description: This function implements the sequence of a graceful shutdown from the side that didn't request to disconnect
Parameters:
	SOCKET socket
Returns: No return value
*/
void confirmShutdown(SOCKET socket);

/*
Description: Wrapper function of inet_addr to make the serverManager more readable
Parameters: 
	char* ip - The IP to use for the address
	int portNumber - The port number to use for the address
	SOCKADDR_IN* service - A pointer to a SOCKADDR_IN struct
Returns: SUCCESS or NOT_SUCCESS
*/
int initAddress(char* ip, int portNumber, SOCKADDR_IN* service);

/*
 * Description - SendBuffer() uses a socket to send a buffer.
 * Arguments -
 *		1. const char* Buffer - the buffer containing the data to be sent.
 *		2. int BytesToSend - the number of bytes from the Buffer to send.
 *		3. SOCKET sd - the socket used for communication.
 * Returns:
 *		TRNS_SUCCEEDED - if sending succeeded
 *		TRNS_FAILED - otherwise
 */
TransferResult_t SendBuffer(const char* Buffer, int BytesToSend, SOCKET sd);

/*
 * Description - SendString() uses a socket to send a string.
 * Arguments:
 *		1. const char* Str - the string to send.
 *		2. SOCKET sd - the socket used for communication.
 * Returns: TRNS_FAILED, TRNS_SUCCEEDED, TRNS_DISCONNECTED or TRNS_TIMEOUT
 */
TransferResult_t SendString(const char* Str, SOCKET sd);

/*
 * Description - ReceiveBuffer() uses a socket to receive a buffer.
 * Arguments:
 *		1. char* OutputBuffer - pointer to a buffer into which data will be written
 *		2. int RemainingBytesToReceive- size in bytes of Output Buffer
 *		3. SOCKET sd - the socket used for communication.
 *		4. int timeOut - The wait time for receiving the buffer
 * Returns:  
		TRNS_SUCCEEDED - if receiving succeeded
 		TRNS_DISCONNECTED - if the socket was disconnected
 		TRNS_FAILED - otherwise
 */
TransferResult_t ReceiveBuffer(char* OutputBuffer, int RemainingBytesToReceive, SOCKET sd, int timeOut);

/**
 * Description - ReceiveString() uses a socket to receive a string, and stores it in dynamic memory.
 * Arguments:
 *		1. char** OutputStrPtr - a pointer to a char-pointer that is initialized to NULL
 *		2. SOCKET sd - the socket used for communication.
 *		3. int timeOut - The wait time for sending the buffer
 *
 * Returns:
 *		TRNS_SUCCEEDED - if receiving and memory allocation succeeded
 *		TRNS_DISCONNECTED - if the socket was disconnected
 *		TRNS_FAILED - otherwise
 */
TransferResult_t ReceiveString(char** OutputStrPtr, SOCKET sd, int timeOut);

#endif // !WINSOCKFUNC_H