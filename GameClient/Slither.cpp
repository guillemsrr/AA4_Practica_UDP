#include "Slither.h"



Slither::Slither(Player* _player)
{
	player = _player;
	UpdateRadius();
	for each (sf::Vector2f  pos in player->bodyPositions)
	{
		CreateBodyCircle(pos);
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
		CreateBodyCircle(player->bodyPositions[i]);
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
	radius = 10.f + player->bodyPositions.size() * radiusCirclesRelation;
}

void Slither::CreateBodyCircle(sf::Vector2f pos)
{
	sf::CircleShape shape(radius);
	shape.setFillColor(player->color);
	shape.setOrigin(shape.getRadius() / 2.f, shape.getRadius() / 2.f);
	shape.setPosition(pos);
	bodyCircles.push_back(shape);
}

bool Slither::IsPlayerDead()
{
	return player->dead;
}
