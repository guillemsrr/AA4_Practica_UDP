#include "PlayerInfo.h"



PlayerInfo::PlayerInfo()
{
}


PlayerInfo::PlayerInfo(int _id, std::string _alias, int _x, int _y, sf::Color _color)
{
	id = _id;
	alias = _alias;
	pos.x = _x;
	pos.y = _y;
	numCircles = 3;
	color = _color;
}

PlayerInfo::~PlayerInfo()
{
}
