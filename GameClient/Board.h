#pragma once
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <Constants.h>
#include "Player.h"
#include "Slither.h"


class Board
{
public:
	Board();
	~Board();
	
	void InitializeSlither(Player* player);
	void UpdateSlither(int id);
	void UpdateSlithers();
	void DrawBoard();
	void Commands();
	sf::RenderWindow window;
	sf::Vector2f playerMovement;
	std::map<int, Slither*> slithersMap;//key is idPlayer
};

