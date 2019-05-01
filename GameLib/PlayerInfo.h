#pragma once
#include <string>
#include "SFML/Network.hpp"
#include <SFML/Graphics.hpp>

class PlayerInfo
{
public:
	PlayerInfo();
	PlayerInfo(int id, std::string alias, int x, int y, sf::Color _color);
	~PlayerInfo();

	int id;
	std::string alias;
	sf::Vector2f pos;
	sf::Color color;
	int numCircles;
};

