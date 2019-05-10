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
	void UpdatePosition(sf::Packet* pack);
	std::vector<sf::Vector2f> GetFuturePositions(sf::Packet* pack);
	void UpdatePosition(sf::Vector2f headPos);
	void UpdateTheRestOfPositions(int numPos, sf::Vector2f headPos, sf::Packet* pack);
	void InterpolateTo(std::vector<sf::Vector2f> finalBodyPositions, float percent);

private:
	std::vector<sf::Vector2f> startBodyPositions;
};

