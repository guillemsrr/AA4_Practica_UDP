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

//timers:
#define CRITICPACKETSTIMER 10.0f
#define PINGTIMER 5.0f
#define PERCENTLOSTTIMER 0.05f
#define DISCONNECTTIMER 10.f
#define MAXUNOPERATIVETIMER 30.f
#define MOVEMENTUPDATETIMER 0.05f
#define FOODUPDATETIMER 0.2f



