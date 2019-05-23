#pragma once
#include <SFML/Graphics.hpp>

class Food
{
public:
	Food();
	Food(int _id, sf::Vector2f _pos);
	~Food();

	int id;
	sf::Vector2f position;
	sf::Color color;

private:

};

