#include "Board.h"



Board::Board()
{
	
}


Board::~Board()
{
}

void Board::InitializePlayerPosition(sf::Vector2f playerPos)
{
	playerPositions.push_back(playerPos);
}

void Board::UpdatePlayerPosition(int playerNum, sf::Vector2f pos)
{
}

/**
 * Si guardamos las posiciones de las piezas con valores del 0 al 7,
 * esta funciÃ³n las transforma a posiciÃ³n de ventana (pixel), que va del 0 al 512
 */
sf::Vector2f Board::BoardToWindows(sf::Vector2f _position)
{
	return sf::Vector2f(_position.x * LADO_CASILLA + OFFSET_AVATAR, _position.y * LADO_CASILLA + OFFSET_AVATAR);
}

/**
 * Contiene el cÃ³digo SFML que captura el evento del clic del mouse y el cÃ³digo que pinta por pantalla
 */
void Board::DibujaSFML()
{
	sf::RenderWindow window(sf::VideoMode(SCREEN_PROVISIONAL, SCREEN_PROVISIONAL), "Ejemplo tablero");
	while (window.isOpen())
	{
		window.clear();

		//A partir de aquÃ­ es para pintar por pantalla
		//Este FOR es para el tablero
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
						rectBlanco.setPosition(sf::Vector2f(i * LADO_CASILLA, j * LADO_CASILLA));
						window.draw(rectBlanco);
					}
				}
				else
				{
					//Empieza por el negro
					if (j % 2 == 1)
					{
						rectBlanco.setPosition(sf::Vector2f(i * LADO_CASILLA, j * LADO_CASILLA));
						window.draw(rectBlanco);
					}
				}
			}
		}

		//pintar los players (provisional)
		for (int i = 0; i < playerPositions.size(); i++)
		{
			sf::CircleShape shape(RADIO_AVATAR);
			
			if (i == 0)
			{
				shape.setFillColor(sf::Color::Blue);
			}
			else
			{
				shape.setFillColor(sf::Color::Red);
			}

			shape.setPosition(BoardToWindows(playerPositions[i]));
			window.draw(shape);
		}


		window.display();
	}

}
