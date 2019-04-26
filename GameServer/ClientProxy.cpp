#include "ClientProxy.h"



ClientProxy::ClientProxy(int _id, std::string _alias, sf::IpAddress _ip, unsigned short _port)
{
	id = _id;
	alias = _alias;
	ip = _ip;
	port = _port;
	numPings = 0;
}


ClientProxy::~ClientProxy()
{
}
