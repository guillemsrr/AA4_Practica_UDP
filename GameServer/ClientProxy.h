#pragma once
#include <string>
#include "SFML/Network.hpp"
#include "SFML/System/Vector2.hpp"
#include <PlayerInfo.h>

class ClientProxy: public PlayerInfo
{
public:
	ClientProxy(int _id, std::string _alias, int _x, int _y, sf::IpAddress _ip, unsigned short _port);
	~ClientProxy();

	sf::IpAddress ip;
	unsigned short port;
	int numPings;
};

