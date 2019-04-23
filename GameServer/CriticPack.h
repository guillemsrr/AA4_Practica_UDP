#pragma once
#include <string>
#include "SFML/Network.hpp"

class CriticPack
{
public:
	CriticPack(int _id, sf::Packet _pack, sf::IpAddress _ip, unsigned short _port);
	~CriticPack();

	int id;
	sf::Packet pack;
	sf::IpAddress ip;
	unsigned short port;
};

