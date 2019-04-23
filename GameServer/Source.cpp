#pragma once
#include <iostream>
#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include "ClientProxy.h"
#include "CriticPack.h"
#include <Constants.h>

sf::UdpSocket sock;
std::map<int, ClientProxy*> clientProxies;
std::map<int, CriticPack*> criticPackets;

//declarations:
void NewPlayer(sf::IpAddress ip, unsigned short port, sf::Packet pack);

int main()
{
	if (sock.bind(PORT) != sf::Socket::Status::Done)
	{
		std::cout << "problem on bind" << std::endl;
		return;
	}

	std::cout << "binded";
	while (true)
	{
		sf::Packet pack;
		sf::IpAddress ip;
		unsigned short port;
		if (sock.receive(pack, ip, port) != sf::Socket::Status::Done)
		{
			std::cout << "problem on receiving" << std::endl;
			return;
		}

		std::cout << "received";
		int num;
		pack >> num;
		switch (static_cast<Protocol>(num))
		{
		case HELLO:
			NewPlayer(ip, port, pack);
			break;
		case ACK:
			break;
		case PONG:
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
		if (client->ip == ip && client->port == port)//ja el tenim
		{
			std::cout << "Client exists" << std::endl;
			pack.clear();
			pack << (int)WELCOME;
			pack << client->id;
			sock.send(pack, ip, port);
			return;
		}
	}

	//és un nou player
	int id = 1;// (int)clientProxies.count;
	ClientProxy* newClient = new ClientProxy(id, alias, ip, port);
	clientProxies[id] = newClient;

}