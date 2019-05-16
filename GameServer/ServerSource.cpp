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
#include "jdbc/mysql_connection.h"
#include "jdbc/mysql_driver.h"
#include "jdbc/cppconn/statement.h"
#include "jdbc/cppconn/resultset.h"


//---------SERVIDOR---------//


sf::UdpSocket sock;
const int maxFood = 100;

//lists / maps
std::map<int, ClientProxy*> clientProxies;
std::map<int, CriticPack*> criticPackets;
std::map<int, Food*> foodMap;

//timers:
const float criticPacketsTimer = 10.0f;
const float pingTimer = 5.0f;
const float percentLostTimer = 0.05f;
const float disconnectTimer = 10.f;
const float maxUnoperativeTime = 30.f;
const float serverUpdateTimer = 0.05f;//?�?�


//declarations:
//threads:
void PingThreadFunction();
void DisconnectionCheckerThreadFunction();
void CriticPacketsManagerThreadFunction();
void MovementControlThread();
//other:
void AnswerRegister(sf::IpAddress ip, unsigned short port, sf::Packet pack);
void AnswerLogin(sf::IpAddress ip, unsigned short port, sf::Packet pack);
void NewPlayer(sf::IpAddress ip, unsigned short port, sf::Packet pack);
void AccumMovement(sf::Packet pack);
void MovementControl(int idPlayer, int idMove);
void InitializeFood();
bool RandomPacketLost();


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

	//Crear thread para administrar paquetes cr�ticos
	std::thread criticPacketsManagerThread(&CriticPacketsManagerThreadFunction);
	criticPacketsManagerThread.detach();

	//Thread de validaci� de moviment
	std::thread movementControlThread(&MovementControlThread);
	movementControlThread.detach();

	//initialize food:
	InitializeFood();

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
			case REGISTER:
				std::cout << "Petici�n de registro recibida." << std::endl;
				AnswerRegister(ip, port, pack);
				break;
			case LOGIN:
				std::cout << "Petici�n de login recibida." << std::endl;
				AnswerLogin(ip, port, pack);
				break;
			}
		}
		else
		{
			std::cout << "Packet lost " << std::endl;
		}
	}

	return 0;
}

void PingThreadFunction()
{
	sf::Clock clock;
	sf::Packet pack;

	pack << static_cast<int>(Protocol::PING);

	while (true)
	{
		sf::Time t1 = clock.getElapsedTime();
		if (t1.asSeconds() > pingTimer)
		{
			for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
			{
				if (sock.send(pack, it->second->ip, it->second->port) != sf::UdpSocket::Status::Done)
				{
					std::cout << "Error sending PING to client with id: " << it->second->id << "and ip: " << it->second->ip.toString() << std::endl;
				}
				else
				{
					it->second->numPings++;
				}
			}

			clock.restart();
		}
	}
}

void DisconnectionCheckerThreadFunction()
{
	sf::Clock clock;
	sf::Packet pack;

	while (true)
	{
		sf::Time t1 = clock.getElapsedTime();
		if (t1.asSeconds() > disconnectTimer)
		{
			//std::cout << "Checking for disconnected players" << std::endl;
			//Comprobar si el numPings supera el l�mite y enviar el DISCONNECTED a todos los dem�s clientes
			for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); /*++it*/)
			{
				if (it->second->numPings >= maxUnoperativeTime)
				{
					//A�adir el paquete a la lista de cr�ticos
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
			clock.restart();
		}
	}
}

void CriticPacketsManagerThreadFunction()
{
	sf::Clock clock;

	while (true)
	{
		sf::Time t1 = clock.getElapsedTime();

		if (t1.asSeconds() > criticPacketsTimer)
		{
			for (std::map<int, CriticPack*>::iterator it = criticPackets.begin(); it != criticPackets.end(); ++it)
			{
				sock.send(it->second->pack, it->second->ip, it->second->port);
			}

			clock.restart();
		}
	}
}

void MovementControlThread()
{
	sf::Clock clock;
	sf::Packet pack;

	pack << static_cast<int>(Protocol::MOVE);

	while (true)
	{
		sf::Time t1 = clock.getElapsedTime();
		if (t1.asSeconds() > percentLostTimer)
		{
			for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
			{
				if (abs(it->second->accumMovement.x) + abs(it->second->accumMovement.y) > 0)
				{
					MovementControl(it->second->id, it->second->lastIdMove);
				}
			}
			clock.restart();
		}
	}
}

void AnswerRegister(sf::IpAddress ip, unsigned short port, sf::Packet pack)
{
	std::string username;
	std::string password;
	std::string email;

	pack >> username >> password >> email;

	pack.clear();

	//Procesado de info del registro, consulta a la BBDD

	//Respuesta a cliente
	pack << static_cast<int>(Protocol::REGISTER);

	if (sock.send(pack, ip, port) != sf::UdpSocket::Status::Done)
		std::cout << "Error al responder al registro." << std::endl;
	else
		std::cout << "Devuelto el mensaje de registro a ip: " << ip.toString() << ", con puerto: " << port << std::endl;

}

void AnswerLogin(sf::IpAddress ip, unsigned short port, sf::Packet pack)
{
	std::string username;
	std::string password;

	pack >> username >> password;

	pack.clear();

	//Procesado de info del login, consulta a la BBDD


	//Respuesta a cliente
	pack << static_cast<int>(Protocol::LOGIN);

	if(sock.send(pack, ip, port) != sf::UdpSocket::Status::Done)
		std::cout << "Error al responder al login." << std::endl;
	else
		std::cout << "Devuelto el mensaje de login a ip: " << ip.toString() << ", con puerto: " << port << std::endl;

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
		//�s un nou player
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

	//introdu�m les dades del player
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
	//pack << static_cast<int>(foodMap.size());
	//for (std::map<int, Food*>::iterator it = foodMap.begin(); it != foodMap.end(); ++it)
	//{
	//	pack << it->second->id;
	//	pack << it->second->position.x;
	//	pack << it->second->position.y;
	//	//pack << it->second->color; //no puc passar el color?
	//}


	sock.send(pack, ip, port);

	//Enviar NEW_PLAYER a todos los dem�s clientes
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
	sf::Vector2f sumPos;
	pack >> sumPos.x >> sumPos.y;

	clientProxies[idPlayer]->accumMovement += sumPos;//en comptes de tractar-lo directament, l'acumulo i es tractar� en el thread
	clientProxies[idPlayer]->lastIdMove = idMove;
}

void MovementControl(int idPlayer, int idMove)
{
	sf::Vector2f possiblePos = clientProxies[idPlayer]->SumToHeadPosition();

	//controlar la posici�
	if (possiblePos.x < 0 || possiblePos.x > SCREEN_WIDTH || possiblePos.y < 0 || possiblePos.y > SCREEN_HEIGHT)
	{
		//el posem just al l�mit de la pantalla? o simplement on era abans de moure's?
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
}

void InitializeFood()
{

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
	return true;

	float f = GetRandomFloat();
	std::cout << "random float is: " << f << std::endl;
	if (f > PERCENT_PACKETLOSS)
	{
		return true;
	}

	return false;
}