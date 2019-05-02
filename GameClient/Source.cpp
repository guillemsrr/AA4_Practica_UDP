#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML\Network.hpp>
#include "Board.h"
#include <Constants.h>
#include <thread>
#include "Player.h"

//---------CLIENTE---------//


sf::UdpSocket sock;
bool received = false;

Player* m_player;
std::map<int, Player*> playersMap;
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

	std::thread accumControlThread(&AccumControl);
	accumControlThread.detach();

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
					m_player = new Player(&pack);
					m_player->color = sf::Color::Green;
					playersMap[m_player->id] = m_player;

					//create the others:
					int sizeOthers;
					pack >> sizeOthers;
					for (int i = 0; i < sizeOthers; i++)
					{
						Player* p = new Player(&pack);
						p->color = sf::Color::Red;
						playersMap[p->id] = p;
					}
					received = true;
				}
					break;
				case NEW_PLAYER:
				{
					Player* p = new Player(&pack);
					p->color = sf::Color::Red;
					playersMap[p->id] = p;
					board.InitializeSlither(p);

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
					pack << m_player->id;
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

					playersMap[idPlayer]->UpdatePosition(&pack);
					board.UpdateSlither(idPlayer);

					if (idPlayer == m_player->id)
					{
						accumMove = sf::Vector2f(0, 0);
					}

					//dins de MOVE també passa els altres jugadors??
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
	std::string alias = "Default Alias";
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
	for (std::map<int, Player*>::iterator it = playersMap.begin(); it != playersMap.end(); ++it)
	{
		Player* player = it->second;
		board.InitializeSlither(player);
	}

	board.window.create(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Slither Remake");
	while (board.window.isOpen())
	{
		board.DrawBoard();
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

	while (true)
	{
		sf::Time t1 = clock.getElapsedTime();
		if (t1.asSeconds() > movementTimer)
		{
			if (abs(accumMove.x) + abs(accumMove.y) > 0)
			{
				pack.clear();
				pack << static_cast<int>(Protocol::MOVE);
				pack << m_player->id;
				pack << 1;//idMove provisional
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