/*-----------------------------------------------------------
  Simulation Header File
  -----------------------------------------------------------*/
#pragma once
#include"vecmath.h"
#include <string>
#include <vector>
#include <random>

/*-----------------------------------------------------------
  Macros
  -----------------------------------------------------------*/
#define TABLE_X			(0.6f) 
#define TABLE_Z			(1.2f)
#define BALL_RADIUS		(0.03f)
#define BALL_MASS		(0.1f)
#define TWO_PI			(6.2832f)
#define	SIM_UPDATE_MS	(10)
#define GOLF_NETWORK_CLIENT 1
#define GOLF_NETWORK_SERVER 0
#define GOLF_NETWORK_NONE -1
//#define NUM_BALLS		(1)		
//#define NUM_CUSHIONS	(5)		

/*-----------------------------------------------------------
  cushion class
  -----------------------------------------------------------*/
class cushion
{
public:
	vec2d	vertices[2]; //2d
	vec2d	centre;
	vec2d	normal;

	void MakeNormal(void);
	void MakeCentre(void);
};
/*-----------------------------------------------------------
  ball class
  -----------------------------------------------------------*/

class ball
{
	static int ballIndexCnt;
public:
	vec2d	position;
	vec2d	velocity;
	float	radius;
	float	mass;
	int		index;
	float rgb[3];

	ball(): position(0.0), velocity(0.0), radius(BALL_RADIUS), 
		mass(BALL_MASS) {index = ballIndexCnt++; Reset();
		rgb[0] = randomF(); rgb[1] = randomF(); rgb[2] = randomF();
	}
	
	static void resetIndex() { ballIndexCnt = 0; };
	void Reset(void);
	void ApplyImpulse(vec2d imp);
	void ApplyFrictionForce(int ms);
	void DoPlaneCollision(const cushion &c);
	void DoBallCollision(ball &b);
	void DoCupCollision();
	void Update(int ms);
	
	bool HasHitPlane(const cushion &c) const;
	bool HasHitBall(const ball &b) const;
	bool HasHitCup() const;

	void HitPlane(const cushion &c);
	void HitBall(ball &b);
	void HitCup();
	float randomF();
};

/*-----------------------------------------------------------
Player class
-----------------------------------------------------------*/
class Player
{
public:
	std::string name;
	std::vector<int> scores;
	bool done;
	ball ball;
	bool hadFirstShot;
	Player()
	{
		name = "Player " + std::to_string(ball.index+1);
		done = false;
		hadFirstShot = false;
	};

	~Player() {};
};

/*-----------------------------------------------------------
  hole class
  -----------------------------------------------------------*/
class hole
{
public:
	std::vector<cushion> cushions;
	vec2d startPosition = vec2d(0, 0);
	vec2d cupPosition = vec2d(0, 0);
	std::string name;

	void SetupCushions(void);
};

class course
{
public:
	std::vector<hole> holes;
	std::vector<Player> players;
	std::string name; 

	void Load_Game(std::string filename);
	bool course::AnyBallsMoving(void) const;
	void Update(int ms);
};

/*-----------------------------------------------------------
global table
-----------------------------------------------------------*/
extern course *gCourse;
extern int gCurrentHole;
extern int gCurrentPlayer;
extern int gNetworkMode;
extern std::string myName, opponentName;
extern std::string newFilename;
extern bool gGameEnd;
extern bool gBallHit;

static void next_player() {
	do {

		gCurrentPlayer = (gCurrentPlayer + 1) % gCourse->players.size();

	} while (gCourse->players[gCurrentPlayer].done);//if next player allread done then do +1 again
	//gluiCurrentPlayer->set_text(players[currentPlayer]->name);
}

static bool all_players_done() {

	for (size_t i = 0; i<gCourse->players.size(); i++) {
		if (!gCourse->players[i].done) {
			return false;
		}
	}
	return true;
}

static void initScores()
{
	for(size_t j=0; j < gCourse->players.size(); j++)
		for (size_t i=0; i < gCourse->holes.size(); i++)
			gCourse->players[j].scores.push_back(-1);
}
static void resetPlayers()
{
	gCurrentPlayer = 0;

	for (size_t i = 0; i < gCourse->players.size(); i++)
	{
		gCourse->players[i].ball.Reset();
		gCourse->players[i].hadFirstShot = false;
		gCourse->players[i].done = false;
	}
}