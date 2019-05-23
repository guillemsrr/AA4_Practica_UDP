#pragma once
#include <PlayerInfo.h>

class Player :
	public PlayerInfo
{
public:
	Player();
	Player(sf::Packet* pack);
	~Player();

	int speed;

	void AddBodyPositions(sf::Packet* pack);
	void UpdatePosition(sf::Packet* pack);
	std::vector<sf::Vector2i> GetFuturePositions(sf::Packet* pack);
	void UpdatePosition(sf::Vector2i headPos);
	void UpdateTheRestOfPositions(int numPos, sf::Vector2i headPos, sf::Packet* pack);
	bool InterpolateTo(std::vector<sf::Vector2i> finalBodyPositions, int percent);

private:
	std::vector<sf::Vector2i> startBodyPositions;
	int Magnitude(sf::Vector2i headPos);

};

