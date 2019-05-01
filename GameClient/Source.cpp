#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML\Network.hpp>
#include "Board.h"
#include <Constants.h>
#include <thread>
#include <PlayerInfo.h>

//---------CLIENTE---------//


sf::UdpSocket sock;
bool received = false;

PlayerInfo* m_playerInfo;
std::map<int, PlayerInfo*> playersMap;//seria millor un map
sf::Vector2f accumMove;
Board board;

//timers:
float helloSendingTimer = 2.f;
float movementTimer = 0.1f;

//declarations:
void HelloSending();
void GraphicsInterface();
void AccumControl();
void SendAcknowledge(int idPack);


int main()
{
	//Enviamos HELLO al servidor
	std::thread helloThread(&HelloSending);
	helloThread.detach();

	std::thread graphicsInterface(&GraphicsInterface);
	graphicsInterface.detach();

	std::thread acumControlThread(&AccumControl);
	acumControlThread.detach();


	while (true)
	{
		sf::Packet pack;
		sf::IpAddress ip;
		unsigned short port;
		if (sock.receive(pack, ip, port) != sf::UdpSocket::Status::Done)
		{
			//std::cout << "Error while receiving packet" << std::endl;
		}
		else
		{
			//std::cout << "Packet received" << std::endl;

			if (ip == IP && port == PORT)//ES EL SERVER
			{
				int num;
				pack >> num;
				switch (static_cast<Protocol>(num))
				{
				case WELCOME:
				{
					std::cout << "WELCOME received" << std::endl;

					//create the player:
					PlayerInfo* p = new PlayerInfo();
					pack >> p->id;
					pack >> p->alias;
					pack >> p->pos.x;
					pack >> p->pos.y;
					m_playerInfo = p;
					playersMap[p->id] = p;

					//create the others:
					int sizeOthers;
					pack >> sizeOthers;
					for (int i = 0; i < sizeOthers; i++)
					{
						PlayerInfo* p = new PlayerInfo();
						pack >> p->id;
						pack >> p->alias;
						pack >> p->pos.x;
						pack >> p->pos.y;

						if (playersMap.count(p->id) == 0)
						{
							playersMap[p->id] = p;
						}
					}
					received = true;
				}
					break;
				case NEW_PLAYER:
				{
					PlayerInfo* p = new PlayerInfo();
					pack >> p->id;
					pack >> p->alias;
					pack >> p->pos.x;
					pack >> p->pos.y;
					playersMap[p->id] = p;

					int idPack;
					pack >> idPack;
					SendAcknowledge(idPack);
				}
					break;
				case PING:
				{
					//std::cout << "PING received" << std::endl;
					pack.clear();
					pack << static_cast<int>(Protocol::PONG);
					pack << m_playerInfo->id;
					sock.send(pack, IP, PORT);
				}
					break;
				case DISCONNECTED:
				{
					std::cout << "DISCONNECTED received" << std::endl;
					int idPack;
					int idPlayer;
					pack >> idPack >> idPlayer;
					//PLAYER IDPLAYER DISCONNECTED

					SendAcknowledge(idPack);
				}
					break;
				case MOVE:
				{
					//std::cout << "MOVE" << std::endl;
					int idPlayer;
					int idMove;
					pack >> idPlayer >> idMove;
					pack >> playersMap[idPlayer]->pos.x;
					pack >> playersMap[idPlayer]->pos.y;
					board.UpdatePlayerPosition(0, playersMap[idPlayer]->pos);
					accumMove = sf::Vector2f(0, 0);
				}
					break;
				}
			}
		}
	}

	return 0;
}

void HelloSending()
{
	sf::Clock clock;
	sf::Packet pack;
	std::string alias = "Guillem";
	pack << static_cast<int>(Protocol::HELLO);
	pack << alias;

	while (!received)
	{
		sf::Time t1 = clock.getElapsedTime();
		if (t1.asSeconds() > helloSendingTimer)
		{
			if (sock.send(pack, IP, PORT) != sf::UdpSocket::Status::Done)
			{
				std::cout << "Error sending the packet" << std::endl;
			}
			else
			{
				std::cout << "HELLO enviado" << std::endl;
			}
			clock.restart();
		}
	}
}

void GraphicsInterface()
{
	while (!received)
	{
		//just don't start
	}

	accumMove = sf::Vector2f(0,0);

	//crear el taulell amb les coordenades que ara ja tenim
	for (std::map<int, PlayerInfo*>::iterator it = playersMap.begin(); it != playersMap.end(); ++it)
	{
		PlayerInfo* player = it->second;
		//board.InitializeSlither(player->pos);
	}

	board.window.create(sf::VideoMode(SCREEN_PROVISIONAL, SCREEN_PROVISIONAL), "Ejemplo tablero");
	while (board.window.isOpen())
	{
		board.DibujaSFML();
		board.Commands();
		accumMove += board.playerMovement;
	}
}


void AccumControl()
{
	while (!received)
	{
		//just don't start
	}

	sf::Clock clock;
	sf::Packet pack;
	sf::Vector2f lastPos = m_playerInfo->pos;

	while (true)
	{
		sf::Time t1 = clock.getElapsedTime();
		if (t1.asSeconds() > movementTimer)
		{
			if (true)//accumMove.x != 0 && accumMove.y != 0)
			{
				pack.clear();
				pack << static_cast<int>(Protocol::MOVE);
				pack << m_playerInfo->id;
				pack << accumMove.x << accumMove.y;
				if (sock.send(pack, IP, PORT) != sf::UdpSocket::Status::Done)
				{
					std::cout << "Error sending the packet" << std::endl;
				}
				else
				{
					//std::cout << "moving" << std::endl;
				}
			}
			
			clock.restart();
		}
	}

}

void SendAcknowledge(int idPack)
{
	sf::Packet pack;
	pack << idPack;
	pack << static_cast<int>(Protocol::ACK);
	sock.send(pack, IP, PORT);
}