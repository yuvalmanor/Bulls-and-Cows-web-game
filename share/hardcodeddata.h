
#ifndef HARDCODEDDATA_H
#define HARDCODEDDATA_H

#define LOCALHOST "127.0.0.1"
#define MAX_NUM_OF_PLAYERS 2
#define PAGE_SIZE 4096
#define SUCCESS 1
#define NOT_SUCCESS -1

#define LOCKEVENT_WAITTIME 5000
#define RESPONSE_WAITTIME 30000

#define MAX_USERNAME_LEN 20
#define SECRETNUMBER_LEN 4
#define DATABLOCKSIZE 16
#define SECRETNUM_OFFSET 0
#define GUESS_OFFSET 6
#define RESULT_OFFSET 12

#define FAILED -1
#define DISCONNECTED -2
#define MAIN_MENU 1
#define GAME_STILL_ON 2
#define QUIT 3

static LPCSTR lockEvent_name = _T("lockEvent");
static LPCSTR syncEvent_name = _T("syncEvent");
static LPCSTR FailureEvent_name = _T("Failure");
static LPCWSTR sharedFileName = _T("GameSession.txt");

#endif // !HARDCODEDDATA_H