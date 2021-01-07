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
int disconnect(SOCKET socket, char* buffer) {
	int status;
	
	if (shutdown(socket, SD_SEND)) {
		printf("Failed to shutdown, error %ld.\n", WSAGetLastError());
		if (closesocket(socket))
			printf("closesocket error (serverFullThread), error %ld.\n", WSAGetLastError());
		free(buffer);
		return NOT_SUCCESS;
	}
	status = ReceiveString(&buffer, socket, RESPONSE_WAITTIME);
	free(buffer);
	if (TRNS_DISCONNECTED != status) {
		printf("Shutdown sequence failed. Closing socket.\n");
		if (closesocket(socket))
			printf("closesocket error (serverFullThread), error %ld.\n", WSAGetLastError());
		return NOT_SUCCESS;
	}
	if (closesocket(socket)) {
		printf("closesocket error (serverFullThread), error %ld.\n", WSAGetLastError());
		return NOT_SUCCESS;
	}
	return SUCCESS;

}

/*SOCKADDR_IN initAddress(char* ip, int portNumber) {
	SOCKADDR_IN service;
	unsigned long Address;

	Address = inet_addr(ip); 
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			ip);
		return NULL;
	}
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address;
	service.sin_port = htons(portNumber);

	return service;

}*/
