#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Player.h"

class Slither
{
public:
	Slither::Slither(Player* player);
	~Slither();

	void UpdateSlitherPosition();
	bool DetectCollision(Slither* enemy);

	std::vector<sf::CircleShape> bodyCircles;

private:
	Player* player;
	int radius;
	const int radiusCirclesRelation = 1;
	void UpdateRadius();
	void CreateBodyCircle(sf::Vector2i pos);
};

