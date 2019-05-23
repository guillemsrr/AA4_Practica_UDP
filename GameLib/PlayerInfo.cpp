#include "PlayerInfo.h"



PlayerInfo::PlayerInfo()
{
	id = 0;
}


PlayerInfo::PlayerInfo(int _id, std::string _alias, int _x, int _y, sf::Color _color)
{
	id = _id;
	alias = _alias;

	sf::Vector2i headPos;
	headPos.x = _x;
	headPos.y = _y;
	bodyPositions.push_back(headPos);

	color = _color;

}

PlayerInfo::~PlayerInfo()
{
}

int PlayerInfo::Distance(sf::Vector2i v1, sf::Vector2i v2)
{
	sf::Vector2i v = v2 - v1;
	return sqrt(v.x*v.x + v.y*v.y);
}

void PlayerInfo::Normalize(sf::Vector2i& vec)
{
	sf::Vector2i zero = sf::Vector2i(0.f, 0.f);
	int dist = Distance(vec, zero);
	vec /= dist;
}