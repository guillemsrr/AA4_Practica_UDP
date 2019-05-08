#pragma once
#include <random>
//casa Guillem
//#define IP "192.168.1.133"
#define IP "127.0.0.1"
//classe
//#define IP "10.40.0.43"

#define PORT 50000

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 540

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

static float GetRandomFloat()
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<float> dis(0.f,1.f);
	return dis(gen);
}