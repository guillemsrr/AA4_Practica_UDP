#include "PlayerInfo.h"



PlayerInfo::PlayerInfo()
{
	appId = 0;
	currentFood = 0;
	dead = false;

}


PlayerInfo::PlayerInfo(int _id, std::string _alias, float _x, float _y, SkinColors _skinColor)
{
	appId = _id;
	alias = _alias;
	currentFood = 0;

	/*sf::Vector2f headPos;
	headPos.x = _x;
	headPos.y = _y;
	bodyPositions.push_back(headPos);*/

	SetPlayerColor(_skinColor);
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

void PlayerInfo::SetPlayerColor(SkinColors _skinColor)
{
	skinColor = _skinColor;
	switch (skinColor)
	{
	case SkinColors::RED:
		headColor = BTN_RED_CLK_COLOR;
		bodyColor = BTN_RED_DEF_COLOR;
		break;
	case SkinColors::ORANGE:
		headColor = BTN_ORG_CLK_COLOR;
		bodyColor = BTN_ORG_DEF_COLOR;
		break;
	case SkinColors::YELLOW:
		headColor = BTN_YLW_CLK_COLOR;
		bodyColor = BTN_YLW_DEF_COLOR;
		break;
	case SkinColors::GREEN:
		headColor = BTN_GRN_CLK_COLOR;
		bodyColor = BTN_GRN_DEF_COLOR;
		break;
	case SkinColors::TURQUOISE:
		headColor = BTN_TRQ_CLK_COLOR;
		bodyColor = BTN_TRQ_DEF_COLOR;
		break;
	case SkinColors::LIGHTBLUE:
		headColor = BTN_CYN_CLK_COLOR;
		bodyColor = BTN_CYN_DEF_COLOR;
		break;
	case SkinColors::INDIGO:
		headColor = BTN_IDG_CLK_COLOR;
		bodyColor = BTN_IDG_DEF_COLOR;
		break;
	case SkinColors::VIOLET:
		headColor = BTN_VLT_CLK_COLOR;
		bodyColor = BTN_VLT_DEF_COLOR;
		break;
	case SkinColors::DEFAULT:
		headColor = BTN_CLK_COLOR;
		bodyColor = BTN_DEF_COLOR;
		break;
	}
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