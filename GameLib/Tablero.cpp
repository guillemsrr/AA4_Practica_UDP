#include "Tablero.h"



Tablero::Tablero()
{
	InitTextures();
}


Tablero::~Tablero()
{
}

/**
 * Si guardamos las posiciones de las piezas con valores del 0 al 7,
 * esta funciÃ³n las transforma a posiciÃ³n de ventana (pixel), que va del 0 al 512
 */
sf::Vector2f Tablero::BoardToWindows(sf::Vector2f _position)
{
	return sf::Vector2f(_position.x*LADO_CASILLA + OFFSET_AVATAR, _position.y*LADO_CASILLA + OFFSET_AVATAR);
}


void Tablero::DibujaSFML()
{
	sf::Vector2f casillaOrigen, casillaDestino;
	bool casillaMarcada = false;

	sf::RenderWindow window(sf::VideoMode(960, 540), "TCP Rumble Arena");
	while (window.isOpen())
	{
		MouseControl(&window);

		window.clear();

		DrawMap(&window);
		DrawCharacters(&window);

		window.display();
	}
}

void Tablero::InitTextures()
{
	//Background:
	sf::Texture texture;
	if (!texture.loadFromFile("res/background_arena.jpg"))
	{
		std::cout << "Couldn't load background" << std::endl;
		//return;
	}

	background.setTexture(texture);

	if (!texture.loadFromFile("res/champion.png"))
	{
		std::cout << "Couldn't load background" << std::endl;
		//return;
	}
	warrior1.setTexture(texture);
	warrior1.setPosition(50, 50);
}

void Tablero::MouseControl(sf::RenderWindow* window)
{
	sf::Event event;

	//Este primer WHILE es para controlar los eventos del mouse
	while (window->pollEvent(event))
	{
		switch (event.type)
		{
		case sf::Event::Closed:
			window->close();
			break;

		default:
			break;

		}
	}
}

void Tablero::DrawMap(sf::RenderWindow* window)
{
	window->draw(background);
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			sf::RectangleShape rectBlanco(sf::Vector2f(LADO_CASILLA, LADO_CASILLA));
			rectBlanco.setFillColor(sf::Color::White);
			if (i % 2 == 0)
			{
				//Empieza por el blanco
				if (j % 2 == 0)
				{
					rectBlanco.setPosition(sf::Vector2f(i*LADO_CASILLA, j*LADO_CASILLA));
					window->draw(rectBlanco);
				}
			}
			else
			{
				//Empieza por el negro
				if (j % 2 == 1)
				{
					rectBlanco.setPosition(sf::Vector2f(i*LADO_CASILLA, j*LADO_CASILLA));
					window->draw(rectBlanco);
				}
			}
		}
	}
}

void Tablero::DrawCharacters(sf::RenderWindow * window)
{
	//Para pintar el un circulito
	sf::CircleShape shape(RADIO_AVATAR);
	shape.setFillColor(sf::Color::Blue);
	sf::Vector2f posicion_bolita(4.f, 7.f);
	posicion_bolita = BoardToWindows(posicion_bolita);
	shape.setPosition(posicion_bolita);
	window->draw(shape);
}

