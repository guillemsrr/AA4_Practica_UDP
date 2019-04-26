#pragma once

//casa Guillem
//#define IP "192.168.1.133"
#define IP "127.0.0.1"
//classe
//#define IP "10.40.0.43"

#define PORT 50000

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 540
#define SCREEN_PROVISIONAL 512

#define MAX 100
#define SIZE_TABLERO 64
#define SIZE_FILA_TABLERO 8
#define LADO_CASILLA 64
#define RADIO_AVATAR 25.f
#define OFFSET_AVATAR 5

#define SIZE_TABLERO 64
#define LADO_CASILLA 64
#define RADIO_AVATAR 25.f
#define OFFSET_AVATAR 5

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
	MOVE
};