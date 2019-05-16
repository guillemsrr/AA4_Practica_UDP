#pragma once
#include <random>
#include <iostream>


//casa Guillem
//#define IP "192.168.1.133"
#define IP "127.0.0.1"
//classe
//#define IP "10.40.0.43"

#define PORT 50000

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 540

#define PERCENT_PACKETLOSS 0.1f

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
	FOOD_EATEN,
	FOOD_SPAWNED
};