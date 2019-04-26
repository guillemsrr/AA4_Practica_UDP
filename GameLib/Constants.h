#pragma once

//casa Guillem
//#define IP "192.168.1.128"
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
	NEWPLAYER,
	ACK,
	PING,
	PONG,
	CMD,
	DISCONNECTED
};