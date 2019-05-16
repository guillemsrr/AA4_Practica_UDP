#pragma once
#include <SFML/Graphics.hpp>

class Food
{
public:
	Food();
	~Food();

	int id;
	sf::Vector2f position;
	sf::Color color;
	float radius;

private:

};

