#pragma once
#include <string>
#include "SFML/Network.hpp"
#include "SFML/System/Vector2.hpp"
class ClientProxy
{
public:
	ClientProxy(int _id, std::string _alias, int _x, int _y, sf::IpAddress _ip, unsigned short _port);
	~ClientProxy();

	int id;
	std::string alias;
	sf::Vector2f pos;
	sf::IpAddress ip;
	unsigned short port;
	int numPings;
};

