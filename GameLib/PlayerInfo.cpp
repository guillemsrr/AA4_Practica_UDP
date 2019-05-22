#include "PlayerInfo.h"



PlayerInfo::PlayerInfo()
{
	id = 0;
}


PlayerInfo::PlayerInfo(int _id, std::string _alias, float _x, float _y, sf::Color _color)
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

float PlayerInfo::Distance(sf::Vector2f v1, sf::Vector2f v2)
{
	sf::Vector2f v = v2 - v1;
	return sqrt(v.x*v.x + v.y*v.y);
}

void PlayerInfo::Normalize(sf::Vector2f& vec)
{
	sf::Vector2f zero = sf::Vector2f(0.f, 0.f);
	float dist = Distance(vec, zero);
	vec /= dist;
}