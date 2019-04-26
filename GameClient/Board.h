#pragma once
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <Constants.h>
#include <vector>


class Board
{
public:
	Board();
	~Board();
	
	void InitializePlayerPosition(sf::Vector2f playerPos);
	void UpdatePlayerPosition(int playerNum, sf::Vector2f pos);
	void DibujaSFML();
	void Commands();
	sf::RenderWindow window;
	sf::Vector2f playerMovement;
	std::vector<sf::Vector2f> playerPositions;//number 0 is the player, the rest are enemies

private:
	sf::Vector2f BoardToWindows(sf::Vector2f _position);

	float pixelMove = 5.f;
};

