#pragma once
#include <random>
#include <iostream>


#define IP "127.0.0.1"
#define PORT 50000

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 540

#define PERCENT_PACKETLOSS 0.05f

enum Protocol
{
	HELLO,
	WELCOME,
	NEW_PLAYER,
	ACK,
	PING,
	PONG,
	CMD,
	DISCONNECTED,
	MOVE,
	FOOD_UPDATE
};