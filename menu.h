//menu.h
#pragma once
#include "networkclient.h"
#include <vector>
#include <string>
#include <glui.h>
#include <thread>

//defines for menu callbacks
#define GOLF_NEW_GAME 82738
#define GOLF_NEW_GAME_CANCEL 82733
#define GOLF_NEW_GAME_OK 82732

#define GOLF_HELP 82632

#define GOLF_HOSTGAME 82631
#define GOLF_JOINGAME 72631

#define GOLF_PORT 20
#define GOLF_HOSTNAME 21
#define GOLF_STARTLISTENING 10
#define GOLF_CANCEL 12

#define GOLF_SERVER_ADDR 120
#define GOLF_SERVER_PORT 121
#define GOLF_REMOTE_NAME 122

//create a vector of threads so they can be managed
extern std::vector<std::thread> gThreads;
extern std::string port, ip;

//start Server
void StartServer() {
	std::cout << "Starting 1v1 Server" << std::endl;
	gServer.start(atoi(port.c_str()));
};

//connect to client
void StartClient() {
	std::cout << "Starting 1v1 Client" << std::endl;
	gClient.start(ip, atoi(port.c_str()));
};

//host game and join game dialog menu 
static GLUI_Button *hostBtnStartListening = NULL, *hostBtnStartGame = NULL, *hostBtnCancel = NULL;
static GLUI *hostgameMenu = NULL;
static GLUI_EditText *gluiHostName = NULL, *gluiPort = NULL;
static GLUI_Checkbox *hostCheckboxes[2];

static GLUI_EditText *gluiServerHostName = NULL, *gluiServerPort = NULL, *gluiRemoteName = NULL;
static GLUI *joingameMenu = NULL;

static std::string serverHostName, serverPort;
std::string remoteName;

//text callback for hostgame
static void txt_hostGame(int code) {
	if (code == GOLF_HOSTNAME) {
		if (myName.length() == 0 || port.length() == 0) {
			hostBtnStartListening->disable();
		}
		else {
			hostBtnStartListening->enable();
		}
	}
}

//callback when user presses buttons in hostgame dialog
static void btn_hostgame(int code) {

	if (code == GOLF_CANCEL) 
	{
		//close dialog if user pressed cancel
		hostgameMenu->close();
		hostgameMenu = NULL;

	}
	else if (code == GOLF_STARTLISTENING) 
	{
		//start listen server with given port from form in another thread;
		if (gThreads.size() == 0)
			gThreads.push_back(std::thread(StartServer));
		else
		{
			//exit old listen server and create new
			gServer.~Server();
			gThreads[0].join();
			gThreads[0] = std::thread(StartServer);
		}

		//close dialog
		hostgameMenu->close();
		hostgameMenu = NULL;
	}
}

void show_hostgame_menu() {
	GLUI_Panel *panel;

	if (hostgameMenu != NULL) {
		return;
	}
	//create dialog
	hostgameMenu = GLUI_Master.create_glui("Host Game");

	new GLUI_EditText(hostgameMenu, "Input file:", newFilename);

	gluiPort = new GLUI_EditText(hostgameMenu, "Port:", port, GOLF_PORT, &txt_hostGame);

	panel = hostgameMenu->add_panel("Players");
	hostCheckboxes[0] = hostgameMenu->add_checkbox_to_panel(panel, "");
	hostCheckboxes[0]->set_int_val(1);
	hostCheckboxes[0]->disable();

	hostCheckboxes[1] = hostgameMenu->add_checkbox_to_panel(panel, "");
	hostCheckboxes[1]->set_int_val(1);
	hostCheckboxes[1]->disable();

	hostgameMenu->add_column_to_panel(panel, false);
	gluiHostName = new GLUI_EditText(panel, "Your name:", myName, GOLF_HOSTNAME, &txt_hostGame);
	/* add statictext for each of the 3 remote players */

	panel = hostgameMenu->add_panel("Controls");
	hostBtnStartListening = hostgameMenu->add_button_to_panel(panel, "Start listening", GOLF_STARTLISTENING, &btn_hostgame);
	hostBtnStartListening->disable();
	hostgameMenu->add_column_to_panel(panel, false);
	hostBtnCancel = hostgameMenu->add_button_to_panel(panel, "Cancel/Stop", GOLF_CANCEL, &btn_hostgame);

}

static void txt_joingame(int code) {
}

static void btn_joingame(int code) {
	if (code == GOLF_CANCEL)
	{
		joingameMenu->close();
		joingameMenu = NULL;

	}
	else
	{
		//Try connect to server
		if (gThreads.size() == 0)
			gThreads.push_back(std::thread(StartClient));
		else
		{
			//exit listen server and create new
			gClient.~Client();
			gThreads[0].join();
			gThreads[0] = std::thread(StartClient);
		}

		//if connected and socket active to server
		
		joingameMenu->close();
		joingameMenu = NULL;

		//send your player name to server
		std::string msg;
		msg = "0:" + myName;
		gClient.sendMessage(msg.c_str());
		gNetworkMode = GOLF_NETWORK_CLIENT;
		
	}
}

void show_joingame_menu() {
	if (joingameMenu != NULL) {
		return;
	}

	joingameMenu = GLUI_Master.create_glui("Join Game");

	gluiServerHostName = new GLUI_EditText(joingameMenu, "Host addr:", ip, GOLF_SERVER_ADDR, &txt_joingame);

	gluiServerPort = new GLUI_EditText(joingameMenu, "Port:", port, GOLF_SERVER_PORT, &txt_joingame);

	gluiRemoteName = new GLUI_EditText(joingameMenu, "Your name:", myName, GOLF_REMOTE_NAME, &txt_joingame);

	joingameMenu->add_button("Join", 0, &btn_joingame);
	joingameMenu->add_button("Cancel/Stop", GOLF_CANCEL, &btn_joingame);
}