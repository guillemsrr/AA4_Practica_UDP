#pragma once
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <Constants.h>
#include <vector>
#include "Slither.h"


class Board
{
public:
	Board();
	~Board();
	
	void InitializeSlither(int idPlayer, sf::Vector2f pos);
	void UpdatePlayerPosition(int idPlayer, sf::Vector2f pos);
	void DibujaSFML();
	void Commands();
	sf::RenderWindow window;
	sf::Vector2f playerMovement;
	std::map<int, Slither*> slithersMap;//key is idPlayer

private:
	sf::Vector2f BoardToWindows(sf::Vector2f _position);

	float pixelMove = 5.f;
};

