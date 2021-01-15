/*
Description - This module is incharge of socket operations and related functions - shared module
 */
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "WinsockFunc.h"


int InitializeWinsock() {
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		// Tell the user that we could not find a usable WinSock DLL.                                  
		return NOT_SUCCESS;
	}	
	return SUCCESS;
}
void shutdownConnection(SOCKET socket) {
	int status;
	char* buffer = NULL;

	if (shutdown(socket, SD_SEND)) {
		printf("Failed to shutdown, error %ld.\n", WSAGetLastError());
		if (closesocket(socket))
			printf("closesocket error (serverFullThread), error %ld.\n", WSAGetLastError());
		free(buffer);
	}
	status = ReceiveString(&buffer, socket, 3000);
	free(buffer);
	if (TRNS_DISCONNECTED != status) {
		printf("Shutdown sequence failed. Closing socket.\n");
		if (closesocket(socket))
			printf("closesocket error (shutDownConnection), error %ld.\n", WSAGetLastError());
	}
	if (closesocket(socket)) 
		printf("closesocket error (shutDownConnection), error %ld.\n", WSAGetLastError());
}
void confirmShutdown(SOCKET socket) {
	if (shutdown(socket, SD_SEND)) {
		printf("Failed to shutdown, error %ld.\n", WSAGetLastError());
		if (closesocket(socket))
			printf("closesocket error (confirmShutdown), error %ld.\n", WSAGetLastError());
	}
	if (closesocket(socket))
		printf("closesocket error (confirmShutdown), error %ld.\n", WSAGetLastError());
}

int initAddress(char* ip, int portNumber, SOCKADDR_IN* service) {
	unsigned long Address;

	Address = inet_addr(ip); 
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			ip);
		return NOT_SUCCESS;
	}
	(*service).sin_family = AF_INET;
	(*service).sin_addr.s_addr = Address;
	(*service).sin_port = htons(portNumber);

	return SUCCESS;

}

TransferResult_t SendBuffer(const char* Buffer, int BytesToSend, SOCKET sd)
{
	const char* CurPlacePtr = Buffer;
	int BytesTransferred;
	int RemainingBytesToSend = BytesToSend;

	while (RemainingBytesToSend > 0)
	{
		/* send does not guarantee that the entire message is sent */
		BytesTransferred = send(sd, CurPlacePtr, RemainingBytesToSend, 0);
		if (BytesTransferred == SOCKET_ERROR)
		{
			printf("send() failed, error %d\n", WSAGetLastError());
			return TRNS_DISCONNECTED;
		}

		RemainingBytesToSend -= BytesTransferred;
		CurPlacePtr += BytesTransferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}

TransferResult_t SendString(const char* Str, SOCKET sd)
{
	/* Send the the request to the server on socket sd */
	int TotalStringSizeInBytes;
	TransferResult_t SendRes;

	/* The request is sent in two parts. First the Length of the string (stored in
	   an int variable ), then the string itself. */

	TotalStringSizeInBytes = (int)(strlen(Str) + 1); // terminating zero also sent	

	SendRes = SendBuffer(
		(const char*)(&TotalStringSizeInBytes),
		(int)(sizeof(TotalStringSizeInBytes)), // sizeof(int) 
		sd);

	if (SendRes != TRNS_SUCCEEDED) return SendRes;

	SendRes = SendBuffer(
		(const char*)(Str),
		(int)(TotalStringSizeInBytes),
		sd);

	return SendRes;
}

TransferResult_t ReceiveBuffer(char* OutputBuffer, int BytesToReceive, SOCKET sd, int timeOut)
{
	char* CurPlacePtr = OutputBuffer;
	int BytesJustTransferred;
	int RemainingBytesToReceive = BytesToReceive;
	struct timeval waitTime = { timeOut, 0 };

	setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&waitTime, sizeof(struct timeval));
	while (RemainingBytesToReceive > 0)
	{
		/* send does not guarantee that the entire message is sent */
		BytesJustTransferred = recv(sd, CurPlacePtr, RemainingBytesToReceive, 0);
		if (BytesJustTransferred == SOCKET_ERROR)
		{
			printf("recv() failed, error %d\n", WSAGetLastError());
			return TRNS_DISCONNECTED;
		}
		else if (BytesJustTransferred == 0)
			return TRNS_DISCONNECTED; // recv() returns zero if connection was gracefully disconnected.
		else if (BytesJustTransferred < 0) {  //recv() is TIME_OUTED
			return TRNS_TIMEOUT;
		}

		RemainingBytesToReceive -= BytesJustTransferred;
		CurPlacePtr += BytesJustTransferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}


TransferResult_t ReceiveString(char** OutputStrPtr, SOCKET sd, int timeOut)
{
	/* Recv the request to the server on socket sd */
	int TotalStringSizeInBytes;
	TransferResult_t RecvRes;
	char* StrBuffer = NULL;

	if ((OutputStrPtr == NULL) || (*OutputStrPtr != NULL))
	{
		printf("The first input to ReceiveString() must be "
			"a pointer to a char pointer that is initialized to NULL. For example:\n"
			"\tchar* Buffer = NULL;\n"
			"\tReceiveString( &Buffer, ___ )\n");
		return TRNS_FAILED;
	}

	/* The request is received in two parts. First the Length of the string (stored in
	   an int variable ), then the string itself. */

	RecvRes = ReceiveBuffer(
		(char*)(&TotalStringSizeInBytes),
		(int)(sizeof(TotalStringSizeInBytes)), // 4 bytes
		sd, timeOut);

	if (RecvRes != TRNS_SUCCEEDED) return RecvRes;

	StrBuffer = (char*)malloc(TotalStringSizeInBytes * sizeof(char));

	if (StrBuffer == NULL)
		return TRNS_FAILED;

	RecvRes = ReceiveBuffer(
		(char*)(StrBuffer),
		(int)(TotalStringSizeInBytes),
		sd, timeOut);

	if (RecvRes == TRNS_SUCCEEDED)
	{
		*OutputStrPtr = StrBuffer;
	}
	else
	{
		free(StrBuffer);
	}

	return RecvRes;
}

