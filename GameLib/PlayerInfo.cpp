#include "PlayerInfo.h"



PlayerInfo::PlayerInfo()
{
	id = 0;
}


PlayerInfo::PlayerInfo(int _id, std::string _alias, int _x, int _y, sf::Color _color)
{
	id = _id;
	alias = _alias;

	sf::Vector2f headPos;
	headPos.x = _x;
	headPos.y = _y;
	bodyPositions.push_back(headPos);

	color = _color;

}

PlayerInfo::~PlayerInfo()
{
}