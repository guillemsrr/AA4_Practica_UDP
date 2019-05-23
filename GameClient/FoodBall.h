#pragma once
#include <Food.h>

class FoodBall :
	public Food
{
public:
	FoodBall(int _id, sf::Vector2i pos);
	~FoodBall();

	sf::CircleShape circleShape;
	int radius = 5;
};

