#include "ClientProxy.h"



ClientProxy::ClientProxy(int _id, std::string _alias, int _x, int _y, sf::IpAddress _ip, unsigned short _port)
{
	id = _id;
	alias = _alias;
	pos.x = _x;
	pos.y = _y;
	ip = _ip;
	port = _port;
	numPings = 0;
}


ClientProxy::~ClientProxy()
{
}
