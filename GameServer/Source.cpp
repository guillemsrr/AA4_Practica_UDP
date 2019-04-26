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
					int idPack = criticPackets.size();
					pack << idPack;
					pack << it->second->id;				

					//Enviar DISCONNECTED
					for (std::map<int, ClientProxy*>::iterator it2 = clientProxies.begin(); it2 != clientProxies.end(); ++it2)
					{
						if (it2 != it)
						{

							CriticPack* cp = new CriticPack(criticPackets.size(), pack, it2->second->ip, it2->second->port);
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

	//busquem per ip i port si el client ja existeix:
	for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
	{
		ClientProxy* client = it->second;
		if (client->ip == ip && client->port == port)
		{
			std::cout << "Client already exists" << std::endl;
			pack.clear();
			pack << static_cast<int>(Protocol::WELCOME);
			pack << client->id;
			pack << client->pos.x;
			pack << client->pos.y;
			sock.send(pack, ip, port);
			return;
		}
	}

	//és un nou player
	int id = clientProxies.size();
	int x = id % SIZE_FILA_TABLERO;
	int y = id % SIZE_FILA_TABLERO;
	ClientProxy* newClient = new ClientProxy(id, alias, x, y, ip, port);
	pack.clear();
	pack << static_cast<int>(Protocol::WELCOME);
	pack << id;
	pack << x;
	pack << y;
	pack << static_cast<int>(clientProxies.size());

	for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
	{
		if (it->second->id != id)
		{
			pack << it->second->id;
			pack << it->second->alias;
			pack << it->second->pos.x;
			pack << it->second->pos.y;
		}
	}
	sock.send(pack, ip, port);

	//Enviar NEW_PLAYER a todos los demás clientes
	for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
	{
		//com que cada vegada hem d'anar posant un id del packet diferent, anem tornant a implir el packet
		pack.clear();
		pack << static_cast<int>(Protocol::NEW_PLAYER);
		pack << id;
		pack << alias;
		pack << x;
		pack << y;
		int idPack = criticPackets.size();
		pack << idPack;

		CriticPack* cp = new CriticPack(criticPackets.size(), pack, it->second->ip, it->second->port);
		criticPackets[idPack] = cp;
		sock.send(pack, it->second->ip, it->second->port);
	}

	//afegim el nou client en el map
	clientProxies[id] = newClient;
}

void MovementControl(sf::Packet pack)
{
	int idPlayer;
	int idMove;
	pack >> idPlayer>>idMove;
	sf::Vector2f pos;
	pack >> pos.x >> pos.y;

	//controlar la posició
	//marges de la pantalla?

	clientProxies[idPlayer]->pos += pos;

	//ho enviem a tots els players
	pack.clear();
	pack << static_cast<int>(Protocol::MOVE);
	pack << idPlayer;
	pack << idMove;
	pack << clientProxies[idPlayer]->pos.x;
	pack << clientProxies[idPlayer]->pos.y;
	for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
	{
		sock.send(pack, it->second->ip, it->second->port);
	}
}