#include "Player.h"



Player::Player()
{
}

Player::Player(sf::Packet* pack)
{
	*pack >> id;
	*pack >> alias;
	AddBodyPositions(pack);
	speed = 2.0f;
}

Player::~Player()
{
}

void Player::AddBodyPositions(sf::Packet* pack)
{
	int numPos;
	*pack >> numPos;
	
	for (int i = 0; i < numPos; i++)
	{
		sf::Vector2f pos;
		*pack >> pos.x;
		*pack >> pos.y;
		bodyPositions.push_back(pos);
	}
}

void Player::UpdatePosition(sf::Packet* pack)
{
	int numPos;
	*pack >> numPos;
	int difference = numPos - (int)bodyPositions.size();
	if (difference > 0)
	{
		//add new positions, just initialized
		for (int i = 0; i < difference; i++)
		{
			sf::Vector2f pos(0.f, 0.f);
			bodyPositions.push_back(pos);
		}
	}

	for (int i = 0; i < bodyPositions.size(); i++)
	{
		*pack >> bodyPositions[i].x;
		*pack >> bodyPositions[i].y;
	}
}


void Player::UpdatePosition(sf::Vector2f move)
{
	for (int i = (int)bodyPositions.size() - 1; i > 0; i--)
	{
		bodyPositions[i] = bodyPositions[i - 1];
	}
	bodyPositions[0] += move;
}
