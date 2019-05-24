#pragma once
#include <Food.h>

class FoodBall :
	public Food
{
public:
	FoodBall(sf::Vector2f pos);
	~FoodBall();

	sf::CircleShape circleShape;
	float radius = 5.f;
};

