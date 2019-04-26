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
std::vector<PlayerInfo*> otherPlayers;

//timers:
float helloSendingTimer = 2.f;

//declarations:
void HelloSending();
void AcumControl();
void Commands();
void GraphicsInterface();
void SendAcknowledge(int idPack);


int main()
{
	//Enviamos HELLO al servidor
	std::thread helloThread(&HelloSending);
	helloThread.detach();

	std::thread graphicsInterface(&GraphicsInterface);
	graphicsInterface.detach();

	std::thread commandsThread(&Commands);
	commandsThread.detach();

	std::thread acumControlThread(&AcumControl);
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
	Board board;
	board.InitializePlayerPosition(playerSelf.pos);

	for(int i = 0; i<otherPlayers.size(); i++)
	{
		board.InitializePlayerPosition(otherPlayers[i]->pos);
	}

	board.DibujaSFML();
}

void Commands()
{

}

void AcumControl()
{

}

void SendAcknowledge(int idPack)
{
	sf::Packet pack;
	pack << idPack;
	pack << static_cast<int>(Protocol::ACK);
	sock.send(pack, IP, PORT);
}