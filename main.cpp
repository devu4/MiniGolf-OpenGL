// Pool Game.cpp : Defines the entry point for the console application.
#include "menu.h"
#include "stdafx.h"
#include "stdafx.h"
#include "iostream"
#include "sstream"
#include <glui.h>
#include<glut.h>
#include <thread>
#include "scorecard.h"
#pragma comment(lib, "SHELL32.LIB")

//cue variables
float gCueAngle = 0.0;
float gCuePower = 0.25;
bool gCueControl[4] = {false,false,false,false};
float gCueAngleSpeed = 2.0f; //radians per second
float gCuePowerSpeed = 0.25f;
float gCuePowerMax = 0.75;
float gCuePowerMin = 0.1;
float gCueBallFactor = 10.0;
bool gDoCue = true;
bool gBallHit = false;
bool gGameEnd = false;

//socket server and client
Server gServer;
Client gClient;

//g variables
int gCurrentHole = 0;
int gCurrentPlayer = 0;
std::string newFilename = "course.golf";
std::string playerNames = "david,player";
std::string myName = "";
std::string opponentName = "";
std::string port = "54000";
std::string ip = "127.0.0.1";
std::vector<std::thread> gThreads; //list of threads
int gNetworkMode = GOLF_NETWORK_NONE;

//camera variables
vec3d gCamPos = vec3d(0.0, 4.0, 0.1);
vec3d gCamLookAt(0.0, 0.0, 0.0);
bool gCamRotate = true;
float gCamRotSpeed = 0.5;
float gCamMoveSpeed = 5.8;
bool gCamL = false;
bool gCamR = false;
bool gCamU = false;
bool gCamD = false;
bool gCamZin = false;
bool gCamZout = false;

static int windowId;
//gui elements
static GLUI *glui = NULL, *gluiNewGame = NULL;
static GLUI *gluiScorecard = NULL;

//rendering options
#define DRAW_SOLID	(0)

void DoCamera(int ms)
{
	static const vec3d up(0.0,1.0,0.0);

	if(gCamRotate)
	{
		if(gCamL)
		{
			vec3d camDir = (gCamLookAt - gCamPos).Normalised();
			vec3d localL = up.Cross(camDir);
			vec3d inc = (localL* ((gCamRotSpeed*ms)/1000.0) );
			gCamLookAt = gCamPos + camDir + inc;
		}
		if(gCamR)
		{
			vec3d camDir = (gCamLookAt - gCamPos).Normalised();
			vec3d localR = up.Cross(camDir);
			vec3d inc = (localR* ((gCamRotSpeed*ms)/1000.0) );
			gCamLookAt = gCamPos + camDir - inc;
		}
		if(gCamU)
		{
			vec3d camDir = (gCamLookAt - gCamPos).Normalised();
			vec3d localR = camDir.Cross(up);
			vec3d localUp = localR.Cross(camDir);
			vec3d inc = (localUp* ((gCamMoveSpeed*ms)/1000.0) );
			gCamLookAt = gCamPos + camDir + inc;
		}
		if(gCamD)
		{
			vec3d camDir = (gCamLookAt - gCamPos).Normalised();
			vec3d localR = camDir.Cross(up);
			vec3d localUp = localR.Cross(camDir);
			vec3d inc = (localUp* ((gCamMoveSpeed*ms)/1000.0) );
			gCamLookAt = gCamPos + camDir - inc;
		}		
	}
	else
	{
		if(gCamL)
		{
			vec3d camDir = (gCamLookAt - gCamPos).Normalised();
			vec3d localL = up.Cross(camDir);
			vec3d inc = (localL* ((gCamMoveSpeed*ms)/1000.0) );
			gCamPos += inc;
			gCamLookAt += inc;
		}
		if(gCamR)
		{
			vec3d camDir = (gCamLookAt - gCamPos).Normalised();
			vec3d localR = camDir.Cross(up);
			vec3d inc = (localR* ((gCamMoveSpeed*ms)/1000.0) );
			gCamPos += inc;
			gCamLookAt += inc;
		}
		if(gCamU)
		{
			vec3d camDir = (gCamLookAt - gCamPos).Normalised();
			vec3d localR = camDir.Cross(up);
			vec3d localUp = localR.Cross(camDir);
			vec3d inc = (localUp* ((gCamMoveSpeed*ms)/1000.0) );
			gCamPos += inc;
			gCamLookAt += inc;
		}
		if(gCamD)
		{
			vec3d camDir = (gCamLookAt - gCamPos).Normalised();
			vec3d localR = camDir.Cross(up);
			vec3d localDown = camDir.Cross(localR);
			vec3d inc = (localDown* ((gCamMoveSpeed*ms)/1000.0) );
			gCamPos += inc;
			gCamLookAt += inc;
		}
	}

	if(gCamZin)
	{
		vec3d camDir = (gCamLookAt - gCamPos).Normalised();
		vec3d inc = (camDir* ((gCamMoveSpeed*ms)/1000.0) );
		gCamPos += inc;
		gCamLookAt += inc;
	}
	if(gCamZout)
	{
		vec3d camDir = (gCamLookAt - gCamPos).Normalised();
		vec3d inc = (camDir* ((gCamMoveSpeed*ms)/1000.0) );
		gCamPos -= inc;
		gCamLookAt -= inc;
	}
}

void startNewGame()
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
	gDoCue = true;
	gBallHit = false;
	ball::resetIndex();

	gCourse->Load_Game(newFilename);
	gCourse->holes[gCurrentHole].SetupCushions();

	std::stringstream ss(playerNames);
	std::string s;

	while (std::getline(ss, s, ','))
	{
		Player player;
		player.name = s;
		gCourse->players.push_back(player);
	}

	initScores();
	gNetworkMode = GOLF_NETWORK_NONE;
}

static void glutBitmapString(void *font, const char *str) {
	int i;
	for (i = 0; str[i] != '\0'; i++) {
		glutBitmapCharacter(font, str[i]);
	}
}

static void push2D() {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

}

static void pop2D() {
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void next_hole() {
	int next = gCurrentHole + 1;

	if (next <= gCourse->holes.size() - 1) {
		//go to next hole of course, reset all players and balls
		gCurrentHole = next;
		gCourse->holes[gCurrentHole].SetupCushions();
		resetPlayers();
	}
	else {
		gGameEnd = true;
		/* end of course; show the hiscores */
	}
}


static void gluiCallBack(int code) {

	if (code == GOLF_NEW_GAME && gluiNewGame == NULL) {
		/* create the window */
		gluiNewGame = GLUI_Master.create_glui("New Game");

		gluiNewGame->add_statictext("Start A New Game:");
		gluiNewGame->add_separator();

		GLUI_EditText *id;
		gluiNewGame->add_statictext("Enter the name of the map file");
		id = new GLUI_EditText(gluiNewGame, "Map file:", newFilename);
		id->set_w(400);
		gluiNewGame->add_statictext("Enter player names seperated by commas");
		id = new GLUI_EditText(gluiNewGame, "Player Name/s:", playerNames);
		id->set_w(400);

		/* OK and cancel buttons */
		gluiNewGame->add_button("OK", GOLF_NEW_GAME_OK, &gluiCallBack);
		gluiNewGame->add_button("Cancel", GOLF_NEW_GAME_CANCEL, &gluiCallBack);

	}
	else if (code == GOLF_NEW_GAME_OK) {
		/* user clicked OK; copy new values into vals */
		startNewGame();
	}
	else if (code == GOLF_HOSTGAME)
	{
		//show hostgame menu
		show_hostgame_menu();
	}
	else if (code == GOLF_JOINGAME)
	{
		//show join game menu
		show_joingame_menu();
	}
	if (code == GOLF_HELP)
	{
		//openr rules pdf and controls help
		ShellExecute(0, 0, L"ReadMe.pdf", 0, 0, SW_SHOW);
	}

	if (code == SCORECARD_OK) {
		gluiScorecard->close();
		gluiScorecard = NULL;

		next_hole();
	}

	/* if user hit OK or Cancel, free memory */
	if (code == GOLF_NEW_GAME_CANCEL || code == GOLF_NEW_GAME_OK) {
		gluiNewGame->close();
		gluiNewGame = NULL;
	}
}

static void show_scorecard() {
	gluiScorecard = CreateScoreCard(gCourse->players, &gluiCallBack);
}

void RenderScene(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set camera
	glLoadIdentity();
	gluLookAt(gCamPos(0),gCamPos(1),gCamPos(2),gCamLookAt(0),gCamLookAt(1),gCamLookAt(2),0.0f,1.0f,0.0f);
	std::string holeTitle;
	glClearColor(0.0f, 0.15f, 0.0f, 1.0f);

	if (gCourse->holes.size() < 1) {
		//not loaded course yet so show title screen
		push2D();
		glColor3f(0.0f, 1.0f, 0.0f);
		glRasterPos2f(-0.25, 0.05);
		holeTitle = "DAVID'S MINI MINIGOLF";
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, holeTitle.c_str());
		pop2D();
		push2D();
		glColor3f(0.0f, 1.0f, 0.0f);
		glRasterPos2f(-0.18, -0.05);
		holeTitle = "Click New Game To Start";
		glutBitmapString(GLUT_BITMAP_HELVETICA_12, holeTitle.c_str());
		pop2D();
	}
	else if (gGameEnd) 
	{
		//show GAME OVER and winner gui
		push2D();
		glColor3f(0.0f, 1.0f, 0.0f);
		glRasterPos2f(-0.9, 0.9);
		holeTitle = "GAME OVER - ";

		std::vector<int> winners = GetWinnerID();
		for (size_t i = 0; i < winners.size(); i++)
		{
			if (i > 0 && (i != winners.size() - 1))
				holeTitle += ", ";

			if ((i == winners.size() - 1) && i != 0)
				holeTitle += " and ";

			holeTitle += gCourse->players[winners[i]].name;

		}

		holeTitle += " WINS!";
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, holeTitle.c_str());
		pop2D();
	}
	else if (gluiScorecard != NULL) {
		/* blank screen; wait for user to close scorecard */
	}
	else
	{
		//draw all balls except current players and balls that have not moved from start
		for (int i = (gCourse->players.size() - 1); i >= 0; i--)
		{
			if (i == gCurrentPlayer) continue;
			if (gCourse->players[i].hadFirstShot && !gCourse->players[i].done)
			{
				glColor3f(gCourse->players[i].ball.rgb[0], gCourse->players[i].ball.rgb[1], gCourse->players[i].ball.rgb[2]);
				glPushMatrix();
				glTranslatef(gCourse->players[i].ball.position(0), (BALL_RADIUS / 2.0), gCourse->players[i].ball.position(1));
				glutWireSphere(gCourse->players[i].ball.radius, 12, 12);
				glPopMatrix();
			}
		}

		//draw the ball for current player
		if (!all_players_done())
		{
			glColor3f(gCourse->players[gCurrentPlayer].ball.rgb[0], gCourse->players[gCurrentPlayer].ball.rgb[1], gCourse->players[gCurrentPlayer].ball.rgb[2]);
			glPushMatrix();
			glTranslatef(gCourse->players[gCurrentPlayer].ball.position(0), (BALL_RADIUS / 2.0), gCourse->players[gCurrentPlayer].ball.position(1));
			glutWireSphere(gCourse->players[gCurrentPlayer].ball.radius, 12, 12);
			glPopMatrix();
		}

		glColor3f(0.3, 0.3, 0.3);
		//Draw Cup
		glPushMatrix();
		glTranslatef(gCourse->holes[gCurrentHole].cupPosition(0), (BALL_RADIUS / 2.0), gCourse->holes[gCurrentHole].cupPosition(1));
		glRotatef(90, 1.0f, 0.0f, 0.0f);
		glBegin(GL_POLYGON);
		for (double i = 0; i < TWO_PI; i += TWO_PI / 24)
		{
			glVertex3f(cos(i) * 0.04, sin(i) * 0.04, 0.0);

		}
		glEnd();
		glPopMatrix();

		//draw the cushions
		glColor3f(0.505, 0.356, 0.211);
		for (size_t i = 0; i < gCourse->holes[gCurrentHole].cushions.size(); i++)
		{
			glBegin(GL_LINE_LOOP);
			glVertex3f(gCourse->holes[gCurrentHole].cushions[i].vertices[0](0), 0.0, gCourse->holes[gCurrentHole].cushions[i].vertices[0](1));
			glVertex3f(gCourse->holes[gCurrentHole].cushions[i].vertices[0](0), 0.1, gCourse->holes[gCurrentHole].cushions[i].vertices[0](1));
			glVertex3f(gCourse->holes[gCurrentHole].cushions[i].vertices[1](0), 0.1, gCourse->holes[gCurrentHole].cushions[i].vertices[1](1));
			glVertex3f(gCourse->holes[gCurrentHole].cushions[i].vertices[1](0), 0.0, gCourse->holes[gCurrentHole].cushions[i].vertices[1](1));
			glEnd();
		}

		
		if (gNetworkMode == GOLF_NETWORK_CLIENT || gNetworkMode == GOLF_NETWORK_SERVER)
		{
			//draw the cue
			if (gDoCue && gCurrentPlayer == gNetworkMode)
			{
				glBegin(GL_LINES);
				float cuex = sin(gCueAngle) * gCuePower;
				float cuez = cos(gCueAngle) * gCuePower;
				glColor3f(1.0, 0.0, 0.0);
				glVertex3f(gCourse->players[gCurrentPlayer].ball.position(0), (BALL_RADIUS / 2.0f), gCourse->players[gCurrentPlayer].ball.position(1));
				glVertex3f((gCourse->players[gCurrentPlayer].ball.position(0) - cuex), (BALL_RADIUS / 2.0f), (gCourse->players[gCurrentPlayer].ball.position(1) - cuez));
				glColor3f(1.0, 1.0, 1.0);
				glEnd();
			}
		}
		else {
			//draw the cue
			if (gDoCue)
			{
				glBegin(GL_LINES);
				float cuex = sin(gCueAngle) * gCuePower;
				float cuez = cos(gCueAngle) * gCuePower;
				glColor3f(1.0, 0.0, 0.0);
				glVertex3f(gCourse->players[gCurrentPlayer].ball.position(0), (BALL_RADIUS / 2.0f), gCourse->players[gCurrentPlayer].ball.position(1));
				glVertex3f((gCourse->players[gCurrentPlayer].ball.position(0) - cuex), (BALL_RADIUS / 2.0f), (gCourse->players[gCurrentPlayer].ball.position(1) - cuez));
				glColor3f(1.0, 1.0, 1.0);
				glEnd();
			}
		}

		//draw hole number
		push2D();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glRasterPos2f(-0.9, 0.9);
		holeTitle = "Hole " + std::to_string(gCurrentHole + 1) + ": " + gCourse->holes[gCurrentHole].name;
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, holeTitle.c_str());
		pop2D();

		//draw scores for current player gui
		push2D();
		glColor3f(gCourse->players[gCurrentPlayer].ball.rgb[0], gCourse->players[gCurrentPlayer].ball.rgb[1], gCourse->players[gCurrentPlayer].ball.rgb[2]);
		glRasterPos2f(-0.9, -0.9);
		holeTitle = "Current Player: " + gCourse->players[gCurrentPlayer].name + ", SCORE: +" + std::to_string(gCourse->players[gCurrentPlayer].scores[gCurrentHole]);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, holeTitle.c_str());
		pop2D();
		

	}

	glutSetWindow(windowId);
	glFlush();
	glutSwapBuffers();
}

void SpecKeyboardFunc(int key, int x, int y) 
{
	switch(key)
	{
		case GLUT_KEY_LEFT:
		{
			gCueControl[0] = true;
			break;
		}
		case GLUT_KEY_RIGHT:
		{
			gCueControl[1] = true;
			break;
		}
		case GLUT_KEY_UP:
		{
			gCueControl[2] = true;
			break;
		}
		case GLUT_KEY_DOWN:
		{
			gCueControl[3] = true;
			break;
		}
	}

	glutSetWindow(windowId);
	glutPostRedisplay();
}

void SpecKeyboardUpFunc(int key, int x, int y) 
{
	switch(key)
	{
		case GLUT_KEY_LEFT:
		{
			gCueControl[0] = false;
			break;
		}
		case GLUT_KEY_RIGHT:
		{
			gCueControl[1] = false;
			break;
		}
		case GLUT_KEY_UP:
		{
			gCueControl[2] = false;
			break;
		}
		case GLUT_KEY_DOWN:
		{
			gCueControl[3] = false;
			break;
		}
	}
}

void KeyboardFunc(unsigned char key, int x, int y) 
{
	switch(key)
	{
		case(13):
		{ 
			if (gNetworkMode == GOLF_NETWORK_CLIENT || gNetworkMode == GOLF_NETWORK_SERVER)
			{
				if (gDoCue && gNetworkMode == gCurrentPlayer)// only allow user to chose direction if the current player is them on online
				{
					vec2d imp((-sin(gCueAngle) * gCuePower * gCueBallFactor),
						(-cos(gCueAngle) * gCuePower * gCueBallFactor));

					std::string msg = "1:" + std::to_string(imp(0)) + ":" + std::to_string(imp(1));

					//send impulse to other opponent
					if (gNetworkMode == GOLF_NETWORK_CLIENT)
						gClient.sendMessage(msg.c_str());
					else if (gNetworkMode == GOLF_NETWORK_SERVER)
						gServer.sendMessage(msg.c_str());

					gCourse->players[gCurrentPlayer].ball.ApplyImpulse(imp);
					IncrementScore();
					gBallHit = true;
					if (!gCourse->players[gCurrentPlayer].hadFirstShot)
					{
						gCourse->players[gCurrentPlayer].hadFirstShot = true;
					}


				}
			}
			else
			{
				if (gDoCue)
				{
					vec2d imp((-sin(gCueAngle) * gCuePower * gCueBallFactor),
						(-cos(gCueAngle) * gCuePower * gCueBallFactor));
					gCourse->players[gCurrentPlayer].ball.ApplyImpulse(imp);
					IncrementScore();
					gBallHit = true;
					if (!gCourse->players[gCurrentPlayer].hadFirstShot)
					{
						gCourse->players[gCurrentPlayer].hadFirstShot = true;
					}


				}
			}

			break;
		}
		case(32):
		{
			gCamRotate = false;
			break;
		}
		case('a'):
		{
			gCamL = true;
			break;
		}
		case('A'):
		{
			gCamL = true;
			break;
		}
		case('d'):
		{
			gCamR = true;
			break;
		}
		case('D'):
		{
			gCamR = true;
			break;
		}
		case('w'):
		{
			gCamU = true;
			break;
		}
		case('W'):
		{
			gCamU = true;
			break;
		}
		case('s'):
		{
			gCamD = true;
			break;
		}
		case('S'):
		{
			gCamD = true;
			break;
		}
		case('r'):
		{
			gCamZin = true;
			break;
		}
		case('R'):
		{
			gCamZin = true;
		}
		case('f'):
		{
			gCamZout = true;
			break;
		}
		case('F'):
		{
			gCamZout = true;
			break;
		}
	}

	glutSetWindow(windowId);
	glutPostRedisplay();

}

void KeyboardUpFunc(unsigned char key, int x, int y) 
{
	switch(key)
	{
		case(32):
			{
				gCamRotate = true;
				break;
			}
		case('a'):
			{
				gCamL = false;
				break;
			}
		case('A'):
		{
			gCamL = false;
			break;
		}
		case('d'):
			{
				gCamR = false;
				break;
			}
		case('D'):
		{
			gCamR = false;
			break;
		}
		case('w'):
		{
			gCamU = false;
			break;
		}
		case('W'):
		{
			gCamU = false;
			break;
		}
		case('s'):
		{
			gCamD = false;
			break;
		}
		case('S'):
		{
			gCamD = false;
			break;
		}
		case('r'):
			{
				gCamZin = false;
				break;
			}
		case('R'):
		{
			gCamZin = false;
			break;
		}
		case('f'):
		{
			gCamZout = false;
			break;
		}
		case('F'):
		{
			gCamZout = false;
			break;
		}
	}
}

void ChangeSize(int w, int h) {

	int gluix, gluiy, gluiw, gluih;

	GLUI_Master.get_viewport_area(&gluix, &gluiy, &gluiw, &gluih);
	glViewport(gluix, gluiy, gluiw, gluih);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, ((double)gluiw) / ((double)gluih), 0.1, 1000.0);

	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity();

	gluLookAt(gCamPos(0), gCamPos(1), gCamPos(2), gCamLookAt(0), gCamLookAt(1), gCamLookAt(2), 0.0f, 1.0f, 0.0f);
}

void UpdateScene(int ms) 
{
	if(gCourse->AnyBallsMoving()==false) gDoCue = true;
	else gDoCue = false;

	if(gDoCue)
	{
		if(gCueControl[0]) gCueAngle += ((gCueAngleSpeed * ms)/1000);
		if(gCueControl[1]) gCueAngle -= ((gCueAngleSpeed * ms)/1000);
		if (gCueAngle <0.0) gCueAngle += TWO_PI;
		if (gCueAngle >TWO_PI) gCueAngle -= TWO_PI;

		if(gCueControl[2]) gCuePower += ((gCuePowerSpeed * ms)/1000);
		if(gCueControl[3]) gCuePower -= ((gCuePowerSpeed * ms)/1000);
		if(gCuePower > gCuePowerMax) gCuePower = gCuePowerMax;
		if(gCuePower < gCuePowerMin) gCuePower = gCuePowerMin;
	}

	//go to next player once ball stops moving
	if (!gDoCue && (gCourse->players[gCurrentPlayer].ball.velocity.Magnitude() < 0.3) && !gCourse->players[gCurrentPlayer].done && gBallHit)
	{
		gBallHit = false;
		next_player();
		std::cout << "k" << std::endl;
		if (!gCourse->players[gCurrentPlayer].hadFirstShot)
		{
			gCourse->players[gCurrentPlayer].ball.position = gCourse->holes[gCurrentHole].startPosition;
		}
	}
	
	//show scorecard once all players are done
	if (all_players_done() && gBallHit)
	{
		gBallHit = false;
		show_scorecard();
	}

	DoCamera(ms);

	gCourse->Update(ms);

	glutSetWindow(windowId);
	glutPostRedisplay();

	glutTimerFunc(SIM_UPDATE_MS, UpdateScene, SIM_UPDATE_MS);
}

int _tmain(int argc, char** argv)
{

	glutInit(&argc, argv);

	gluiNewGame = NULL;

	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE| GLUT_RGBA);
	glutInitWindowPosition(0,0);
	glutInitWindowSize(1000,700);

	windowId = glutCreateWindow("David's Mini MiniGolf");
	GLUI_Master.set_glutIdleFunc(&RenderScene);
	glui = GLUI_Master.create_glui_subwindow(windowId, GLUI_SUBWINDOW_BOTTOM);
	glutDisplayFunc(RenderScene);
	glutTimerFunc(SIM_UPDATE_MS, UpdateScene, SIM_UPDATE_MS);
	GLUI_Master.set_glutReshapeFunc(ChangeSize);
	glui->set_main_gfx_window(windowId);

	GLUI_Master.set_glutKeyboardFunc(KeyboardFunc);
	GLUI_Master.set_glutSpecialFunc(SpecKeyboardFunc);

	glutIgnoreKeyRepeat(1);

	glutKeyboardUpFunc(KeyboardUpFunc);
	glutSpecialUpFunc(SpecKeyboardUpFunc);

	/* menu and quit buttons */
	GLUI_Panel *panel;
	panel = glui->add_panel("");
	panel->set_alignment(GLUI_ALIGN_CENTER);
	glui->add_button_to_panel(panel, "NEW LOCAL GAME", GOLF_NEW_GAME, &gluiCallBack);
	glui->add_column_to_panel(panel, true);
	glui->add_button_to_panel(panel, "HOST ONLINE 1v1", GOLF_HOSTGAME, &gluiCallBack);
	glui->add_column_to_panel(panel, true);
	glui->add_button_to_panel(panel, "JOIN ONLINE 1v1", GOLF_JOINGAME, &gluiCallBack);
	glui->add_column_to_panel(panel, true);
	glui->add_button_to_panel(panel, "HELP", GOLF_HELP, &gluiCallBack);

	glEnable(GL_DEPTH_TEST);
	glutMainLoop();
}
