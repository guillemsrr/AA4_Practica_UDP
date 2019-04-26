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
std::map<int, sf::Packet> criticPackets;

//declarations:
void NewPlayer(sf::IpAddress ip, unsigned short port, sf::Packet pack);

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
			std::cout << "Checking for disconnected players" << std::endl;
			//Comprobar si el numPings supera el límite y enviar el DISCONNECTED a todos los demás clientes
			for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); /*++it*/)
			{
				if (it->second->numPings >= 30)
				{
					//Añadir el paquete a la lista de críticos
					pack << static_cast<int>(Protocol::DISCONNECTED);
					pack << criticPackets.size();
					pack << it->second->id;
					criticPackets[criticPackets.size()] = pack;					

					//Enviar DISCONNECTED
					for (std::map<int, ClientProxy*>::iterator it2 = clientProxies.begin(); it2 != clientProxies.end(); ++it2)
					{
						if (it2 != it)
						{
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


	while (true)
	{
		sf::Packet pack;
		sf::IpAddress ip;
		unsigned short port;
		if (sock.receive(pack, ip, port) != sf::Socket::Status::Done)
		{
			//std::cout << "Error on receiving packet ip:" << ip.toString() << std::endl;
		}

		std::cout << "Packet received" << std::endl;
		int num;
		pack >> num;
		switch (static_cast<Protocol>(num))
		{
		case HELLO:
			NewPlayer(ip, port, pack);
			break;
		case ACK:
			std::cout << "ACK received" << std::endl;
			break;
		case PONG:
			int pId;
			pack >> pId;
			clientProxies[pId]->numPings = 0;
			break;
		case CMD:

			break;
		}
	}

	return 0;
}

void NewPlayer(sf::IpAddress ip, unsigned short port, sf::Packet pack)
{
	std::string alias;
	pack >> alias;

	//busquem per ip i port:
	for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
	{
		ClientProxy* client = it->second;
		if (client->ip == ip && client->port == port)//ya lo tenemos
		{
			std::cout << "Client exists" << std::endl;
			pack.clear();
			pack << static_cast<int>(Protocol::WELCOME);
			pack << client->id;
			sock.send(pack, ip, port);
			return;
		}
	}

	//és un nou player
	int id = clientProxies.size();
	ClientProxy* newClient = new ClientProxy(id, alias, ip, port);
	clientProxies[id] = newClient;
	pack.clear();
	pack << static_cast<int>(Protocol::WELCOME);
	pack << clientProxies[id]->id;
	sock.send(pack, ip, port);

}