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
#include "FoodBall.h"
#include <mutex>


class Board
{
public:
	Board();
	~Board();
	

	void InitializeSlither(Player* player);
	void UpdateSlither(int id);
	void UpdateSlithers();
	void DrawBoard(std::mutex& mtx);

	void Commands(Player* player);
	sf::RenderWindow window;
	sf::Vector2f playerMovement;
	std::map<int, Slither*> slithersMap;//key is idPlayer
	std::vector<sf::Vector2f> foodPositions;
	std::vector<sf::CircleShape*> foodShapeBuffer;

private:
	sf::Clock frameTimeClock;
};

