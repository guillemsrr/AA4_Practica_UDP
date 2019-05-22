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
	PlayerInfo(int id, std::string alias, float x, float y, sf::Color _color);
	~PlayerInfo();

	int id;
	std::string alias;
	std::vector<sf::Vector2f> bodyPositions;
	sf::Color color;
	sf::Vector2f direction;

	float PlayerInfo::Distance(sf::Vector2f v1, sf::Vector2f v2);
	void PlayerInfo::Normalize(sf::Vector2f& vec);
};

