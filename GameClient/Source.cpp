#pragma once
#include <SFML\Network.hpp>
#include <Tablero.h>
#include <Constants.h>
#include <thread>

sf::UdpSocket sock;
bool received = false;

//declarations:
void Receive_Thread();

int main()
{
	//enviem HELLO al server:
	sf::Packet pack;
	std::string alias = "Guillem";
	pack << (int)HELLO;
	pack << alias;
	float timer = 0;

	std::thread receiver(&Receive_Thread);
	receiver.detach();

	while (!received)
	{
		timer++;
		if (timer > 200)//200 ms?¿
		{
			sock.send(pack, IP, PORT);
			timer = 0;
		}
	}

	std::cout << "received";
	system("pause");
	
	return 0;
}

void Receive_Thread()
{
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

		if (ip == IP && port == PORT)//ES EL SERVER
		{
			int num;
			pack >> num;
			switch (static_cast<Protocol>(num))
			{
			case WELCOME:
				received = true;
				break;
			case NEWPLAYER:
				break;
			case PING:
				break;
			}
		}
	}
}