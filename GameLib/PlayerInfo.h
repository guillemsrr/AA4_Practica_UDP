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

	float Distance(sf::Vector2f v1, sf::Vector2f v2);
	void Normalize(sf::Vector2f& vec);
	void CreateBodyPosition();
	const float separation = 10.0f;

	int currentFood;
	const int foodToGrow = 3;
	bool dead;

};

