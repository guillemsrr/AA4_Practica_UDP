#include "Slither.h"



Slither::Slither(sf::Vector2f _headPos, sf::Color _color, int _numCircles, sf::Vector2f dir = sf::Vector2f(1.f, 0.f))
{
	headPos = _headPos;
	color = _color;
	numCircles = _numCircles;
	direction = dir;
}


Slither::~Slither()
{
}
