#pragma once
#include <string>
#include "SFML/Network.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include "Constants.h"

class PlayerInfo
{
public:
	PlayerInfo();
	PlayerInfo(int id, std::string alias, int x, int y, sf::Color _color);
	~PlayerInfo();

	int id;
	std::string alias;
	std::vector<sf::Vector2i> bodyPositions;
	sf::Color color;
	sf::Vector2i direction;

	int PlayerInfo::Distance(sf::Vector2i v1, sf::Vector2i v2);
	void PlayerInfo::Normalize(sf::Vector2i& vec);
};

