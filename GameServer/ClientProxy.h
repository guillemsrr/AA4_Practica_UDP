#pragma once
#include <string>
#include "SFML/Network.hpp"
#include "SFML/System/Vector2.hpp"
#include <PlayerInfo.h>


enum class UserState {NOTLOGGEDIN, LOBBY, PLAY};
class ClientProxy: public PlayerInfo
{
public:
	ClientProxy::ClientProxy(int _id, std::string _alias, sf::IpAddress _ip, unsigned short _port, sf::Vector2f headPos);
	~ClientProxy();

	sf::IpAddress ip;
	unsigned short port;
	int numPings;
	bool isLogging;
	bool isLogged;
	UserState uState;
	float mmr;

	sf::Vector2f accumMovement;

	void CreateInitialBodyPositions(sf::Vector2f headPos);
	void PutBodyPositions(sf::Packet* pack);
	void AddDataToPacket(sf::Packet* pack);

	sf::Vector2f SumToHeadPosition();
	void EatBall();
	void UpdatePosition(sf::Vector2f headPos);
	int lastIdMove;
};

