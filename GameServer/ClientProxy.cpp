#include "ClientProxy.h"


ClientProxy::ClientProxy(int _id, std::string _alias, sf::IpAddress _ip, unsigned short _port, sf::Vector2f headPos)
{
	id = _id;
	alias = _alias;
	ip = _ip;
	port = _port;
	numPings = 0;
	CreateInitialBodyPositions(headPos);
}

ClientProxy::~ClientProxy()
{
}

void ClientProxy::CreateInitialBodyPositions(sf::Vector2f headPos)
{
	bodyPositions.push_back(headPos);

	//create random direction
	direction = sf::Vector2f(1.f,0.f);

	//make two more bodies:
	sf::Vector2f pos = headPos + direction * speed;
	bodyPositions.push_back(pos);

	sf::Vector2f secondPos = pos + direction * speed;
	bodyPositions.push_back(secondPos);
}

void ClientProxy::PutBodyPositions(sf::Packet* pack)
{
	*pack << (int)bodyPositions.size();
	for each (sf::Vector2f pos in bodyPositions)
	{
		*pack << pos.x;
		*pack << pos.y;
	}
}

void ClientProxy::AddDataToPacket(sf::Packet* pack)
{
	*pack << id;
	*pack << alias;
	PutBodyPositions(pack);
}

sf::Vector2f ClientProxy::SumToHeadPosition(sf::Vector2f movement)
{
	return bodyPositions[0] + movement;
}

void ClientProxy::CreateBodyPosition()
{
	sf::Vector2f pos = bodyPositions.back();
	bodyPositions.push_back(pos);
}

void ClientProxy::UpdatePosition(sf::Vector2f headPos)
{
	for (int i = (int)bodyPositions.size() - 1; i > 0; i--)
	{
		bodyPositions[i] = bodyPositions[i - 1];
	}
	bodyPositions[0] = headPos;
}