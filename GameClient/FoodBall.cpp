#include "FoodBall.h"



FoodBall::FoodBall(sf::Vector2f _pos)
{
	position = _pos;

	circleShape =  sf::CircleShape(radius);
	circleShape.setFillColor(sf::Color::Blue);
	circleShape.setOrigin(circleShape.getRadius() / 2.f, circleShape.getRadius() / 2.f);
	circleShape.setPosition(position.x, position.y);
}

FoodBall::~FoodBall()
{
}
