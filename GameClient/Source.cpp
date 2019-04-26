#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML\Network.hpp>
#include "Board.h"
#include <Constants.h>
#include <thread>
#include "PlayerInfo.h"

//---------CLIENTE---------//


sf::UdpSocket sock;
bool received = false;

PlayerInfo playerSelf;
std::vector<PlayerInfo*> otherPlayers;//seria millor un map
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
					pack >> playerSelf.id;
					pack >> playerSelf.pos.x;
					pack >> playerSelf.pos.y;
					int sizeOthers;
					pack >> sizeOthers;
					for (int i = 0; i < sizeOthers; i++)
					{
						PlayerInfo* p = new PlayerInfo();
						pack >> p->id;
						pack >> p->alias;
						pack >> p->pos.x;
						pack >> p->pos.y;

						bool alreadyExists = false;
						for (int j = 0; j < otherPlayers.size(); j++)
						{
							if (otherPlayers[j]->id == p->id)
							{
								alreadyExists = true;
							}
						}
						if (!alreadyExists)
							otherPlayers.push_back(p);
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
					otherPlayers.push_back(p);

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
					pack << playerSelf.id;
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
					if (playerSelf.id == idPlayer)
					{
						//std::cout << "idPlayer" << std::endl;
						pack >> playerSelf.pos.x;
						pack >> playerSelf.pos.y;
						board.UpdatePlayerPosition(0, playerSelf.pos);
						accumMove = sf::Vector2f(0,0);
						//accumMove = play;
					}
					else
					{
						// amb un map aniríem directament al idPlayer que volem
					}
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

	//crear el taulell amb les coordenades que ara ja tenim
	board.InitializePlayerPosition(playerSelf.pos);

	accumMove = sf::Vector2f(0,0);

	for(int i = 0; i<otherPlayers.size(); i++)
	{
		board.InitializePlayerPosition(otherPlayers[i]->pos);
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
	sf::Vector2f lastPos = playerSelf.pos;

	while (true)
	{
		sf::Time t1 = clock.getElapsedTime();
		if (t1.asSeconds() > movementTimer)
		{
			if (true)//accumMove.x != 0 && accumMove.y != 0)
			{
				pack.clear();
				pack << static_cast<int>(Protocol::MOVE);
				pack << playerSelf.id;
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