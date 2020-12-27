#include "serverFullThread.h"


DWORD ServerFullThread(SOCKET* t_socket) {

	printf("This server is full!\n");
	//inform client and finish connection
	closesocket(*t_socket);
	return 0;

}