#pragma once
#include <string>
#include "SFML/Network.hpp"
#include "SFML/System/Vector2.hpp"
#include <PlayerInfo.h>


class ClientProxy: public PlayerInfo
{
public:
	ClientProxy::ClientProxy(int _id, std::string _alias, sf::IpAddress _ip, unsigned short _port, sf::Vector2i headPos);
	~ClientProxy();

	sf::IpAddress ip;
	unsigned short port;
	int numPings;

	sf::Vector2i accumMovement;

	void CreateInitialBodyPositions(sf::Vector2i headPos);
	void PutBodyPositions(sf::Packet* pack);
	void AddDataToPacket(sf::Packet* pack);

	sf::Vector2i SumToHeadPosition();
	void CreateBodyPosition();
	void UpdatePosition(sf::Vector2i headPos);
	int lastIdMove;

private:
	const int separation = 10;
};

