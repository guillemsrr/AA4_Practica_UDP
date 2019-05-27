#include "ClientProxy.h"


ClientProxy::ClientProxy(int _id, std::string _alias, sf::IpAddress _ip, unsigned short _port, sf::Vector2f headPos) :
	PlayerInfo(_id, _alias, headPos.x, headPos.y, SkinColors::DEFAULT)
{
	queryId = 0;
	ip = _ip;
	port = _port;
	numPings = 0;
	CreateInitialBodyPositions(headPos);
	accumMovement = sf::Vector2f(0.f,0.f);
	isLogging = false;
	isLogged = false;
	uState = UserState::NOTLOGGEDIN;
	mmr = 0;
	idSesionActual = 0;
	totalKills = 0;
	maxLongitud = 0;
	matchID = -1;
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
	sf::Vector2f pos = headPos + direction * separation;
	bodyPositions.push_back(pos);

	sf::Vector2f secondPos = pos + direction * separation;
	bodyPositions.push_back(secondPos);
}

void ClientProxy::PutBodyPositions(sf::Packet* pack)
{
	*pack << (int)bodyPositions.size();
	for each (sf::Vector2f pos in bodyPositions)
	{
		*pack << (int)(pos.x*1000);
		*pack << (int)(pos.y*1000);
	}
}

void ClientProxy::AddDataToPacket(sf::Packet* pack)
{
	*pack << appId;
	*pack << alias;
	*pack << static_cast<int>(skinColor);
	PutBodyPositions(pack);
}

sf::Vector2f ClientProxy::SumToHeadPosition()
{
	return bodyPositions[0] + accumMovement;
}

void ClientProxy::EatBall()
{
	if (currentFood == foodToGrow - 1)
	{
		CreateBodyPosition(false, sf::Vector2f(0.0f, 0.0f));
	}
	currentFood = (currentFood + 1) % foodToGrow;
}

void ClientProxy::UpdatePosition(sf::Vector2f headPos)
{
	bodyPositions[0] = headPos;

	for (int i = 1; i < (int)bodyPositions.size() ; i++)
	{
		sf::Vector2f dir = bodyPositions[i] - bodyPositions[i - 1];
		Normalize(dir);
		bodyPositions[i] = bodyPositions[i - 1] + dir * separation;
	}

	/*for (int i = (int)bodyPositions.size() - 1; i > 0; i--)
	{
		bodyPositions[i] = bodyPositions[i - 1];
	}
	bodyPositions[0] = headPos;*/

	accumMovement = sf::Vector2f(0,0);
}