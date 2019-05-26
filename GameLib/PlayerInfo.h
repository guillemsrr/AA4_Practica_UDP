#pragma once
#include <string>
#include "SFML/Network.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include "Constants.h"
#include "UIButton.h"

enum class SkinColors
{
	DEFAULT,
	RED,
	ORANGE,
	YELLOW,
	GREEN,
	TURQUOISE,
	LIGHTBLUE,
	INDIGO,
	VIOLET
};

class PlayerInfo
{
public:
	PlayerInfo();
	PlayerInfo(int id, std::string alias, float x, float y, SkinColors _skinColor);
	~PlayerInfo();

	int appId;
	int queryId;
	std::string alias;
	std::vector<sf::Vector2f> bodyPositions;
	sf::Color bodyColor;
	sf::Color headColor;
	sf::Vector2f direction;
	SkinColors skinColor;

	float Distance(sf::Vector2f v1, sf::Vector2f v2);
	void Normalize(sf::Vector2f& vec);
	void SetPlayerColor(SkinColors _skinColor);
	void CreateBodyPosition(bool overridePos, sf::Vector2f position);
	const float separation = 10.0f;

	int currentFood;
	const int foodToGrow = 3;
	bool dead;

};

