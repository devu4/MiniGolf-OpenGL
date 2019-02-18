#include <iostream>
#include <sstream>
#include <string>
#include <WS2tcpip.h>
#include "simulation.h"
#pragma comment(lib, "ws2_32.lib")

//method to increament score in multiplayer
static void IncrementScore() {

	if (gCourse->players[gCurrentPlayer].scores[gCurrentHole] == -1)
		gCourse->players[gCurrentPlayer].scores[gCurrentHole] += 2;
	else
		gCourse->players[gCurrentPlayer].scores[gCurrentHole]++;
};

//init a new game once client has connect
static void startNewOnlineGame()
{
	if (gCourse != NULL)
	{
		delete gCourse;
		//free(gCourse);
	}

	gCourse = NULL;
	gCourse = new course;
	gCurrentHole = 0;
	gCurrentPlayer = 0;
	gGameEnd = false;
	gBallHit = false;
	ball::resetIndex();

	gCourse->Load_Game(newFilename);
	gCourse->holes[gCurrentHole].SetupCushions();

	Player player;
	Player player1;

	if (gNetworkMode == GOLF_NETWORK_CLIENT)
	{
		player.name = opponentName;
		player1.name = myName;
	}
	else if (gNetworkMode == GOLF_NETWORK_SERVER)
	{
		player1.name = opponentName;
		player.name = myName;
	}

	gCourse->players.push_back(player);
	gCourse->players.push_back(player1);
	initScores();
}


class Server
{

public:
	sockaddr_in cli_addr;
	SOCKET gSocket;
	char buf[4096];

	~Server()
	{
		// Close the socket
		closesocket(gSocket);

		// Cleanup winsock
		WSACleanup();

	};

	void receive()
	{
		// While loop: accept and echo message back to client
		while (true)
		{
			ZeroMemory(buf, 4096);

			// Wait for client to send data
			int bytesReceived = recv(gSocket, buf, 4096, 0);
			if (bytesReceived == SOCKET_ERROR)
			{
				std::cerr << "Error in recv(). Quitting" << std::endl;
				break;
			}

			if (bytesReceived == 0)
			{
				std::cout << "Client disconnected " << std::endl;
				break;
			}

			//read in data from opponent
			readMessage(buf);
		}
	};

	void start(int port)
	{
		// Initialze winsock
		WSADATA wsData;
		WORD ver = MAKEWORD(2, 2);

		int wsOk = WSAStartup(ver, &wsData);
		if (wsOk != 0)
		{
			std::cerr << "Can't Initialize winsock! Quitting" << std::endl;
			return;
		}

		// Create a socket
		SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
		if (listening == INVALID_SOCKET)
		{
			std::cerr << "Can't create a socket! Quitting" << std::endl;
			return;
		}

		// Bind the ip address and port to a socket
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(port);
		hint.sin_addr.S_un.S_addr = INADDR_ANY; // Could also use inet_pton .... 

		bind(listening, (sockaddr*)&hint, sizeof(hint));

		std::cout << "Waiting for Client to Join!" << std::endl;

		// Tell Winsock the socket is for listening 
		listen(listening, SOMAXCONN);

		// Wait for a connection
		int clientSize = sizeof(cli_addr);

		gSocket = accept(listening, (sockaddr*)&cli_addr, &clientSize);

		char host[NI_MAXHOST];		// Client's remote name
		char service[NI_MAXSERV];	// Service (i.e. port) the client is connect on

		ZeroMemory(host, NI_MAXHOST); // same as memset(host, 0, NI_MAXHOST);
		ZeroMemory(service, NI_MAXSERV);

		if (getnameinfo((sockaddr*)&cli_addr, sizeof(cli_addr), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
		{
			std::cout << host << " connected on port " << service << std::endl;
		}
		else
		{
			inet_ntop(AF_INET, &cli_addr.sin_addr, host, NI_MAXHOST);
			std::cout << host << " connected on port " <<
				ntohs(cli_addr.sin_port) << std::endl;
		}

		// Close listening socket
		closesocket(listening);

		receive();

	};

	void sendMessage(const char* msg) {
		// Echo message back to client
		int sendResult = SOCKET_ERROR;

		do {
			sendResult = send(gSocket, msg, strlen(msg), 0);
		} while (sendResult == SOCKET_ERROR);
	};

	virtual void readMessage(const char* msg) {
		// Echo message back to client
		//print data coming from client into game
		std::string message = std::string(msg);
		std::cout << "Client: " << message << std::endl;

		std::stringstream ss(message);
		std::string s ;
		//parse
		while (std::getline(ss, s, ':'))
		{
			//Client is sending over name
			if(s == "0")
			{ 
				std::getline(ss, s, ':');
				opponentName = s;
				std::cout << opponentName << std::endl;
				gNetworkMode = GOLF_NETWORK_SERVER;
				std::string msg = "0:";
				msg += myName + ":";
				msg += newFilename;
				sendMessage(msg.c_str());
				startNewOnlineGame();
			}
			else if(s == "1")//client is sending over ball impulse vector
			{
				if (gCourse->players.size() > 1)
				{
					//parse impulse from msg received
					vec2d imp;
					std::getline(ss, s, ':');
					imp(0) = atof(s.c_str());
					std::getline(ss, s, ':');
					imp(1) = atof(s.c_str());

					//do impulse on players ball
					gCourse->players[gCurrentPlayer].ball.ApplyImpulse(imp);
					IncrementScore();//increment their score
					gBallHit = true;
					if (!gCourse->players[gCurrentPlayer].hadFirstShot)
					{
						gCourse->players[gCurrentPlayer].hadFirstShot = true;
					}

				}
			}
		}
	};
};

extern Server gServer;