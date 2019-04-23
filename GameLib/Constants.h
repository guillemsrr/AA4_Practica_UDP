#pragma once

//casa Guillem
#define IP "192.168.1.128"
//classe
//#define IP "10.40.0.43"

#define PORT 52000

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
	DESCONNECTED
};