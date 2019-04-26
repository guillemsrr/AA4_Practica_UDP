#pragma once
#include <string>
#include "SFML/Network.hpp"
class PlayerInfo
{
public:
	PlayerInfo();
	PlayerInfo(int id);
	~PlayerInfo();

	int id;
	//sf::Vector2 pos;
};

