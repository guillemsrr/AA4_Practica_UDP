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
int actualMoveID = 0;

//maps:
std::map<int, Player*> playersMap;
std::map<int, sf::Vector2f> movesMap;//idMove i headPos
std::map<int, FoodBall*> foodBallMap;
std::map<int, std::vector<sf::Vector2f>> interpolationsMap;

Player* m_player;
sf::Vector2f accumMove;
Board board;

//timers:
const float helloSendingTimer = 2.f;
const float movementTimer = 0.1f;
const float criticResendTimer = 2.f;
const float interpolationTimer = 0.001f;

//declarations:
void HelloSending();
void GraphicsInterface();
void MoveSending();
void SendAcknowledge(int idPack);
void InterpolatePositions();



int main()
{
	//Enviamos HELLO al servidor
	std::thread helloThread(&HelloSending);
	helloThread.detach();

	std::thread graphicsInterface(&GraphicsInterface);
	graphicsInterface.detach();

	std::thread accumControlThread(&MoveSending);
	accumControlThread.detach();

	std::thread interpolationsThread(&InterpolatePositions);
	interpolationsThread.detach();

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

					//create the balls:
					int numFood;
					pack >> numFood;
					std::cout << "num balls: " << numFood << std::endl;

					for (int i = 0; i < numFood; i++)
					{
						int id;
						sf::Vector2f pos;
						pack >> id;
						pack >> pos.x;
						pack >> pos.y;
						foodBallMap[id] = new FoodBall(id, pos);
					}

					board.foodBallMap = foodBallMap;//això s'hauria de millorar..

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

							//comprovem en quina posició estàvem / estem, i si coincideix
							int numPos;
							sf::Vector2f headPos;
							pack >> numPos;
							pack >> headPos.x;
							pack >> headPos.y;

							if (movesMap[idMove] != headPos)
							{
								std::cout << "Position modified!" << std::endl;

								m_player->UpdateTheRestOfPositions(numPos, headPos, &pack);
								board.UpdateSlither(idPlayer);
							}

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
						//just save the final position to interpolate!
						//std::cout << "saved move to interpolation" << std::endl;
						interpolationsMap[idPlayer] = playersMap[idPlayer]->GetFuturePositions(&pack);
					}
				}
					break;
				case FOOD_UPDATE:
				{
					if (!received)
					{
						break;
					}

					std::lock_guard<std::mutex> guard(mtx);

					//update food map
					//create the balls:
					int numFood;
					pack >> numFood;
					std::cout << "num balls: " << numFood << std::endl;

					foodBallMap.clear();//easy però no és òptim
					board.foodBallMap.clear();

					for (int i = 0; i < numFood; i++)
					{
						int id;
						sf::Vector2f pos;
						pack >> id;
						pack >> pos.x;
						pack >> pos.y;
						foodBallMap[id] = new FoodBall(id, pos);
					}

					board.foodBallMap = foodBallMap;//això s'hauria de millorar..
				}
					break;
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
		std::lock_guard<std::mutex> guard(mtx);

		board.DrawBoard();
		board.Commands(m_player);
		accumMove += board.playerMovement;

		//prediction movement:
		if (abs(board.playerMovement.x) + abs(board.playerMovement.y) > 0)
		{
			m_player->UpdatePosition(board.playerMovement);
			board.UpdateSlither(m_player->id);
		}

		//draw foodballs:
		for (std::map<int, FoodBall*>::iterator it = foodBallMap.begin(); it != foodBallMap.end(); ++it)
		{
			board.DrawFood(it->second->circleShape);
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
				pack << actualMoveID;

				movesMap[actualMoveID] = m_player->bodyPositions[0];
				actualMoveID++;
				pack << accumMove.x << accumMove.y;

				if (sock.send(pack, IP, PORT) != sf::UdpSocket::Status::Done)
				{
					std::cout << "Error sending the packet" << std::endl;
				}

				accumMove = sf::Vector2f(0, 0);
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

void InterpolatePositions()
{
	while (!received)
	{
		//just don't start
	}

	sf::Clock clock;

	while (true)
	{
		sf::Time t1 = clock.getElapsedTime();
		if (t1.asSeconds() > interpolationTimer)
		{
			std::vector<std::map<int, std::vector<sf::Vector2f>>::iterator> toErase;//vector per eliminar després
			for (std::map<int, std::vector<sf::Vector2f>>::iterator it = interpolationsMap.begin(); it != interpolationsMap.end(); ++it)
			{
				if (playersMap[it->first]->InterpolateTo(it->second, 0.5f))
				{
					//std::cout << "interpolation arrived and erased" << std::endl;
					toErase.push_back(it);
				}

				board.UpdateSlither(it->first);

				//std::cout << "interpolating" << std::endl;
			}

			while (toErase.size() != 0)
			{
				interpolationsMap.erase(toErase[0]);
				toErase.erase(toErase.begin());
			}

			clock.restart();
		}
	}
}