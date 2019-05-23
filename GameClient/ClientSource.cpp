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
#include <chrono>

//---------CLIENTE---------//


sf::UdpSocket sock;
bool received = false;
std::mutex mtx;
int actualMoveID = 0;

//maps:
std::map<int, Player*> playersMap;
std::map<int, sf::Vector2f> movesMap;//idMove i headPos
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

					std::vector<sf::Vector2f> foodPositions;

					for (int i = 0; i < numFood; i++)
					{
						int id;
						sf::Vector2f pos;
						pack >> pos.x;
						pack >> pos.y;
						foodPositions.push_back(pos);
					}

					board.foodPositions = foodPositions;//aix� s'hauria de millorar..

					received = true;

					std::thread graphicsInterface(&GraphicsInterface);
					graphicsInterface.detach();

					std::thread accumControlThread(&MoveSending);
					accumControlThread.detach();

					std::thread interpolationsThread(&InterpolatePositions);
					interpolationsThread.detach();
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
							//RECONCILIACI�:

							//comprovem en quina posici� est�vem / estem, i si coincideix
							int numPos;
							int x, y;
							pack >> numPos;
							pack >> x;
							pack >> y;

							sf::Vector2f headPos;
							headPos.x = (float)x / 1000.f;
							headPos.y = (float)y / 1000.f;


							float tol = 1;// sqrt(2) / 10.f;
							sf::Vector2f sub = movesMap[idMove] - headPos;
							sub.x = abs(sub.x);
							sub.y = abs(sub.y);
							if (sqrt(sub.x*sub.x + sub.y*sub.y) > tol)
							{
								std::cout << "Position modified!" << std::endl;
								//std::cout << "movesMap y: "<< (float)movesMap[idMove].y << " head pos y " << (float)headPos.y << std::endl;

								m_player->UpdateTheRestOfPositions(numPos, headPos, &pack);
								board.UpdateSlither(idPlayer);

								accumMove = sf::Vector2f(0.0f, 0.0f);
								bool x = (float)movesMap[idMove].x != (float)headPos.x;
								bool y = ((float)movesMap[idMove].y != (float)headPos.y);
								std::cout << "x bool: " << x << std::endl;
								std::cout << "y bool: " << y << std::endl;
								std::cout << "headPos: (x "<< headPos.x << " , y " << headPos.y << std::endl;
								std::cout << "movesMap[idMove]: (x "<< movesMap[idMove].x << " , y " << movesMap[idMove].y << std::endl;
							}
							else
							{
								std::cout << "good position" << std::endl;
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
								std::cout << "erasing" << std::endl;
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

					int numFood;
					pack >> numFood;
					//std::cout << "num balls: " << numFood << std::endl;
					std::vector<sf::Vector2f> foodPositions;
					for (int i = 0; i < numFood; i++)
					{
						int id;
						sf::Vector2f pos;
						pack >> pos.x;
						pack >> pos.y;
						foodPositions.push_back(pos);
					}

					board.foodPositions = foodPositions;
					//std::cout << "foodPositions size: " << foodPositions.size() << std::endl;
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
	//sf::Clock clock;
	sf::Packet pack;
	std::string alias = "Default Alias";
	pack << static_cast<int>(Protocol::HELLO);
	pack << alias;

	while (!received)
	{
		//sf::Time t1 = clock.getElapsedTime();
		//if (t1.asSeconds() > helloSendingTimer)
		//{
			if (sock.send(pack, IP, PORT) != sf::UdpSocket::Status::Done)
			{
				std::cout << "Error sending the packet" << std::endl;
			}
			else
			{
				std::cout << "HELLO enviado" << std::endl;
			}
			//clock.restart();

			//std::cout << "THREADS: HELLO going to sleep for " << helloSendingTimer * 1000 << " Milliseconds." << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds((int)(1000 * helloSendingTimer)));
			//std::cout << "THREADS: HELLO awakened" << std::endl;
		//}
	}
}

void GraphicsInterface()
{
	while (!received)
	{
		std::cout << "NOT STARTING YET" << std::endl;
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

		sf::Vector2i sumInt = sf::Vector2i((int)(board.playerMovement.x*1000), (int)(board.playerMovement.y*1000));
		board.playerMovement = sf::Vector2f((float)sumInt.x/1000.f, (float)sumInt.y / 1000.f);
		accumMove += board.playerMovement;

		sumInt = sf::Vector2i((int)(accumMove.x * 1000), (int)(accumMove.y * 1000));
		accumMove = sf::Vector2f((float)sumInt.x / 1000.f, (float)sumInt.y / 1000.f);

		////prediction movement:
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

	//sf::Clock clock;
	sf::Packet pack;

	while (true)
	{
		//sf::Time t1 = clock.getElapsedTime();
		//if (t1.asSeconds() > movementTimer)
		//{
			if (abs(accumMove.x) + abs(accumMove.y) > 0)
			{
				pack.clear();
				pack << static_cast<int>(Protocol::MOVE);
				pack << m_player->id;
				pack << actualMoveID;

				movesMap[actualMoveID] = m_player->bodyPositions[0];
				actualMoveID++;
				pack << (int)(accumMove.x*1000) << (int)(accumMove.y*1000);

				if (sock.send(pack, IP, PORT) != sf::UdpSocket::Status::Done)
				{
					std::cout << "Error sending the packet" << std::endl;
				}

				accumMove = sf::Vector2f(0, 0);
			}

			//std::cout << "THREADS: MOVEMENT going to sleep for " << movementTimer * 1000 << " Milliseconds." << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds((int)(movementTimer * 1000)));
			//std::cout << "THREADS: MOVEMENT awakened" << std::endl;
			//clock.restart();
		//}
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

	//sf::Clock clock;

	while (true)
	{
		//sf::Time t1 = clock.getElapsedTime();
		//if (t1.asSeconds() > interpolationTimer)
		//{
			std::vector<std::map<int, std::vector<sf::Vector2f>>::iterator> toErase;//vector per eliminar despr�s
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

			//std::cout << "THREADS: INTERP going to sleep for " << interpolationTimer * 1000 << " Milliseconds." << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds((int)(interpolationTimer * 1000)));
			//std::cout << "THREADS: INTERP awakened" << std::endl;
			//clock.restart();
		//}
	}
}