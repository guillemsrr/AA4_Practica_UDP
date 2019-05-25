#include "PlayerInfo.h"



PlayerInfo::PlayerInfo()
{
	appId = 0;
	currentFood = 0;
	dead = false;

}


PlayerInfo::PlayerInfo(int _id, std::string _alias, float _x, float _y, sf::Color _color)
{
	appId = _id;
	alias = _alias;
	currentFood = 0;

	sf::Vector2f headPos;
	headPos.x = _x;
	headPos.y = _y;
	bodyPositions.push_back(headPos);

	color = _color;
	dead = false;
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

void PlayerInfo::CreateBodyPosition()
{
	//std::cout << "create before" << std::endl;

	sf::Vector2f dir = bodyPositions[bodyPositions.size()-1] - bodyPositions[bodyPositions.size()-2];
	Normalize(dir);

	sf::Vector2f pos = bodyPositions[bodyPositions.size() - 1] + dir * separation;
	bodyPositions.push_back(pos);
	//std::cout << "create after" << std::endl;

}