#pragma once
#include <string>
#include "SFML/Network.hpp"
class ClientProxy
{
public:
	ClientProxy(int _id, std::string _alias, sf::IpAddress _ip, unsigned short _port);
	~ClientProxy();

	int id;
	std::string alias;
	//sf::Vector2 pos;
	sf::IpAddress ip;
	unsigned short port;
	int numPings;
};

