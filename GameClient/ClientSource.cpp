#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML\Network.hpp>
#include "Board.h"
#include <Constants.h>
#include <thread>
#include "Player.h"
#include <mutex>
#include "FoodBall.h"

//---------CLIENTE---------//


sf::UdpSocket sock;
bool received = false;
std::mutex mtx;

//maps:
std::map<int, Player*> playersMap;
std::map<int, sf::Vector2f> movesMap;//idMove i headPos
std::map<int, Food*> foodBallMap;

Player* m_player;
sf::Vector2f accumMove;
Board board;

//timers:
const float helloSendingTimer = 2.f;
const float movementTimer = 0.1f;
const float criticResendTimer = 2.f;
const float percentLostTimer = 0.05f;

//declarations:
void HelloSending();
void GraphicsInterface();
void MoveSending();
void SendAcknowledge(int idPack);



int main()
{
	//Enviamos HELLO al servidor
	std::thread helloThread(&HelloSending);
	helloThread.detach();

	std::thread graphicsInterface(&GraphicsInterface);
	graphicsInterface.detach();

	std::thread accumControlThread(&MoveSending);
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
			//std::lock_guard<std::mutex> guard(mtx);

			//std::cout << "Packet received" << std::endl;
			if (ip == IP && port == PORT)//ES EL SERVER
			{
				int num;
				pack >> num;
				switch (static_cast<Protocol>(num))
				{
				case WELCOME:
				{
					//create the player:
					m_player = new Player(&pack);
					m_player->color = sf::Color::Green;
					playersMap[m_player->id] = m_player;

					std::cout << "WELCOME player "<< m_player->id << std::endl;

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

					//create the balls:

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
					//std::cout << "MOVE received" << std::endl;
					int idPlayer;
					int idMove;
					pack >> idPlayer >> idMove;

					if (idPlayer == m_player->id)
					{
						//vector on posem els moviments que volem esborrar
						std::vector<std::map<int, sf::Vector2f>::iterator> toErase;
						
						if (movesMap.find(idMove) != movesMap.end())//si existeix el idMove
						{
							//RECONCILIACIÓ:
							//si coincideix amb la posició, s'elimina. Sinó, s'ha de corregir la posició del jugador.
							//també podria ser que fos un moviment vell. Com ho tractem? Potser guardant l'últim idMove que hem tractat?

							//m_player->UpdatePosition(&pack);
							//board.UpdateSlither(idPlayer);
							accumMove = sf::Vector2f(0,0);//tornem a posar l'acumulat a 0

							//esborrem els moviments anteriors posant-los a toErase
							for (std::map<int, sf::Vector2f>::iterator it = movesMap.begin(); it != movesMap.end(); ++it)
							{
								if (it->first < idMove)
								{
									toErase.push_back(it);
								}
							}
							while ((int)toErase.size() != 0)
							{
								movesMap.erase(toErase[0]);
								toErase.erase(toErase.begin());
							}
						}
					}
					else
					{
						playersMap[idPlayer]->UpdatePosition(&pack);
						board.UpdateSlither(idPlayer);
					}
				}
					break;
				case FOOD_EATEN:
				{
					//update food map
				}
					break;
				case FOOD_SPAWNED:
				{
					//update food map
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
		std::lock_guard<std::mutex> guard(mtx);

		Player* player = it->second;
		board.InitializeSlither(player);
	}

	board.window.create(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Slither Remake");
	while (board.window.isOpen())
	{
		board.DrawBoard();
		board.Commands(m_player);
		accumMove += board.playerMovement;

		//prediction movement:
		if (abs(board.playerMovement.x) + abs(board.playerMovement.y) > 0)
		{
			m_player->UpdatePosition(board.playerMovement);
			board.UpdateSlither(m_player->id);
		}
	}
}

void MoveSending()
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
				pack << (int)movesMap.size();//podria ser que no funcionés així
				movesMap[(int)movesMap.size()] = accumMove;
				pack << accumMove.x << accumMove.y;

				if (sock.send(pack, IP, PORT) != sf::UdpSocket::Status::Done)
				{
					std::cout << "Error sending the packet" << std::endl;
				}
			}
			
			clock.restart();
		}
	}

}

void SendAcknowledge(int idPack)
{
	sf::Packet pack;
	pack << static_cast<int>(Protocol::ACK);
	pack << idPack;
	sock.send(pack, IP, PORT);
}