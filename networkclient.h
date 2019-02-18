#include "networkserver.h"
#include <iostream>
#include <string>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")


class Client : public Server
{

public:

	~Client()
	{
		std::cout << "Closing 1v1 Client" << std::endl;
	};

	void start(std::string ip, int port)
	{
		// Initialize WinSock
		WSAData data;
		WORD ver = MAKEWORD(2, 2);
		int wsResult = WSAStartup(ver, &data);
		if (wsResult != 0)
		{
			std::cerr << "Can't start Winsock, Err #" << wsResult << std::endl;
			return;
		}

		// Create socket and save socket
		gSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (gSocket == INVALID_SOCKET)
		{
			std::cerr << "Can't create socket, Err #" << WSAGetLastError() << std::endl;
			WSACleanup();
			return;
		}

		// Fill in a hint structure
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(port);
		inet_pton(AF_INET, ip.c_str(), &hint.sin_addr);

		// Connect to server
		int connResult = connect(gSocket, (sockaddr*)&hint, sizeof(hint));
		if (connResult == SOCKET_ERROR)
		{
			std::cerr << "Can't connect to server, Err #" << WSAGetLastError() << std::endl;
			closesocket(gSocket);
			WSACleanup();
			return;
		}

		std::cout << "Client has connected to Server!" << std::endl;

		receive();

	};

	void readMessage(const char* msg) {
		// Echo message back to client
		//print data coming from client into game
		std::string message = std::string(msg);
		std::cout << "Server: " << message << std::endl;

		std::stringstream ss(message);
		std::string s;

		while (std::getline(ss, s, ':'))
		{
			//Server is sending over name
			if (s == "0")
			{
				std::getline(ss, s, ':');
				opponentName = s;
				std::cout << opponentName << std::endl;
				std::getline(ss, s, ':');
				newFilename = s;
				gNetworkMode = GOLF_NETWORK_CLIENT;
				startNewOnlineGame();
			}
			else if (s == "1")//Server is sending over ball impulse vector
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
					IncrementScore();//increment score
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

extern Client gClient;

