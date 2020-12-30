#include "serverFullThread.h"


DWORD ServerFullThread(ThreadParam* lpParam) {

	ThreadParam* p_param;
	//get thread parameters
	if (NULL == lpParam) {
		printf("Service thread can't work with NULL as parameters\n");
		return -1;
	}
	p_param = (ThreadParam*)lpParam;
	SOCKET socket = p_param->socket;

	printf("This server is full!\n");
	//inform client and finish connection
	closesocket(socket); //change to a more subtle way of ending connection
	return 0;

}