#pragma once
#include <PlayerInfo.h>

class Player :
	public PlayerInfo
{
public:
	Player();
	Player(sf::Packet* pack);
	~Player();

	float speed;

	void AddBodyPositions(sf::Packet* pack);
	void Player::UpdatePosition(sf::Packet* pack);
	void Player::UpdatePosition(sf::Vector2f headPos);

};

