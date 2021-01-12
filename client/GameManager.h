// This module is incharge of managing the game.
#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hardcodeddata.h"
#include "SocketSendRecvTools.h"
#include "sharedMessagesProcess.h"
#include "WinsockFunc.h"

#define CLIENT_MSG_NUM 5
#define REQUEST_LEN 14
#define CLEAN 1
#define EXIT 0
#define START_AGAIN 2
#define CONTINUE 4

typedef enum { MAIN, FAILURE, DENIED } menuStatus;
typedef enum { MID_GAME, WIN, TIE } gameStatus;

/*Description: playGame is incharge of managing the game. It's start right after that the connection with
* the server is maid.
* Arguments:
* 1. char* username - The player username that recived from the player.
* 2. SOCKET c_socket
* 3. SOCKADDR_IN clientService - Passed from clientManager. Needed for reconnection.
* 4. char* ip - The IP address recived from the user.
* 5. int portNumber - The port number that recived from the user.
* returns: SUCCESS or NOT_SUCCESS
*/
int playGame(char* username, SOCKET c_socket, SOCKADDR_IN clientService, char* ip, int portNumber);
/*Description: setup function is incharge of the client request from the server. It is also check if the server 
* approved the request ot denied it and making the reconnection if needed.
* Arguments:
* 1. char* username - The player username that recived from the player.
* 2. SOCKET c_socket
* 3. SOCKADDR_IN clientService - Passed from clientManager. Needed for reconnection.
* 4. char* ip - The IP address recived from the user.
* 5. int portNumber - The port number that recived from the user.
* returns: SUCCESS or NOT_SUCCESS
*/
int setup(char* username, SOCKET c_socket, SOCKADDR_IN clientService, char* ip, int portNumber);
/*Description: playAgainst is called if player want to play against another player. It is managine the
* communication between client and server and tell if it's possible to play against someone, if it is,
* it calls to GameIsOn function.
* Arguments:
* 1. SOCKET c_socket
* returns: SUCCESS or NOT_SUCCESS or START_AGAIN
*/
int playAgainst(SOCKET c_socket);
/*Description: GameIsOn is incharge of the game flow - It asks the user his secret number
* and his guesses, gets from the server the results and acts accordingly. 
* Arguments:
* 1. SOCKET c_socket
* returns: SUCCESS or NOT_SUCCESS or START_AGAIN
*/
int GameIsOn(SOCKET c_socket);
/*Description: playerChoice gets the user choice from the menu and validate it.
* Arguments: None
* returns: Integer that represnt the user choice - '1' or '2'
*/
int playerChoice();
/*Description: checkTRNSCode checks the transmission code that recived from the socket communication
* messages and acts accordingly. 
* Arguments:
* 1. int TRNSCode - Transmission code that recived from the socket communication.
* 2. char* ip - The IP address recived from the user.
* 3. int portNumber - The port number that recived from the user.
* 4. SOCKET c_socket
* 5. SOCKADDR_IN clientService - Passed from clientManager. Needed for reconnection.
* returns: SUCCESS or NOT_SUCCESS or EXIT
*/
int checkTRNSCode(int TRNSCode, char* ip, int portNumber, SOCKET c_socket, SOCKADDR_IN clientService);
/*Description: menu function prints the desired menu according to the game situation, calls the playerChoice
* function and return the player choice.
* Arguments:
* 1. menuStatus desiredMenu - enum that represent the desired menu: MAIN, FAILURE, DENIED.
* 2. char* ip - The IP address recived from the user.
* 3. int portNumber - The port number that recived from the user.
* returns: Integer that represnt the player choice or NOT_SUCCESS.
*/
int menu(menuStatus desiredMenu, char* ip, int portNumber);
/*Description: chooseNumber gets from user his secret number or guess and validate it.
* Arguments: None
* returns: String pointer to the user number.
*/
char* chooseNumber();
/*Description: opponentQuit checks if the opponent quit.
* Arguments:
* 1. char* message - The message type that recived from the server.
* 2. Message* serverMsg - Message struct variable for reciving message from the server.
* 3. SOCKET c_socket
* returns: NOT_SUCCESS or START_AGAIN or CONTINUE.
*/
int opponentQuit(char* message, Message* serverMsg, SOCKET c_socket);
/*Description: gameResults prints to the user the game results.
* Arguments:
* 1. Message* serverMsg - Message struct variable that contain the SERVER_GAMER_RESULTS message.
* 2. gameStatus status - enum that represnt the game status: MID_GAME, WIN, TIE.
* returns: None.
*/
void gameResults(Message* message, gameStatus status);
/*Description: makeConnection is a wrapper for connect function - it takes care for the situation that
* the connect failed, asks the user what to do next and acts accordingly.
* Arguments:
* 1. SOCKET c_socket
* 2. SOCKADDR_IN clientService - Passed from clientManager. Needed for reconnection. 
* 3. char* ip - The IP address recived from the user.
* 4. int portNumber - The port number that recived from the user.
* returns: SUCCESS or NOT_SUCCESS ot EXIT.
*/
int makeConnection(SOCKET c_socket, SOCKADDR_IN clientService, char* ip, int portNumber);
/*Description: resourcesManager manage the socket and the WSACleanup.
* Arguments:
* 1. SOCKET c_socket
* 2. int WSACleanFlag - Flag to decide if WSAC need cleanup or not.
* returns: None.
*/
void resourcesManager(SOCKET c_socket, int WSACleanFlag);
#endif // !GAMEMANAGER_H
