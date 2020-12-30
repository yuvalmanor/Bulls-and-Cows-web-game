#include "servicethread.h"


DWORD ServiceThread(ThreadParam* lpParam) {
	//get thread parameters
	ThreadParam* p_param;
	if (NULL == lpParam) {
		printf("Service thread can't work with NULL as parameters\n");
		return -1;
	}
	p_param = (ThreadParam*)lpParam;
	SOCKET socket = p_param->socket;
	int offset = p_param->offset;

	TransferResult_t transResult;
	char* receivedStr = NULL;

	transResult = ReceiveString(&receivedStr, socket);
	if (transResult != TRNS_SUCCEEDED) { //If string not received properly
		if (transResult == TRNS_FAILED) {
			printf("transfer failed\n"); 
			//do something
		}
		else {
			printf("transfer disconnected\n");
			//do something
		}
	}
	printf("%s\n", receivedStr);
	

	printf("Im the server thread.\n");
	//communicate with client.
	closesocket(socket);
	return 0;


}

