#pragma once
#include <iostream>
#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include "ClientProxy.h"
#include "CriticPack.h"
#include <Constants.h>
#include <thread>
#include <math.h>
#include <Food.h>


//---------SERVIDOR---------//


sf::UdpSocket sock;
const int maxFood = 100;
const float minFoodDist = 200.f;

//lists / maps
std::map<int, ClientProxy*> clientProxies;
std::map<int, CriticPack*> criticPackets;
std::vector<Food*> foodVector;


//declarations:
//threads:
void PingThreadFunction();
void DisconnectionCheckerThreadFunction();
void CriticPacketsManagerThreadFunction();
void MovementControlThread();
void FoodUpdateThread();
//other:
void NewPlayer(sf::IpAddress ip, unsigned short port, sf::Packet pack);
void AccumMovement(sf::Packet pack);
void MovementControl(int idPlayer, int idMove);
void InitializeFood();
bool RandomPacketLost();
float Distance(sf::Vector2f v1, sf::Vector2f v2);


int main()
{
	#pragma region Init

	srand(time(NULL));
	if (sock.bind(PORT) != sf::Socket::Status::Done)
	{
		std::cout << "Error on binding socket" << std::endl;
		//return 0;
	}

	std::cout << "Socket binded successfully" << std::endl;

	//Crear thread para PING
	std::thread pingThread(&PingThreadFunction);
	pingThread.detach();

	//Crear thread para comprobar desconexiones
	std::thread disconnectionCheckerThread(&DisconnectionCheckerThreadFunction);
	disconnectionCheckerThread.detach();

	//Crear thread para administrar paquetes críticos
	std::thread criticPacketsManagerThread(&CriticPacketsManagerThreadFunction);
	criticPacketsManagerThread.detach();

	//Thread de validació de moviment
	std::thread movementControlThread(&MovementControlThread);
	movementControlThread.detach();

	//initialize food:
	InitializeFood();

	//Thread de food
	/*std::thread foodUpdateThread(&FoodUpdateThread);
	foodUpdateThread.detach();*/

	#pragma endregion

	while (true)
	{
		sf::Packet pack;
		sf::IpAddress ip;
		unsigned short port;

		if (sock.receive(pack, ip, port) != sf::Socket::Status::Done)
		{
			std::cout << "Error on receiving packet ip:" << ip.toString() << std::endl;
		}

		if (RandomPacketLost())
		{
			//std::cout << "Reading package" << std::endl;
			int num;
			pack >> num;
			switch (static_cast<Protocol>(num))
			{
			case HELLO:
				std::cout << "HELLO received" << std::endl;
				NewPlayer(ip, port, pack);
				break;
			case ACK:
				std::cout << "ACK received" << std::endl;
				int auxIdPack;
				pack >> auxIdPack;
				criticPackets.erase(criticPackets.find(auxIdPack));
				break;
			case PONG:
				//std::cout << "PONG received" << std::endl;
				int pId;
				pack >> pId;
				clientProxies[pId]->numPings = 0;
				break;
			case MOVE:
				//std::cout << "MOVE received" << std::endl;
				AccumMovement(pack);
				break;
			}
		}
		else
		{
			std::cout << "Packet lost (on purpose)" << std::endl;
		}
	}

	return 0;
}

void PingThreadFunction()
{
	//sf::Clock clock;
	sf::Packet packPing;

	packPing << static_cast<int>(Protocol::PING);

	sf::Packet packFood;
	while (true)
	{
		//sf::Time t1 = clock.getElapsedTime();
		//if (t1.asSeconds() > PINGTIMER)
		//{
			for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
			{
				if (sock.send(packPing, it->second->ip, it->second->port) != sf::UdpSocket::Status::Done)
				{
					std::cout << "Error sending PING to client with id: " << it->second->id << "and ip: " << it->second->ip.toString() << std::endl;
				}
				else
				{
					it->second->numPings++;
				}
			}

			//ERE LAYS THE CULPRIT
			for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
			{
				packFood << static_cast<int>(Protocol::FOOD_UPDATE);

				//tota la info del menjar:
				//només els que té més aprop
				std::vector<Food*> closeFood;

				for (int i = 0; i < (int)foodVector.size(); i++)
				{
					if (Distance(it->second->bodyPositions[0], foodVector[i]->position) < minFoodDist)
					{
						closeFood.push_back(foodVector[i]);
					}
				}
				//num foods:
				packFood << static_cast<int>(closeFood.size());
				for (std::vector<Food*>::iterator closeit = closeFood.begin(); closeit != closeFood.end(); ++closeit)
				{
					Food* food = *closeit;
					//pack << food->id;
					packFood << food->position.x;
					packFood << food->position.y;
					//pack << food->color; //no puc passar el color?
				}

				sock.send(packFood, it->second->ip, it->second->port);
				packFood.clear();
			}

			//std::cout << "THREADS: PING going to sleep for " << PINGTIMER * 1000 << " Milliseconds." << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds((int)(PINGTIMER * 1000)));
			//std::cout << "THREADS: PING awakened " << std::endl;
			//clock.restart();
		//}
	}
}

void DisconnectionCheckerThreadFunction()
{
	//sf::Clock clock;
	sf::Packet pack;

	while (true)
	{
		//sf::Time t1 = clock.getElapsedTime();
		//if (t1.asSeconds() > DISCONNECTTIMER)
		//{
			//std::cout << "Checking for disconnected players" << std::endl;
			//Comprobar si el numPings supera el límite y enviar el DISCONNECTED a todos los demás clientes
			for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); /*++it*/)
			{
				if (it->second->numPings >= MAXUNOPERATIVETIMER)
				{
					//Añadir el paquete a la lista de críticos
					pack << static_cast<int>(Protocol::DISCONNECTED);
					int idPack = (int)criticPackets.size();
					pack << idPack;
					pack << it->second->id;

					//Enviar DISCONNECTED
					for (std::map<int, ClientProxy*>::iterator it2 = clientProxies.begin(); it2 != clientProxies.end(); ++it2)
					{
						if (it2 != it)
						{

							CriticPack* cp = new CriticPack((int)criticPackets.size(), pack, it2->second->ip, it2->second->port);
							criticPackets[idPack] = cp;

							sock.send(pack, it2->second->ip, it2->second->port);
						}
					}

					it = clientProxies.erase(it);
				}
				else
				{
					++it;
				}
			}

			//std::cout << "THREADS: DISCONNECT going to sleep for " << DISCONNECTTIMER * 1000 << " Milliseconds." << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds((int)(DISCONNECTTIMER * 1000)));
			//std::cout << "THREADS: DISCONNECT awakened" << std::endl;
			//clock.restart();
		//}
	}
}

void CriticPacketsManagerThreadFunction()
{
	//sf::Clock clock;

	while (true)
	{
		//sf::Time t1 = clock.getElapsedTime();

		//if (t1.asSeconds() > CRITICPACKETSTIMER)
		//{
			for (std::map<int, CriticPack*>::iterator it = criticPackets.begin(); it != criticPackets.end(); ++it)
			{
				sock.send(it->second->pack, it->second->ip, it->second->port);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds((int)(CRITICPACKETSTIMER * 1000)));
			//clock.restart();
		//}
	}
}

void MovementControlThread()
{
	//sf::Clock clock;

	while (true)
	{
		//sf::Time t1 = clock.getElapsedTime();
		//if (t1.asSeconds() > MOVEMENTUPDATETIMER)
		//{
			for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
			{
				if (abs(it->second->accumMovement.x) + abs(it->second->accumMovement.y) > 0)
				{
					MovementControl(it->second->id, it->second->lastIdMove);
				}
			}

			//std::cout << "THREADS: MOVEMENT going to sleep for " << MOVEMENTUPDATETIMER * 1000 << " Milliseconds." << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds((int)(MOVEMENTUPDATETIMER * 1000)));
			//std::cout << "THREADS: MOVEMENT awakened" << std::endl;
			//clock.restart();
		//}
	}
}

void FoodUpdateThread()
{
	//sf::Clock clock;
	sf::Packet pack;

	while (true)
	{
		//sf::Time t1 = clock.getElapsedTime();
		//if (t1.asSeconds() > FOODUPDATETIMER)
		//{
			for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
			{
				pack << static_cast<int>(Protocol::FOOD_UPDATE);

				//tota la info del menjar:
				//només els que té més aprop
				std::vector<Food*> closeFood;

				for (int i = 0; i < (int)foodVector.size(); i++)
				{
					if (Distance(it->second->bodyPositions[0], foodVector[i]->position) < minFoodDist)
					{
						closeFood.push_back(foodVector[i]);
					}
				}
				//num foods:
				pack << static_cast<int>(closeFood.size());
				for (std::vector<Food*>::iterator closeit = closeFood.begin(); closeit != closeFood.end(); ++closeit)
				{
					Food* food = *closeit;
					//pack << food->id;
					pack << food->position.x;
					pack << food->position.y;
					//pack << food->color; //no puc passar el color?
				}

				sock.send(pack, it->second->ip, it->second->port);
				pack.clear();
			}

			//std::cout << "THREADS: FOOD going to sleep for " << FOODUPDATETIMER * 1000 << " Milliseconds." << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds((int)(FOODUPDATETIMER * 1000)));
			//std::cout << "THREADS: FOOD awakened" << std::endl;
			//clock.restart();
		//}
	}
}

void NewPlayer(sf::IpAddress ip, unsigned short port, sf::Packet pack)
{
	std::string alias;
	pack >> alias;

	bool clientExists = false;
	ClientProxy* newClient = nullptr;
	//busquem per ip i port si el client ja existeix:
	for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
	{
		ClientProxy* client = it->second;
		if (client->ip == ip && client->port == port)
		{
			std::cout << "Client already exists. The id is: "<<client->id << std::endl;
			newClient = client;
			clientExists = true;
			break;
		}
	}

	if (!clientExists)
	{
		std::cout << "creating new client" << std::endl;
		//és un nou player
		int id = (int)clientProxies.size();
		float x = rand() % SCREEN_WIDTH;
		float y = rand() % SCREEN_HEIGHT;
		sf::Vector2f headPos(x, y);
		newClient = new ClientProxy(id, alias, ip, port, headPos);
	}

	if (newClient == nullptr)
	{
		std::cout << "Problem with newClient" << std::endl;
		return;
	}

	//introduïm les dades del player
	pack.clear();
	pack << static_cast<int>(Protocol::WELCOME);
	newClient->AddDataToPacket(&pack);

	//std::cout << "INITIAL POS x (new Client) " << newClient->bodyPositions[0].x << " y: " << newClient->bodyPositions[0].y << std::endl;
	
	//els altres players:
	pack << static_cast<int>(clientProxies.size());
	for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
	{
		if (it->second->id != newClient->id)
		{
			it->second->AddDataToPacket(&pack);
		}
	}

	//tota la info del menjar:
	//només els que té més aprop
	std::vector<Food*> closeFood;
	for (int i = 0; i < (int)foodVector.size(); i++)
	{
		if (Distance(newClient->bodyPositions[0], foodVector[i]->position) < minFoodDist)
		{
			closeFood.push_back(foodVector[i]);
		}
	}

	//num foods:
	pack << static_cast<int>(closeFood.size());
	for (std::vector<Food*>::iterator it = closeFood.begin(); it != closeFood.end(); ++it)
	{
		Food* food = *it;
		pack << food->position.x;
		pack << food->position.y;
		//pack << food->color; //no puc passar el color?
	}

	sock.send(pack, ip, port);

	//Enviar NEW_PLAYER a todos los demás clientes
	for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
	{
		//com que cada vegada hem d'anar posant un id del packet diferent, anem tornant a omplir el packet
		pack.clear();
		pack << static_cast<int>(Protocol::NEW_PLAYER);
		newClient->AddDataToPacket(&pack);
		int idPack = (int)criticPackets.size();
		pack << idPack;

		CriticPack* cp = new CriticPack((int)criticPackets.size(), pack, it->second->ip, it->second->port);
		criticPackets[idPack] = cp;
		std::cout << "sending NEW_PLAYER" << std::endl;
		sock.send(pack, it->second->ip, it->second->port);
	}

	//finalment afegim el nou client al map
	if(!clientExists)
		clientProxies[newClient->id] = newClient;
}

void AccumMovement(sf::Packet pack)
{
	int idPlayer;
	int idMove;
	pack >> idPlayer>>idMove;
	int x, y;
	pack >> x >> y;

	sf::Vector2f sumPos;
	sumPos.x = (float)x / 1000.f;
	sumPos.y = (float)y / 1000.f;

	clientProxies[idPlayer]->accumMovement += sumPos;//en comptes de tractar-lo directament, l'acumulo i es tractarà en el thread de moviment
	clientProxies[idPlayer]->lastIdMove = idMove;
}

void FoodCollisionCheck(std::vector<sf::Vector2f> playerPositions, float playerBodyRadius)
{
	int i = 0;
	//std::cout << "Food vector size: " << (int)foodVector.size() << std::endl;
	while (i < (int)foodVector.size())
	{
		bool collided = false;
		for (int j = 0; j < (int)playerPositions.size(); j++)
		{
			if (Distance(foodVector[i]->position, playerPositions[j]) < playerBodyRadius)
			{
				collided = true;
				break;
			}
		}

		if (/*Collision*/collided)
		{
			//Food eaten
			foodVector.erase(foodVector.begin() + i);//comprovar que funcioni
		}
		else
		{
			i++;
		}
	}
}

void MovementControl(int idPlayer, int idMove)
{
	sf::Vector2f possiblePos = clientProxies[idPlayer]->SumToHeadPosition();

	//controlar la posició
	if (possiblePos.x < 0 || possiblePos.x > SCREEN_WIDTH || possiblePos.y < 0 || possiblePos.y > SCREEN_HEIGHT)
	{
		//el posem just al límit de la pantalla? o simplement on era abans de moure's?
		//si no fem update position, simplement el retornarem on era.
	}
	else
	{
		//tot correcte, movem el player
		clientProxies[idPlayer]->UpdatePosition(possiblePos);
	}

	clientProxies[idPlayer]->accumMovement = sf::Vector2f(0,0);

	//ho enviem a tots els players
	sf::Packet pack;
	pack << static_cast<int>(Protocol::MOVE);
	pack << idPlayer;
	pack << idMove;
	clientProxies[idPlayer]->PutBodyPositions(&pack);

	for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
	{
		sock.send(pack, it->second->ip, it->second->port);
	}

	FoodCollisionCheck(clientProxies[idPlayer]->bodyPositions, 13.f);//13 provisional
}

void InitializeFood()
{
	for (int i = 0; i < maxFood; i++)
	{
		sf::Vector2f pos;
		pos.x = rand() % SCREEN_WIDTH;
		pos.y = rand() % SCREEN_HEIGHT;
		foodVector.push_back(new Food(i, pos));
	}
}

float GetRandomFloat()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0.f, 1.f);
	return dis(gen);
}

bool RandomPacketLost()
{
	//return true;

	float f = GetRandomFloat();
	//std::cout << "random float is: " << f << std::endl;
	if (f > PERCENT_PACKETLOSS)
	{
		return true;//all good
	}

	return false;//packet lost
}

float Distance(sf::Vector2f v1, sf::Vector2f v2)
{
	sf::Vector2f v = v2 - v1;
	return sqrt(v.x*v.x + v.y*v.y);
}