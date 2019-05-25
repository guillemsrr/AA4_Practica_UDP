#include "Slither.h"



Slither::Slither(Player* _player)
{
	player = _player;
	UpdateRadius();
	bool flag = true;
	for each (sf::Vector2f  pos in player->bodyPositions)
	{
		CreateBodyCircle(pos, flag);
		if (flag)
			flag = false;
	}
}


Slither::~Slither()
{
}

void Slither::UpdateSlitherPosition()
{
	//std::
	for (int i = (int)bodyCircles.size(); i < (int)player->bodyPositions.size(); i++)
	{
		CreateBodyCircle(player->bodyPositions[i], i == 0);
	}

	//std::cout << "bodyCircles size: " << (int)bodyCircles.size() << std::endl;
	//std::cout << "(int)player->bodyPositions.size(): " << (int)player->bodyPositions.size() << std::endl;

	for(int i = 0; i< player->bodyPositions.size(); i++)
	{
		if ((int)bodyCircles.size() == (int)player->bodyPositions.size())
		{
			bodyCircles[i].setPosition(player->bodyPositions[i]);
		}
		else
		{
			std::cout << "ERROR: bodyCircles.size() != bodyPositions.size() " << std::endl;
			std::cout << "bodyCircles size: " << (int)bodyCircles.size() << std::endl;
			std::cout << "(int)player->bodyPositions.size(): " << (int)player->bodyPositions.size() << std::endl;
		}
	}
}

bool Slither::DetectCollision(Slither* enemy)
{
	return false;
}

void Slither::UpdateRadius()
{
	std::cout << "Radius: " << (int)player->bodyPositions.size() << std::endl;
	radius = 10.f + (int)player->bodyPositions.size() * radiusCirclesRelation;
}

void Slither::CreateBodyCircle(sf::Vector2f pos, bool isHead)
{
	sf::CircleShape shape(radius);
	sf::Color color;
	if (isHead)
		color = player->headColor;
	else
		color = player->bodyColor;

	shape.setFillColor(color);
	shape.setOrigin(shape.getRadius() / 2.f, shape.getRadius() / 2.f);
	shape.setPosition(pos);
	bodyCircles.push_back(shape);
}

bool Slither::IsPlayerDead()
{
	return player->dead;
}
