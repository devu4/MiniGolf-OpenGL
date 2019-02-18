/*-----------------------------------------------------------
  Simulation Source File
  -----------------------------------------------------------*/
#include"stdafx.h"
#include"simulation.h"
#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<fstream>
#include<sstream>

/*-----------------------------------------------------------
  macros
  -----------------------------------------------------------*/
#define SMALL_VELOCITY		(0.1f)

/*-----------------------------------------------------------
  globals
  -----------------------------------------------------------*/

course *gCourse = new course;

float gCoeffRestitution = 0.5f;
float gCoeffFriction = 0.1f;
float gGravityAccn = 9.8f;


/*-----------------------------------------------------------
  cushion class members
  -----------------------------------------------------------*/
void cushion::MakeNormal(void)
{
	//can do this in 2d
	vec2d temp = vertices[1]-vertices[0];
	normal(0) = temp(1);
	normal(1) = -temp(0);
	normal.Normalise();
}

void cushion::MakeCentre(void)
{
	centre = vertices[0];
	centre += vertices[1];
	centre/=2.0;
}

/*-----------------------------------------------------------
  ball class members
  -----------------------------------------------------------*/
int ball::ballIndexCnt = 0;


void ball::Reset(void)
{
	//set velocity to zero
	velocity = 0.0;

	//randomise positions
	if ((gCurrentPlayer) == index)
	{
		position = gCourse->holes[gCurrentHole].startPosition;
	}
	else
	{
		position(1) = randomF();
		position(0) = randomF();
	}

}

void ball::ApplyImpulse(vec2d imp)
{
	velocity = imp;
}

void ball::ApplyFrictionForce(int ms)
{
	if(velocity.Magnitude()<=0.0) return;

	//accelaration is opposite to direction of motion
	vec2d accelaration = -velocity.Normalised();
	//friction force = constant * mg
	//F=Ma, so accelaration = force/mass = constant*g
	accelaration *= (gCoeffFriction * gGravityAccn);
	//integrate velocity : find change in velocity
	vec2d velocityChange = ((accelaration * ms)/1000.0f);
	//cap magnitude of change in velocity to remove integration errors
	if(velocityChange.Magnitude() > velocity.Magnitude()) velocity = 0.0;
	else velocity += velocityChange;
}

void ball::DoBallCollision(ball &b)
{
	if(HasHitBall(b)) HitBall(b);
}

void ball::DoPlaneCollision(const cushion &b)
{
	if(HasHitPlane(b)) HitPlane(b);
}

void ball::DoCupCollision()
{
	if (HasHitCup()) HitCup();
}

void ball::Update(int ms)
{
	//apply friction
	ApplyFrictionForce(ms);
	//integrate position
	position += ((velocity * ms)/1000.0f);
	//set small velocities to zero
	if(velocity.Magnitude()<SMALL_VELOCITY) velocity = 0.0;
}

bool ball::HasHitPlane(const cushion &c) const
{
	//if moving away from plane, cannot hit
	if(velocity.Dot(c.normal) >= 0.0) return false;

	//get rel position of ball
	vec2d relPos = position - c.vertices[0];
	double sep = relPos.Dot(c.normal);
	if(sep > radius) return false;

	//get distance between ball to both sides of the cushion
	double d1 = position.Dist(c.vertices[0]);
	double d2 = position.Dist(c.vertices[1]);

	//get length of cushion
	double lineLen = c.vertices[0].Dist(c.vertices[1]);
	
	if (d1 + d2 >= lineLen - radius && d1 + d2 <= lineLen + radius) {
		return true;
	}

	return false;
}

bool ball::HasHitCup() const
{
	//get distance between cup and ball
	double d1 = position.Dist(gCourse->holes[gCurrentHole].cupPosition);

	//if closer than randius of cup then it has hit
	if(d1 < 0.04)
		return true;

	return false;
}

bool ball::HasHitBall(const ball &b) const
{
	//work out relative position of ball from other ball,
	//distance between balls
	//and relative velocity
	vec2d relPosn = position - b.position;
	float dist = (float) relPosn.Magnitude();
	vec2d relPosnNorm = relPosn.Normalised();
	vec2d relVelocity = velocity - b.velocity;

	//if moving apart, cannot have hit
	if(relVelocity.Dot(relPosnNorm) >= 0.0) return false;
	//if distnce is more than sum of radii, have not hit
	if(dist > (radius+b.radius)) return false;
	return true;
}

void ball::HitPlane(const cushion &c)
{
	//reverse velocity component perpendicular to plane  
	double comp = velocity.Dot(c.normal) * (1.0+gCoeffRestitution);
	vec2d delta = -(c.normal * comp);
	velocity += delta; 
}

void ball::HitCup()
{
	if (velocity.Magnitude() < 2.2)
	{
		std::cout << "hitcup" << std::endl;
		velocity = vec2d(0, 0);
		gCourse->players[index].done = true;

		if (gCourse->players[gCurrentPlayer].done && !all_players_done())
		{
			next_player();

			if (!gCourse->players[gCurrentPlayer].hadFirstShot)
			{
				gCourse->players[gCurrentPlayer].ball.position = gCourse->holes[gCurrentHole].startPosition;
			}
		}
	}
	else 
	{
		
		vec2d relDir = (position - gCourse->holes[0].cupPosition);
		std::cout << relDir.Magnitude() << std::endl;
		if (relDir.Magnitude() > 0.002)
		{
			velocity = relDir.Normalised();
		}
	}
}

void ball::HitBall(ball &b)
{
	//find direction from other ball to this ball
	vec2d relDir = (position - b.position).Normalised();

	//split velocities into 2 parts:  one component perpendicular, and one parallel to 
	//the collision plane, for both balls
	//(NB the collision plane is defined by the point of contact and the contact normal)
	float perpV = (float)velocity.Dot(relDir);
	float perpV2 = (float)b.velocity.Dot(relDir);
	vec2d parallelV = velocity-(relDir*perpV);
	vec2d parallelV2 = b.velocity-(relDir*perpV2);
	
	//Calculate new perpendicluar components:
	float sumMass = mass + b.mass;
	float perpVNew = (float)((perpV*(mass-b.mass))/sumMass) + (float)((perpV2*(2.0*b.mass))/sumMass);
	float perpVNew2 = (float)((perpV2*(b.mass-mass))/sumMass) + (float)((perpV*(2.0*mass))/sumMass);
	
	//find new velocities by adding unchanged parallel component to new perpendicluar component
	velocity = parallelV + (relDir*perpVNew);
	b.velocity = parallelV2 + (relDir*perpVNew2);

}

/*-----------------------------------------------------------
  table class members
  -----------------------------------------------------------*/
void hole::SetupCushions(void)
{

	for(size_t i=0;i<cushions.size();i++)
	{
		cushions[i].MakeCentre();
		cushions[i].MakeNormal();
	}
}

void course::Update(int ms)
{
	//check for collisions for each ball
	for(size_t i=0;i<players.size();i++) 
	{
		for(size_t j=0;j<holes[gCurrentHole].cushions.size();j++)
		{
			players[i].ball.DoPlaneCollision(holes[gCurrentHole].cushions[j]);
		}

		for(size_t j=(i+1);j<players.size();j++)
		{
			if(players[i].hadFirstShot && players[j].hadFirstShot && !players[i].done && !players[j].done)
				players[i].ball.DoBallCollision(players[j].ball);
		}

		//only allows ball that isn't done to collide with cup.
		if(!players[i].done)
			players[i].ball.DoCupCollision();
	}
	
	//update all balls
	for(size_t i=0;i<players.size();i++) players[i].ball.Update(ms);;

	//make some new particles
	//vec3 pos(0.0,BALL_RADIUS,0.0);
	//parts.AddParticle(pos);
}

bool course::AnyBallsMoving(void) const
{
	//return true if any ball has a non-zero velocity
	for(size_t i=0;i<players.size();i++)
	{
		if(players[i].ball.velocity(0)!=0.0) return true;
		if(players[i].ball.velocity(1)!=0.0) return true;
	}
	return false;
}



//function to load game from course file
void course::Load_Game(std::string filename)
{
	std::ifstream levelFile;
	levelFile.open(filename);
	std::string line;
	std::getline(levelFile, line);
	gCourse->name = line;

	std::cout << gCourse->name << std::endl;
	
	while (std::getline(levelFile, line))
	{
		if (line == "begin_hole")
		{
			//get name
			std::getline(levelFile, line);
			hole tempTable;
			gCourse->holes.push_back(tempTable);
			int id = gCourse->holes.size() - 1;
			gCourse->holes[id].name = line;

			while (std::getline(levelFile, line))
			{
				if (line == "end_hole")
				{
					break;
				}

				std::stringstream ss(line);
				std::string s;

				while (std::getline(ss, s, ' '))
				{
					if (s == "cushion")
					{
						cushion tempCushion;
						double x, y;
						std::getline(ss, s, ' ');
						x = std::atof(s.c_str());
						std::getline(ss, s, ' ');
						y = std::atof(s.c_str());
						tempCushion.vertices[0](0)= x;
						tempCushion.vertices[0](1) = y;
						std::getline(ss, s, ' ');
						x = std::atof(s.c_str());
						std::getline(ss, s, ' ');
						y = std::atof(s.c_str());
						tempCushion.vertices[1](0) = x;
						tempCushion.vertices[1](1) = y;
						gCourse->holes[id].cushions.push_back(tempCushion);
					}
					if (s == "cup")
					{
						std::getline(ss, s, ' ');
						double x, y;
						x = std::atof(s.c_str());
						std::getline(ss, s, ' ');
						y = std::atof(s.c_str());
						gCourse->holes[id].cupPosition(0) = x;
						gCourse->holes[id].cupPosition(1) = y;
					}
					if (s == "start")
					{
						std::getline(ss, s, ' ');
						double x, y;
						x = std::atof(s.c_str());
						std::getline(ss, s, ' ');
						y = std::atof(s.c_str());
						gCourse->holes[id].startPosition(0) = x;
						gCourse->holes[id].startPosition(1) = y;
					}
				}

			}
		}

	}

}

float ball::randomF() {

	//create random number
	std::random_device rd;
	std::default_random_engine generator(rd()); // rd() provides a random seed
	std::uniform_real_distribution<float> distribution(0.00, 1.00);
	return (float)distribution(generator);
}