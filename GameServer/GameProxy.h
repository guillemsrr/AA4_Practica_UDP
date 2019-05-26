#pragma once
#include <vector>
#include <Food.h>
#include "ClientProxy.h"
#include <Constants.h>

class GameProxy
{
public:
	GameProxy();
	GameProxy(int _id, std::vector<int> _arr);
	~GameProxy();

	int id;
	std::vector<int> idPlayersInGame;
	std::vector<Food*> foodVector;
	const int maxFood = 100;


	void InitializeFood();
	void FoodCollisionCheck(std::map<int, ClientProxy*> &clientProxies, int pID, std::vector<sf::Vector2f> playerPositions, float playerBodyRadius);
	float Distance(sf::Vector2f v1, sf::Vector2f v2);
	void MovementControl(sf::UdpSocket &sock, std::map<int, ClientProxy*> &clientProxies, int idPlayer, int idMove);
	void PlayerCollisionCheck(sf::UdpSocket &sock, std::map<int, ClientProxy*> &clientProxies, int idPlayer);

};

