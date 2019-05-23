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
	for(int i = 0; i< bodyCircles.size(); i++)
	{
		bodyCircles[i].setPosition(player->bodyPositions[i]);
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
