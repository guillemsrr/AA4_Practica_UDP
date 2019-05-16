#include "FoodBall.h"



FoodBall::FoodBall(int _id, sf::Vector2f _pos)
{
	id = _id;
	position = _pos;

	circleShape =  sf::CircleShape(radius);
	circleShape.setFillColor(sf::Color::Blue);
	circleShape.setPosition(position.x, position.y);
}

FoodBall::~FoodBall()
{
}
