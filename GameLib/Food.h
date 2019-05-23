#pragma once
#include <SFML/Graphics.hpp>

class Food
{
public:
	Food();
	Food(int _id, sf::Vector2i _pos);
	~Food();

	int id;
	sf::Vector2i position;
	sf::Color color;

private:

};

