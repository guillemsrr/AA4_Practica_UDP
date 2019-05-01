#pragma once
#include <SFML/Graphics.hpp>

class Slither
{
public:
	Slither::Slither(sf::Vector2f _headPos, sf::Color _color, int _numCircles, sf::Vector2f dir = sf::Vector2f(1.f, 0.f));
	~Slither();

	sf::Vector2f headPos;
	sf::Vector2f direction;
	sf::Color color;
	int numCircles;
	float radius;

private:
	const float circlesDistance = 1.f;
};

