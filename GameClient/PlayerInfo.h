#pragma once
#include <string>
#include "SFML/Network.hpp"
#include "SFML/System/Vector2.hpp"
class PlayerInfo
{
public:
	PlayerInfo();
	PlayerInfo(int id, std::string alias, int x, int y);
	~PlayerInfo();

	int id;
	std::string alias;
	sf::Vector2i pos;
};

