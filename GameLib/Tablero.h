#pragma once
#include <stdlib.h>
//#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
//#include <wait.h>
#include <stdio.h>
#include <iostream>
#include <SFML/Graphics.hpp>


#define MAX 100
#define SIZE_TABLERO 64
#define SIZE_FILA_TABLERO 8
#define LADO_CASILLA 64
#define RADIO_AVATAR 25.f
#define OFFSET_AVATAR 5

class Tablero
{
public:
	Tablero();
	~Tablero();

	char tablero[SIZE_TABLERO];
	void DibujaSFML();

private:
	sf::Vector2f BoardToWindows(sf::Vector2f _position);
	void MouseControl(sf::RenderWindow* window);
	void DrawMap(sf::RenderWindow* window);
	void DrawCharacters(sf::RenderWindow* window);
	void InitTextures();

	//sprites:
	sf::Sprite background;
	sf::Sprite warrior1;
	sf::Sprite warrior2;
	sf::Sprite warrior3;
	sf::Sprite warrior4;

};

