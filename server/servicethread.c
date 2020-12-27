#include "servicethread.h"

DWORD ServiceThread(SOCKET* t_socket) {

	printf("Im the server thread.\n");
	//communicate with client.
	closesocket(*t_socket);
	return 0;


}

