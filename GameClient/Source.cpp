#pragma once
#include <SFML/Graphics.hpp>
#include <SFML\Network.hpp>
#include <Tablero.h>
#include <Constants.h>
#include <thread>
#include "PlayerInfo.h"

//---------CLIENTE---------//


sf::UdpSocket sock;
bool received = false;

PlayerInfo playerSelf;



void Receive_Thread()
{
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
			std::cout << "Packet received" << std::endl;

			if (ip == IP && port == PORT)//ES EL SERVER
			{
				int num;
				pack >> num;
				switch (static_cast<Protocol>(num))
				{
				case WELCOME:
					std::cout << "WELCOME received" << std::endl;
					received = true;
					pack >> playerSelf.id;

					break;
				case NEWPLAYER:
					break;
				case PING:
					std::cout << "PING received" << std::endl;
					pack.clear();
					pack << static_cast<int>(Protocol::PONG);
					pack << playerSelf.id;
					sock.send(pack, IP, PORT);
					break;
				case DISCONNECTED:
					std::cout << "DISCONNECTED received" << std::endl;
					int auxIdPack;
					int auxIdPlayer;
					pack >> auxIdPack >> auxIdPlayer;
					pack.clear();
					pack << static_cast<int>(Protocol::ACK);
					pack << auxIdPack;
					sock.send(pack, IP, PORT);
					break;
				}
			}
		}

		
	}
}

sf::Vector2f BoardToWindows(sf::Vector2f _position)
{
	return sf::Vector2f(_position.x*LADO_CASILLA + OFFSET_AVATAR, _position.y*LADO_CASILLA + OFFSET_AVATAR);
}

void DibujaSFML()
{
	sf::Vector2f casillaOrigen, casillaDestino;
	bool casillaMarcada = false;

	sf::RenderWindow window(sf::VideoMode(512, 512), "Ejemplo tablero");
	while (window.isOpen())
	{
		sf::Event event;

		//Este primer WHILE es para controlar los eventos del mouse
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close();
				break;

			default:
				break;

			}
		}

		window.clear();

		//A partir de aquí es para pintar por pantalla
		//Este FOR es para el tablero
		for (int i = 0; i<8; i++)
		{
			for (int j = 0; j<8; j++)
			{
				sf::RectangleShape rectBlanco(sf::Vector2f(LADO_CASILLA, LADO_CASILLA));
				rectBlanco.setFillColor(sf::Color::White);
				if (i % 2 == 0)
				{
					//Empieza por el blanco
					if (j % 2 == 0)
					{
						rectBlanco.setPosition(sf::Vector2f(i*LADO_CASILLA, j*LADO_CASILLA));
						window.draw(rectBlanco);
					}
				}
				else
				{
					//Empieza por el negro
					if (j % 2 == 1)
					{
						rectBlanco.setPosition(sf::Vector2f(i*LADO_CASILLA, j*LADO_CASILLA));
						window.draw(rectBlanco);
					}
				}
			}
		}

		//Para pintar el un circulito
		sf::CircleShape shape(RADIO_AVATAR);
		shape.setFillColor(sf::Color::Blue);
		sf::Vector2f posicion_bolita(4.f, 7.f);
		posicion_bolita = BoardToWindows(posicion_bolita);
		shape.setPosition(posicion_bolita);
		window.draw(shape);




		window.display();
	}

}



int main()
{
	//Enviamos HELLO al servidor
	sf::Packet pack;
	std::string alias = "Guillem";
	pack << static_cast<int>(Protocol::HELLO);
	pack << alias;
	float timer = 0;

	std::thread receiver(&Receive_Thread);
	receiver.detach();

	sf::Clock clock;

	while (!received)
	{
		
		sf::Time t1 = clock.getElapsedTime();
		if (t1.asSeconds() > 2.0f)
		{
			std::cout << "Entro" << std::endl;
			if (sock.send(pack, IP, PORT) != sf::UdpSocket::Status::Done)
			{
				std::cout << "Error sending the packet" << std::endl;
			}
			else
			{
				std::cout << "Paquete enviado" << std::endl;
			}
			clock.restart();
		}
			

	}

	
	DibujaSFML();
	return 0;
}



