#pragma once
#include <SFML/Graphics.hpp>

class Food
{
public:
	Food();
	Food(sf::Vector2f _pos);
	~Food();

	sf::Vector2f position;
	sf::Color color;

private:

};

