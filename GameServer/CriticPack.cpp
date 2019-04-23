#include "CriticPack.h"



CriticPack::CriticPack(int _id, sf::Packet _pack, sf::IpAddress _ip, unsigned short _port)
{
	id = _id;
	pack = _pack;
	ip = _ip;
	port = _port;
}


CriticPack::~CriticPack()
{
}
