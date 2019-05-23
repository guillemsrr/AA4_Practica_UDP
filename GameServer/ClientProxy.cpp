#include "ClientProxy.h"


ClientProxy::ClientProxy(int _id, std::string _alias, sf::IpAddress _ip, unsigned short _port, sf::Vector2i headPos)
{
	id = _id;
	alias = _alias;
	ip = _ip;
	port = _port;
	numPings = 0;
	CreateInitialBodyPositions(headPos);
	accumMovement = sf::Vector2i(0,0);
}

ClientProxy::~ClientProxy()
{
}

void ClientProxy::CreateInitialBodyPositions(sf::Vector2i headPos)
{
	bodyPositions.push_back(headPos);

	//create random direction
	direction = sf::Vector2i(1.f,0.f);

	//make two more bodies:
	sf::Vector2i pos = headPos + direction * separation;
	bodyPositions.push_back(pos);

	sf::Vector2i secondPos = pos + direction * separation;
	bodyPositions.push_back(secondPos);
}

void ClientProxy::PutBodyPositions(sf::Packet* pack)
{
	*pack << (int)bodyPositions.size();
	for each (sf::Vector2i pos in bodyPositions)
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

sf::Vector2i ClientProxy::SumToHeadPosition()
{
	return bodyPositions[0] + accumMovement;
}

void ClientProxy::CreateBodyPosition()
{
	sf::Vector2i pos = bodyPositions.back();
	bodyPositions.push_back(pos);
}

void ClientProxy::UpdatePosition(sf::Vector2i headPos)
{
	bodyPositions[0] = headPos;

	for (int i = 1; i < (int)bodyPositions.size() ; i++)
	{
		sf::Vector2i dir = bodyPositions[i] - bodyPositions[i - 1];
		Normalize(dir);
		bodyPositions[i] = bodyPositions[i - 1] + dir * separation;
	}

	/*for (int i = (int)bodyPositions.size() - 1; i > 0; i--)
	{
		bodyPositions[i] = bodyPositions[i - 1];
	}
	bodyPositions[0] = headPos;*/

	accumMovement = sf::Vector2i(0,0);
}