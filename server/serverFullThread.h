/*
Description - A module for the thread that is incharge of letting the client know it is
			denied.
*/

#ifndef SERVERFULLTHREAD_H
#define SERVERFULLTHREAD_H
#include "servicethread.h"
#include "WinsockFunc.h"

/*
Description: In order to deny a client connection, this function will be called by a new thread.
			The function lets the clint know it's being denied and then disconnects from it.
Parameters:
	1. ThreadParam* lpParam - The thread parameters
Returns: 0 if successful, o.w- NOT_SUCCESS.
*/
DWORD ServerFullThread(ThreadParam* lpParam);

#endif // !SERVERFULLTHREAD_H