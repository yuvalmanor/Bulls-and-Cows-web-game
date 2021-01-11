#include "serverFullThread.h"


DWORD ServerFullThread(ThreadParam* lpParam) {

	ThreadParam* p_param;
	char* p_serverMsg = NULL;
	int status;
	Message* p_clientMsg = NULL;
	//get thread parameters
	if (NULL == lpParam) {
		printf("Service thread can't work with NULL as parameters\n");
		return NOT_SUCCESS; //who close the socket in such case?
	}
	p_param = (ThreadParam*)lpParam;
	SOCKET socket = p_param->socket;
	status = getMessage(socket, &p_clientMsg, RESPONSE_WAITTIME);
	if (status == TRNS_FAILED) {
		if (closesocket(socket))
			printf("closesocket error (serverFullThread), error %ld.\n", WSAGetLastError());
		printf("Socket closed.\n");
		return NOT_SUCCESS;
	}
	if (strcmp(p_clientMsg->type, "CLIENT_REQUEST")) { 
		free(p_clientMsg);
		if (closesocket(socket))
			printf("closesocket error (serverFullThread), error %ld.\n", WSAGetLastError());
		printf("Socket closed.\n");
		return NOT_SUCCESS;
	}
	free(p_clientMsg);
	p_serverMsg = prepareMsg("SERVER_DENIED:", "Server is full.");
	if (NULL == p_serverMsg) {
		if (closesocket(socket))
			printf("closesocket error (serverFullThread), error %ld.\n", WSAGetLastError());
		printf("Socket closed.\n");
		return NOT_SUCCESS;
	}
	status = SendString(p_serverMsg, socket); 
	if (status == TRNS_FAILED) {
		free(p_serverMsg);
			if (closesocket(socket))
				printf("closesocket error (serverFullThread), error %ld.\n", WSAGetLastError());
		printf("Socket closed.\n");
		return NOT_SUCCESS;
	}
	shutdownConnection(socket);
	return SUCCESS;


}