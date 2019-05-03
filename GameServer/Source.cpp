#pragma once
#include <iostream>
#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include "ClientProxy.h"
#include "CriticPack.h"
#include <Constants.h>
#include <thread>


//---------SERVIDOR---------//


sf::UdpSocket sock;
std::map<int, ClientProxy*> clientProxies;
std::map<int, CriticPack*> criticPackets;
float criticPacketsTimer = 10.0f;

//declarations:
void NewPlayer(sf::IpAddress ip, unsigned short port, sf::Packet pack);
void MovementControl(sf::Packet pack);

void PingThreadFunction()
{
	sf::Clock clock;
	sf::Packet pack;

	pack << static_cast<int>(Protocol::PING);


	while (true)
	{
		sf::Time t1 = clock.getElapsedTime();
		if (t1.asSeconds() > 2.0f)
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
		if (t1.asSeconds() > 10.0f)
		{
			//std::cout << "Checking for disconnected players" << std::endl;
			//Comprobar si el numPings supera el límite y enviar el DISCONNECTED a todos los demás clientes
			for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); /*++it*/)
			{
				if (it->second->numPings >= 30)
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

int main()
{
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


	while (true)
	{
		sf::Packet pack;
		sf::IpAddress ip;
		unsigned short port;
		if (sock.receive(pack, ip, port) != sf::Socket::Status::Done)
		{
			std::cout << "Error on receiving packet ip:" << ip.toString() << std::endl;
		}

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
			//std::cout << "num criticPackets before: " << (int)criticPackets.size() << std::endl;
			criticPackets.erase(criticPackets.find(auxIdPack));
			//std::cout << "num criticPackets after: " << (int)criticPackets.size() << std::endl;
			break;
		case PONG:
			//std::cout << "PONG received" << std::endl;
			int pId;
			pack >> pId;
			clientProxies[pId]->numPings = 0;
			break;
		case MOVE:
			//std::cout << "MOVE received" << std::endl;
			MovementControl(pack);
			break;
		}
	}

	return 0;
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
	
	//els altres players:
	pack << static_cast<int>(clientProxies.size());
	for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
	{
		if (it->second->id != newClient->id)
		{
			it->second->AddDataToPacket(&pack);
		}
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

void MovementControl(sf::Packet pack)
{
	int idPlayer;
	int idMove;
	pack >> idPlayer>>idMove;
	sf::Vector2f sumPos;
	pack >> sumPos.x >> sumPos.y;

	sf::Vector2f possiblePos = clientProxies[idPlayer]->SumToHeadPosition(sumPos);
	//controlar la posició
	//marges de la pantalla?

	//movem el player
	clientProxies[idPlayer]->UpdatePosition(possiblePos);

	//ho enviem a tots els players
	pack.clear();
	pack << static_cast<int>(Protocol::MOVE);
	pack << idPlayer;
	pack << idMove;
	clientProxies[idPlayer]->PutBodyPositions(&pack);

	for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
	{
		sock.send(pack, it->second->ip, it->second->port);
	}
}