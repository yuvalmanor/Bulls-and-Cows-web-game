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
