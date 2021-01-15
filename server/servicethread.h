/*
Description – An implementation of the thread that gives service to the clients 
				that should be accepted to the game.
*/

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
#include "ThreadsCommunication.h"

typedef enum { MAIN, FAILURE, DENIED } menuStatus;
typedef enum { MID_GAME, WIN, TIE } gameStatus;

typedef struct ThreadParam {
	SOCKET socket;
	int* p_numOfPlayersInGame;
	int* p_numOfPlayersSyncing;
	SOCKET* p_socket;

}ThreadParam;

/*
Description: In order to accept a client connection, this function will be called by a new thread.
			The function manages a single client's connection from the beginning to the end.
Parameters: 
	1. void* lpParam - The thread parameters
Returns: 0 if main menu was reached, o.w- NOT_SUCCESS.
*/
DWORD ServiceThread(void* lpParam);

/*
Description: A function that get's the client's connection request and username and send SERVER_APPROVE message
Parameters:
	1. SOCKET socket
	2. char** p_username - a pointer to a buffer where the username will be stored
Returns: SUCCESS, DISCONNECTED or NOT_SUCCESS
*/
int getUserNameAndApproveClient(SOCKET socket, char** p_username);

/*
Description: The main menu of the game: It sends the client the MAIN_MENU message and deals with the response.
			If possible, it will call the function that creates the communication file and initiate communication
			between the two clients.
Parameters:
	1. SOCKET socket
	2. HANDLE lockEvent
	3. HANDLE syncEvent
	4. int* p_numOfPlayersInGame
	5. int* p_playerOne - a poniter to an int the says if this is player 1 or not
	6. char* username - the player's user name
	7. char** p_opponentUsername - a pointer to the opponent user name
Returns: 0 if main menu was reached, o.w- NOT_SUCCESS.
*/
int Main_menu(SOCKET socket, HANDLE lockEvent, HANDLE syncEvent, int* p_numOfPlayersInGame, int* p_playerOne, char* username, char** p_opponentUsername);

/*
Description: The main menu of the game: It sends the client the MAIN_MENU message and deals with the response.
			If possible, it will call the function that creates the communication file and initiate communication
			between the two clients.
Parameters:
	1. SOCKET socket
	2. HANDLE lockEvent
	3. HANDLE syncEvent
	4. int* p_numOfPlayersInGame
	5. int* p_playerOne - a poniter to an int that indicates if this is player 1 or not
	6. char** p_username -a pointer to the player's user name
	7. char** p_opponentUsername - a pointer to the opponent user name
Returns: NOT_SUCCESS, DISCONNECT or SUCCESS
*/
int ExchangeClientsNames(SOCKET socket, HANDLE lockEvent, HANDLE syncEvent, int* p_numOfPlayersInGame, int* p_playerOne, char** p_username, char** p_opponentUsername);

/*
Description: Free resources before thread finishes
Parameters:
	1. SOCKET socket - the socket to close or NULL if no need to close the socket
	2. HANDLE lockEvent - the handle to close or NULL if no need to close the handle
	3. HANDLE syncEvent - the handle to close or NULL if no need to close the handle
	4. HANDLE failureEvent - the handle to close or NULL if no need to close the handle
	5. char* username - the username to free or NULL if no need to free the string
Returns: No return value
*/
void freeServiceThreadResources(SOCKET socket, HANDLE lockEvent, HANDLE syncEvent, HANDLE failureEvent, char* username);

/*
Description: startGame is incharge of managing the game right after the two players send CLIENT_VERSUS with
		 thier names. It manages the communication with the client, calcuates the game results and read/write the 
		 guesses and secret numbers.
Arguments:
	1. SOCKET socket
	2. HANDLE h_sharedFile - Handle to the threads communication file.
	3. HANDLE lockEvent - TODO
	4. HANDLE syncEvent - TODO
	5. int playerOne - Integer that flags to the thread if it is incharge of first player or second.
	6. int* p_numOfPlayersInGame - Pointer to an integer that counts the number of clients are connected to the server.
	7. char* username - The player username that recived from the player.
	8. char* opponentName - The opponent name that recived from the shared file.
	9. int* p_numOfPlayersSyncing - Flag that used for syncing the two threads.
returns: NOT_SUCCESS or MAIN_MENU or DISCONNECTED
*/
int startGame(SOCKET socket, HANDLE h_sharedFile, HANDLE lockEvent, HANDLE syncEvent, int playerOne, int* p_players, char* username, char* opponentName, int* p_playersCount);

//TODO - add Documentation
int secretNumInit(SOCKET socket, HANDLE h_sharedFile, HANDLE lockEvent, HANDLE syncEvent, int playerOne,
	int* p_numOfPlayersInGame, char* opponentName, int* p_numOfPlayersSyncing, char** p_userNum, char** p_opponentNum);

/*
Description: getResults calculates the game results and prepares the server message that will be sent to client.
Arguments:
	1. char** resultMsg - Pointer to string pointer that will contain the server message that will be sent.
	2. char* username - The player username that recived from the player.
	3. char* opponentName - The opponent name that recived from the shared file.
	4. char* userNum - User secret number.
	5. char* opponentNum - Opponent secret number.
	6. char* userGuess - User guess
	7. char* opponentGuess - Opponent guess.
returns: NOT_SUCCESS or MAIN_MENU or GAME_STILL_ON
*/
int getResults(char** resultMsg, char* username, char* opponentName, char* userNum, char* opponentNum, char* userGuess, char* opponentGuess);

/*Description: freeMemory made for more readable code. It's release memory for number of variables.
Arguments:
	1. char* userNum - User secret number. 
	2. char* opponentNum - Opponent secret number.
	3. char* userGuess - User guess
	4. char* opponentGuess - Opponent guess.
returns: None
*/
void freeSingleGameMemory(char* userNum, char* opponentNum, char* userGuess, char* opponentGuess);

/*
Description: winMsg creates the SERVER_WIN message.
Arguments:
	1. char* opponentNum - Opponent secret number.
	2. char* winnerName - The winner name.
returns: String poiner which contains the message.
*/
char* winMsg(char* opponentNum, char* winnerName);

#endif // !SERVICETHREAD_H