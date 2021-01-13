#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef SERVICETHREAD_H
#define SERVICETHREAD_H
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "SocketSendRecvTools.h"
#include "sharedMessagesProcess.h"
#include "hardcodeddata.h"
#include "WinsockFunc.h"

typedef enum { MAIN, FAILURE, DENIED } menuStatus;
typedef enum { MID_GAME, WIN, TIE } gameStatus;

typedef struct ThreadParam {
	SOCKET socket;
	int* p_numOfPlayersInGame;
	int* p_numOfPlayersSyncing;
	SOCKET* p_socket;

}ThreadParam;

static numOfPlayers = 0;

DWORD ServiceThread(void* lpParam);
int Main_menu(SOCKET socket, HANDLE lockEvent, HANDLE syncEvent, int* p_players, int* playerOne, char* username, char** otherUsername); int getUserNameAndApproveClient(SOCKET socket, char** username);
int getUserNameAndApproveClient(SOCKET socket, char** username);
HANDLE openOrCreateFile(int* playerOne);
int writeToFile(HANDLE h_file, int offset, char* data, int playerOne, int writeUsername);
int readFromFile(HANDLE h_sharedFile, int offset, char** data, int playerOne, int readUsername);
int getEvents(HANDLE* lockEvent, HANDLE* syncEvent, HANDLE* FailureEvent);
void freeServiceThreadResources(SOCKET socket, HANDLE lockEvent, HANDLE syncEvent, HANDLE failureEvent, char* username);
int SyncTwoThreads(SOCKET socket, int* p_numOfPlayersSyncing, int* p_numOfPlayersInGame, HANDLE lockEvent, HANDLE syncEvent, int waitTime);
/*Description: startGame is incharge of managing the game right after the two players send CLIENT_VERSUS with
* thier names. It manages the communication with the client, calcuates the game results and read/write the 
* guesses and secret numbers.
* Arguments:
* 1. SOCKET socket
* 2. HANDLE h_sharedFile - Handle to the threads communication file.
* 3. HANDLE lockEvent - TODO
* 4. HANDLE syncEvent - TODO
* 5. int playerOne - Integer that flags to the thread if it is incharge of first player or second.
* 6. int* p_numOfPlayersInGame - Pointer to an integer that counts the number of clients are connected to the server.
* 7. char* username - The player username that recived from the player.
* 8. char* opponentName - The opponent name that recived from the shared file.
* 9. int* p_numOfPlayersSyncing - Flag that used for syncing the two threads.
* returns: NOT_SUCCESS or MAIN_MENU or DISCONNECTED
*/
int startGame(SOCKET socket, HANDLE h_sharedFile, HANDLE lockEvent, HANDLE syncEvent, int playerOne, int* p_players, char* username, char* opponentName, int* p_playersCount);
/*Description: getResults calculates the game results and prepares the server message that will be sent to
* client.
* Arguments:
* 1. char** resultMsg - Pointer to string pointer that will contain the server message that will be sent.
* 2. char* username - The player username that recived from the player.
* 3. char* opponentName - The opponent name that recived from the shared file.
* 4. char* userNum - User secret number.
* 5. char* opponentNum - Opponent secret number.
* 6. char* userGuess - User guess
* 7. char* opponentGuess - Opponent guess.
* returns: NOT_SUCCESS or MAIN_MENU or GAME_STILL_ON
*/
int getResults(char** resultMsg, char* username, char* opponentName, char* userNum, char* opponentNum, char* userGuess, char* opponentGuess);
/*Description: opponentLeftGame checks if the opponent left the game, if he is, send SERVER_NO_OPPONENTS
* message to the client.
* Arguments:
* 1. SOCKET socket
* 2. int* p_players - Pointer to an integer that counts the number of clients are connected to the server.
* 3. HANDLE lockEvent - TODO
* returns: NOT_SUCCESS or MAIN_MENU
*/
int opponentLeftGame(SOCKET socket, int* p_players, HANDLE lockEvent);
/*Description: freeMemory made for more readable code. It's release memory for number of variables.
* Arguments:
* 1. char* userNum - User secret number.
* 2. char* opponentNum - Opponent secret number.
* 3. char* userGuess - User guess
* 4. char* opponentGuess - Opponent guess.
* returns: None
*/
void freeSingleGameMemory(char* userNum, char* opponentNum, char* userGuess, char* opponentGuess);
/*Description: winMsg creates the SERVER_WIN message.
* Arguments:
* 2. char* opponentNum - Opponent secret number.
* 3. char* winnerName - The winner name.
* returns: String poiner which contains the message.
*/
char* winMsg(char* opponentNum, char* winnerName);
#endif // !SERVICETHREAD_H