#pragma once
#include <Food.h>

class FoodBall :
	public Food
{
public:
	FoodBall();
	~FoodBall();

	sf::CircleShape circleShape;
};

