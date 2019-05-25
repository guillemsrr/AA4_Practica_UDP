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
	bool IsPlayerDead();

private:
	Player* player;
	float radius;
	const float radiusCirclesRelation = 1.f;
	void UpdateRadius();
	void CreateBodyCircle(sf::Vector2f pos, bool isHead);
};

