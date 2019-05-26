#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML\Network.hpp>
#include "Board.h"
#include <Constants.h>
#include <thread>
#include "Player.h"
#include <mutex>
#include "FoodBall.h"
#include <chrono>
#include "SceneObjects.h"

//---------CLIENTE---------//

//client data:
std::string username;
std::string password;
std::string email;


sf::UdpSocket sock;
bool received = false;
bool startGame = false;
bool registerResponse = false;
bool loginResponse = false;
bool loginOrRegister = true;
int codigoRegistro = 0;
int codigoLogin = 0;
std::mutex mtx_food;
std::mutex mtx_bodies;
int actualMoveID = 0;

//maps:
std::map<int, Player*> playersMap;
std::map<int, sf::Vector2f> movesMap;//idMove i headPos
std::map<int, std::vector<sf::Vector2f>> interpolationsMap;
std::map<int, sf::Packet> criticPackets;

Player* m_player;
sf::Vector2f accumMove;
Board board;

//timers:
const float helloSendingTimer = 2.f;
const float registerSendingTimer = 1.f;
const float movementTimer = 0.1f;
const float criticResendTimer = 2.f;
const float interpolationTimer = 0.001f;

//declarations:
void HelloSending();
void GraphicsInterface();
void MoveSending();
void SendAcknowledge(int idPack);
void InterpolatePositions();
void RegisterUser();
void LoginUser();
void RegisterLoginThreadFunction();
void CriticPacketsManagerThreadFunction();
void FindGameFunction();

enum class SceneStage
{
	LOGIN,
	SKIN_SELECT,
	GAME,
	DEATH,
	WAITINGFORGAME,
	WIN
};

SceneStage sceneStage = SceneStage::LOGIN;
sf::Vector2i mouseCoords;
bool LMBPressed = false;
bool LMBDown = false;

const std::string btnRedID = "redBtn";
const std::string btnOrgID = "orgBtn";
const std::string btnYlwID = "ylwBtn";
const std::string btnGrnID = "grnBtn";
const std::string btnTrqID = "trqBtn";
const std::string btnCynID = "cynBtn";
const std::string btnIdgID = "idgBtn";
const std::string btnVltID = "vltBtn";
const std::string btnBackToSkinSelectionID = "backToSkinSelBtn";

std::string lastPressedButtonID = "None";

int main()
{
	//Enviamos HELLO al servidor
	std::thread helloThread(&HelloSending);
	helloThread.detach();

	//Crear thread para administrar paquetes críticos
	std::thread criticPacketsManagerThread(&CriticPacketsManagerThreadFunction);
	criticPacketsManagerThread.detach();


	while (true)
	{
		sf::Packet pack;
		sf::IpAddress ip;
		unsigned short port;
		if (sock.receive(pack, ip, port) != sf::UdpSocket::Status::Done)
		{
			//std::cout << "Error while receiving packet" << std::endl;
		}
		else
		{
			//std::cout << "Packet received" << std::endl;
			if (ip == IP && port == PORT)//ES EL SERVER
			{
				int num;
				int auxID;
				pack >> num;
				switch (static_cast<Protocol>(num))
				{
				case WELCOME:
				{
					//Crear jugador propio
					mtx_bodies.lock();
					m_player = new Player(&pack);
					mtx_bodies.unlock();
					playersMap[m_player->appId] = m_player;

					std::cout << "WELCOME player "<< m_player->appId << std::endl;


					//Dar paso al registro y/o login

					std::thread RegisterLoginThread(RegisterLoginThreadFunction);
					RegisterLoginThread.detach();

					received = true;

					std::thread graphicsInterface(&GraphicsInterface);
					graphicsInterface.detach();

				}
					break;
				case NEW_PLAYER:
				{
					mtx_bodies.lock();
					Player* p = new Player(&pack);
					mtx_bodies.unlock();
					playersMap[p->appId] = p;
					board.InitializeSlither(p);

					int idPack;
					pack >> idPack;
					SendAcknowledge(idPack);
				}
					break;
				case PING:
				{
					//std::cout << "PING received" << std::endl;
					pack.clear();
					pack << static_cast<int>(Protocol::PONG);
					pack << m_player->appId;
					sock.send(pack, IP, PORT);
				}
					break;
				case DISCONNECTED:
				{
					std::cout << "DISCONNECTED received" << std::endl;
					int idPack;
					int idPlayer;
					pack >> idPack >> idPlayer;
					//PLAYER IDPLAYER DISCONNECTED

					SendAcknowledge(idPack);
				}
					break;
				case MOVE:
				{
					//std::cout << "MOVE received" << std::endl;
					int idPlayer;
					int idMove;
					pack >> idPlayer >> idMove;

					if (idPlayer == m_player->appId)
					{
						//std::cout << "dentro idPlayer == m_player->id" << std::endl;
						//vector on posem els moviments que volem esborrar
						std::vector<std::map<int, sf::Vector2f>::iterator> toErase;
						
						if (movesMap.find(idMove) != movesMap.end())//si existeix el idMove
						{
							//RECONCILIACIO:
							//std::cout << "reconciliacio" << std::endl;
							//comprovem en quina posici� est�vem / estem, i si coincideix
							int numPos;
							int x, y;
							pack >> numPos;
							pack >> x;
							pack >> y;

							sf::Vector2f headPos;
							headPos.x = (float)x / 1000.f;
							headPos.y = (float)y / 1000.f;

							float tol = 1;// sqrt(2) / 10.f;
							sf::Vector2f sub = movesMap[idMove] - headPos;
							sub.x = abs(sub.x);
							sub.y = abs(sub.y);
							if (sqrt(sub.x*sub.x + sub.y*sub.y) > tol)
							{
								//std::cout << "Position modified!" << std::endl;
								//std::cout << "movesMap y: "<< (float)movesMap[idMove].y << " head pos y " << (float)headPos.y << std::endl;

								mtx_bodies.lock();
								m_player->UpdateTheRestOfPositions(numPos, headPos, &pack);
								board.UpdateSlither(idPlayer);
								mtx_bodies.unlock();

								accumMove = sf::Vector2f(0.0f, 0.0f);
								/*bool x = (float)movesMap[idMove].x != (float)headPos.x;
								bool y = ((float)movesMap[idMove].y != (float)headPos.y);
								std::cout << "x bool: " << x << std::endl;
								std::cout << "y bool: " << y << std::endl;
								std::cout << "headPos: (x "<< headPos.x << " , y " << headPos.y << std::endl;
								std::cout << "movesMap[idMove]: (x "<< movesMap[idMove].x << " , y " << movesMap[idMove].y << std::endl;*/
							}
							else if (numPos > (int)m_player->bodyPositions.size())
							{
								mtx_bodies.lock();
								while (numPos > (int)m_player->bodyPositions.size())
								{
									m_player->CreateBodyPosition(false, sf::Vector2f(0.0f, 0.0f));
								}

								board.UpdateSlither(idPlayer);
								mtx_bodies.unlock();
							}
							else
							{
								//std::cout << "good position" << std::endl;
							}
							//std::cout << "after" << std::endl;
							//esborrem els moviments anteriors posant-los a toErase
							for (std::map<int, sf::Vector2f>::iterator it = movesMap.begin(); it != movesMap.end(); ++it)
							{
								if (it->first < idMove)
								{
									toErase.push_back(it);
								}
							}

							while ((int)toErase.size() != 0)
							{
								movesMap.erase(toErase[0]);
								toErase.erase(toErase.begin());
								//std::cout << "erasing" << std::endl;
							}
						}

						//std::cout << "body positions: " << (int)m_player->bodyPositions.size() << std::endl;
					}
					else
					{
						//just save the final position to interpolate!
						//std::cout << "saved move to interpolation" << std::endl;
						mtx_bodies.lock();
						interpolationsMap[idPlayer] = playersMap[idPlayer]->GetFuturePositions(&pack);
						mtx_bodies.unlock();
					}
				}
					break;
				case FOOD_UPDATE:
				{
					if (!received)
					{
						break;
					}

					//std::lock_guard<std::mutex> guard(mtx);

					//update food map

					int numFood;
					pack >> numFood;
					//std::cout << "num balls: " << numFood << std::endl;
					std::vector<sf::Vector2f> foodPositions;
					for (int i = 0; i < numFood; i++)
					{
						sf::Vector2f pos;
						pack >> pos.x;
						pack >> pos.y;
						foodPositions.push_back(pos);
					}
					
					if (true)
					{
						std::lock_guard<std::mutex> guard(mtx_food);
						board.foodPositions = foodPositions;
					}
					//std::cout << "foodPositions size: " << foodPositions.size() << std::endl;
				}
					break;

				case KILL:
				{
					int idPlayer;
					pack >> idPlayer;

					if (idPlayer == m_player->appId)//myself
					{
						//std::lock_guard<std::mutex> guard(mtx_bodies);
						//m_player->bodyPositions.clear();
						//board.UpdateSlither(idPlayer);
						m_player->dead = true;

						std::cout << "YOU DIED" << std::endl;
						sceneStage = SceneStage::DEATH;
					}
					else//other player killed
					{
						//std::lock_guard<std::mutex> guard(mtx_bodies);
						//playersMap[idPlayer]->bodyPositions.clear();
						//board.UpdateSlither(idPlayer);
						playersMap[idPlayer]->dead = true;

						std::cout << "PLAYER KILLED" << std::endl;
					}
				}
					break;
				case WIN:
				{
					sceneStage = SceneStage::WIN;
				}
					break;
				case REGISTER:
				{
					pack >> codigoRegistro;

					std::cout << "Recibo confirmacion del registro con codigo: " << codigoRegistro << std::endl;

					registerResponse = true;
				}
					break;
				case LOGIN:
				{
					pack >> codigoLogin;

					if (codigoLogin == 1)
					{
						loginResponse = true;
						std::cout << "Sesion Iniciada: " << codigoLogin << std::endl;

						//Ir al menu de skins/play
						sceneStage = SceneStage::SKIN_SELECT;
					}
					else
					{
						loginResponse = true;
						std::cout << "Error al iniciar sesion: " << codigoLogin << std::endl;

					}

				}
					break;
				case ACK:
					pack >> auxID;
					std::cout << "Acknowledge recibido con idPacket: " << auxID << std::endl;

					criticPackets.erase(criticPackets.find(auxID));
					break;
				case STARTGAME:
					//Me pasan la informacion de todos los players de la partida, incluido yo mismo
					int size;
					pack >> size;

					for (int i = 0; i < size; i++)
					{
						mtx_bodies.lock();
						Player* p = new Player(&pack);
						mtx_bodies.unlock();
						if (m_player->appId == p->appId)
						{
							//Sustituir datos de ownPlayer
							delete m_player;
							m_player = p;
						}
						playersMap[p->appId] = p;
						
					}

					sceneStage = SceneStage::GAME;


					std::thread accumControlThread(&MoveSending);
					accumControlThread.detach();

					std::thread interpolationsThread(&InterpolatePositions);
					interpolationsThread.detach();

					break;
				break;
				}
			}
		}
	}

	return 0;
}

void CriticPacketsManagerThreadFunction()
{
	//sf::Clock clock;

	while (true)
	{
		//sf::Time t1 = clock.getElapsedTime();

		//if (t1.asSeconds() > CRITICPACKETSTIMER)
		//{
		for (std::map<int, sf::Packet>::iterator it = criticPackets.begin(); it != criticPackets.end(); ++it)
		{
			sock.send(it->second, IP, PORT);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds((int)(CRITICPACKETSTIMER * 1000)));
		//clock.restart();
	//}
	}
}

void RegisterLoginThreadFunction()
{
	std::string accion;

	do
	{
		std::cout << "Registrarse(Register) o Iniciar Sesión(Login): ";
		std::cin >> accion;

		if (accion == "Register")
		{
			//Recoger datos de formulario
			std::cout << "Introduce el nombre de usuario deseado: ";
			std::cin >> username;
			std::cout << std::endl;

			std::cout << "Introduce tu contraseña deseada: ";
			std::cin >> password;
			std::cout << std::endl;

			std::cout << "Introduce tu email: ";
			std::cin >> email;
			std::cout << std::endl;

			//loginOrRegister = false;

			RegisterUser();
		}
		else if (accion == "Login")
		{
			//Recoger datos de formulario
			std::cout << "Introduce el nombre de usuario deseado: ";
			std::cin >> username;
			std::cout << std::endl;

			std::cout << "Introduce tu contraseña deseada: ";
			std::cin >> password;
			std::cout << std::endl;

			//loginOrRegister = false;

			LoginUser();
		}
		else
		{
			//loginOrRegister = true;
		}



	} while (loginOrRegister);


	//Menu principal
	startGame = true;
}

void RegisterUser()
{
	//Utilizar cabecera REGISTER, con username, password, email.
	sf::Packet packRegister;
	packRegister << static_cast<int>(Protocol::REGISTER);
	packRegister << static_cast<int>(criticPackets.size()) << username << password << email;
		
			//Enviar paquete a servidor cada x segundos, hasta que  me diga ok
			criticPackets[criticPackets.size()] = packRegister;
			/*if (sock.send(packRegister, IP, PORT) != sf::UdpSocket::Status::Done)
				std::cout << "Error al enviar el registro" << std::endl;
			else
				std::cout << "Registro enviado" << std::endl;*/

	//loginOrRegister = false;

}

void LoginUser()
{
	//Utilizar cabecera LOGIN con username y password.
	sf::Packet packetLogin;
	packetLogin << static_cast<int>(Protocol::LOGIN);
	packetLogin << static_cast<int>(criticPackets.size()) << username << password;

			//Enviar paquete a servidor cada x segundos, hasta que me diga ok
			criticPackets[criticPackets.size()] = packetLogin;
			//if (sock.send(packetLogin, IP, PORT) != sf::UdpSocket::Status::Done)
			//	std::cout << "Error al enviar el login" << std::endl;
			//else
			//	std::cout << "Login enviado" << std::endl;




	loginOrRegister = false;
}

void HelloSending()
{
	//sf::Clock clock;
	sf::Packet pack;
	std::string alias = "Default Alias";
	pack << static_cast<int>(Protocol::HELLO);
	pack << alias;

	while (!received)
	{
		//sf::Time t1 = clock.getElapsedTime();
		//if (t1.asSeconds() > helloSendingTimer)
		//{
			if (sock.send(pack, IP, PORT) != sf::UdpSocket::Status::Done)
			{
				std::cout << "Error sending the packet" << std::endl;
			}
			else
			{
				std::cout << "HELLO enviado" << std::endl;
			}
			//clock.restart();

			//std::cout << "THREADS: HELLO going to sleep for " << helloSendingTimer * 1000 << " Milliseconds." << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds((int)(1000 * helloSendingTimer)));
			//std::cout << "THREADS: HELLO awakened" << std::endl;
		//}
	}
}

void UpdateButtonAppearance(UIButton & btn, std::string bt)
{
	if (btn.GetInteractable())
	{
		if (btn.CheckCursorCollision(mouseCoords))
		{
			if (LMBDown)
			{
				btn.SetPressed();
				lastPressedButtonID = bt;
			}
			else if (!LMBPressed)
			{
				btn.SetHover();
			}
		}
		else
		{
			btn.SetDefault();
		}
	}
}

///Falta implementar
void ButtonFunctionality()
{
	bool skinSelected = false;

	if (lastPressedButtonID == btnBackToSkinSelectionID)
	{
		//From Death screen to Skin selection
		playersMap.clear();
		playersMap[m_player->appId] = m_player;
		board.slithersMap.clear();
		interpolationsMap.clear();
		sceneStage = SceneStage::SKIN_SELECT;
	}
	else if (lastPressedButtonID == btnRedID)
	{
		//Select Red skin...
		m_player->SetPlayerColor(SkinColors::RED);
		skinSelected = true;
	}
	else if (lastPressedButtonID == btnOrgID)
	{
		//Select Orange skin...
		m_player->SetPlayerColor(SkinColors::ORANGE);
		skinSelected = true;
	}
	else if (lastPressedButtonID == btnYlwID)
	{
		//Select Yellow skin...
		m_player->SetPlayerColor(SkinColors::YELLOW);
		skinSelected = true;
	}
	else if (lastPressedButtonID == btnGrnID)
	{
		//Select Green skin...
		m_player->SetPlayerColor(SkinColors::GREEN);
		skinSelected = true;
	}
	else if (lastPressedButtonID == btnTrqID)
	{
		//Select Turquoise skin...
		m_player->SetPlayerColor(SkinColors::TURQUOISE);
		skinSelected = true;
	}
	else if (lastPressedButtonID == btnCynID)
	{
		//Select Cyan skin...
		m_player->SetPlayerColor(SkinColors::LIGHTBLUE);
		skinSelected = true;
	}
	else if (lastPressedButtonID == btnIdgID)
	{
		//Select Indigo skin...
		m_player->SetPlayerColor(SkinColors::INDIGO);
		skinSelected = true;
	}
	else if (lastPressedButtonID == btnVltID)
	{
		//Select Violet skin...
		m_player->SetPlayerColor(SkinColors::VIOLET);
		skinSelected = true;
	}

	if (skinSelected)
	{
		//Start/join game
		FindGameFunction();

		sceneStage = SceneStage::WAITINGFORGAME;
	}
}

void FindGameFunction()
{
	//Enviar al servidor que quiero buscar una partida
	sf::Packet packFindGame;
	packFindGame << static_cast<int>(Protocol::FINDGAME);
	packFindGame << static_cast<int>(criticPackets.size()) << m_player->appId;
	packFindGame << static_cast<int>(m_player->skinColor);


	criticPackets[criticPackets.size()] = packFindGame;

}

void GraphicsInterface()
{
	SceneStage lastSceneStage = sceneStage;
	bool startFlag = true;
	//while (!startGame)
	//{
	//	//std::cout << "NOT STARTING YET" << std::endl;
	//	//just don't start
	//}

	SceneObjects sceneObjs;

	sf::Font font;

	if (!font.loadFromFile("courbd.ttf"))
	{
		std::cout << "Can't load the font file" << std::endl;
	}

	board.window.create(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Slither Remake");
	while (board.window.isOpen())
	{
		if (lastSceneStage != sceneStage || startFlag)
		{
			lastPressedButtonID = "None";
			sceneObjs.buttonIDs.clear();
			sceneObjs.buttons.clear();
			sceneObjs.standaloneRects.clear();
			sceneObjs.texts.clear();
					   
			switch (sceneStage)
			{
			case SceneStage::LOGIN:
				sceneObjs.AddStandaloneRect(sf::Vector2f(SCREEN_WIDTH * .8f, SCREEN_HEIGHT * .8f), sf::Vector2f(SCREEN_WIDTH * .1f, SCREEN_HEIGHT * .1f), MENUS_BG_RECT_COLOR, 10, BTN_DEF_COLOR);
				sceneObjs.AddText("Log in through the console", sf::Color::White, font, 3, 15, sf::Vector2i(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2), sf::Vector2f(.5f, .5f));
				break;
			case SceneStage::SKIN_SELECT:
				sceneObjs.AddStandaloneRect(sf::Vector2f(SCREEN_WIDTH * .8f, SCREEN_HEIGHT * .8f), sf::Vector2f(SCREEN_WIDTH * .1f, SCREEN_HEIGHT * .1f), MENUS_BG_RECT_COLOR, 10, BTN_DEF_COLOR);
				sceneObjs.AddText("Pick a skin", sf::Color::White, font, 3, 15, sf::Vector2i(SCREEN_WIDTH / 2, SCREEN_HEIGHT * .2f), sf::Vector2f(.5f, .5f));

				sceneObjs.AddButton(btnRedID, sf::Vector2f(SCREEN_WIDTH * .2f * .8f, SCREEN_HEIGHT * .2f * .8f), sf::Vector2i(SCREEN_WIDTH * .2f, SCREEN_HEIGHT * .4f), sf::Vector2f(.5f, .5f), font);
				sceneObjs.buttons[btnRedID].OverrideBtnInnerColors(BTN_RED_DEF_COLOR, BTN_RED_HOV_COLOR, BTN_RED_CLK_COLOR, BTN_RED_DIS_COLOR);
				sceneObjs.buttons[btnRedID].OverrideBtnOutlineColors(CBTN_OUTLINE_COLOR);
				sceneObjs.buttons[btnRedID].showText = false;

				sceneObjs.AddButton(btnOrgID, sf::Vector2f(SCREEN_WIDTH * .2f * .8f, SCREEN_HEIGHT * .2f * .8f), sf::Vector2i(SCREEN_WIDTH * .2f, SCREEN_HEIGHT * .6f), sf::Vector2f(.5f, .5f), font);
				sceneObjs.buttons[btnOrgID].OverrideBtnInnerColors(BTN_ORG_DEF_COLOR, BTN_ORG_HOV_COLOR, BTN_ORG_CLK_COLOR, BTN_ORG_DIS_COLOR);
				sceneObjs.buttons[btnOrgID].OverrideBtnOutlineColors(CBTN_OUTLINE_COLOR);
				sceneObjs.buttons[btnOrgID].showText = false;

				sceneObjs.AddButton(btnYlwID, sf::Vector2f(SCREEN_WIDTH * .2f * .8f, SCREEN_HEIGHT * .2f * .8f), sf::Vector2i(SCREEN_WIDTH * .4f, SCREEN_HEIGHT * .4f), sf::Vector2f(.5f, .5f), font);
				sceneObjs.buttons[btnYlwID].OverrideBtnInnerColors(BTN_YLW_DEF_COLOR, BTN_YLW_HOV_COLOR, BTN_YLW_CLK_COLOR, BTN_YLW_DIS_COLOR);
				sceneObjs.buttons[btnYlwID].OverrideBtnOutlineColors(CBTN_OUTLINE_COLOR);
				sceneObjs.buttons[btnYlwID].showText = false;

				sceneObjs.AddButton(btnGrnID, sf::Vector2f(SCREEN_WIDTH * .2f * .8f, SCREEN_HEIGHT * .2f * .8f), sf::Vector2i(SCREEN_WIDTH * .4f, SCREEN_HEIGHT * .6f), sf::Vector2f(.5f, .5f), font);
				sceneObjs.buttons[btnGrnID].OverrideBtnInnerColors(BTN_GRN_DEF_COLOR, BTN_GRN_HOV_COLOR, BTN_GRN_CLK_COLOR, BTN_GRN_DIS_COLOR);
				sceneObjs.buttons[btnGrnID].OverrideBtnOutlineColors(CBTN_OUTLINE_COLOR);
				sceneObjs.buttons[btnGrnID].showText = false;

				sceneObjs.AddButton(btnTrqID, sf::Vector2f(SCREEN_WIDTH * .2f * .8f, SCREEN_HEIGHT * .2f * .8f), sf::Vector2i(SCREEN_WIDTH * .6f, SCREEN_HEIGHT * .4f), sf::Vector2f(.5f, .5f), font);
				sceneObjs.buttons[btnTrqID].OverrideBtnInnerColors(BTN_TRQ_DEF_COLOR, BTN_TRQ_HOV_COLOR, BTN_TRQ_CLK_COLOR, BTN_TRQ_DIS_COLOR);
				sceneObjs.buttons[btnTrqID].OverrideBtnOutlineColors(CBTN_OUTLINE_COLOR);
				sceneObjs.buttons[btnTrqID].showText = false;

				sceneObjs.AddButton(btnCynID, sf::Vector2f(SCREEN_WIDTH * .2f * .8f, SCREEN_HEIGHT * .2f * .8f), sf::Vector2i(SCREEN_WIDTH * .6f, SCREEN_HEIGHT * .6f), sf::Vector2f(.5f, .5f), font);
				sceneObjs.buttons[btnCynID].OverrideBtnInnerColors(BTN_CYN_DEF_COLOR, BTN_CYN_HOV_COLOR, BTN_CYN_CLK_COLOR, BTN_CYN_DIS_COLOR);
				sceneObjs.buttons[btnCynID].OverrideBtnOutlineColors(CBTN_OUTLINE_COLOR);
				sceneObjs.buttons[btnCynID].showText = false;

				sceneObjs.AddButton(btnIdgID, sf::Vector2f(SCREEN_WIDTH * .2f * .8f, SCREEN_HEIGHT * .2f * .8f), sf::Vector2i(SCREEN_WIDTH * .8f, SCREEN_HEIGHT * .4f), sf::Vector2f(.5f, .5f), font);
				sceneObjs.buttons[btnIdgID].OverrideBtnInnerColors(BTN_IDG_DEF_COLOR, BTN_IDG_HOV_COLOR, BTN_IDG_CLK_COLOR, BTN_IDG_DIS_COLOR);
				sceneObjs.buttons[btnIdgID].OverrideBtnOutlineColors(CBTN_OUTLINE_COLOR);
				sceneObjs.buttons[btnIdgID].showText = false;

				sceneObjs.AddButton(btnVltID, sf::Vector2f(SCREEN_WIDTH * .2f * .8f, SCREEN_HEIGHT * .2f * .8f), sf::Vector2i(SCREEN_WIDTH * .8f, SCREEN_HEIGHT * .6f), sf::Vector2f(.5f, .5f), font);
				sceneObjs.buttons[btnVltID].OverrideBtnInnerColors(BTN_VLT_DEF_COLOR, BTN_VLT_HOV_COLOR, BTN_VLT_CLK_COLOR, BTN_VLT_DIS_COLOR);
				sceneObjs.buttons[btnVltID].OverrideBtnOutlineColors(CBTN_OUTLINE_COLOR);
				sceneObjs.buttons[btnVltID].showText = false;
				break;
			case SceneStage::GAME:
				//std::cout << "GAME stage started" << std::endl;
				accumMove = sf::Vector2f(0, 0);

				//crear el taulell amb les coordenades que ara ja tenim
				for (std::map<int, Player*>::iterator it = playersMap.begin(); it != playersMap.end(); ++it)
				{
					//std::lock_guard<std::mutex> guard(mtx);
					Player* player = it->second;
					board.InitializeSlither(player);
				}
				break;
			case SceneStage::DEATH:
				sceneObjs.AddText("You Died, eat faster next time.", sf::Color::White, font, 3, 15, sf::Vector2i(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2), sf::Vector2f(.5f, .5f), sf::Color::Red);
				sceneObjs.AddButton(btnBackToSkinSelectionID, sf::Vector2f(SCREEN_WIDTH * .2f * .8f, SCREEN_HEIGHT * .2f), sf::Vector2i(SCREEN_WIDTH * .5f, SCREEN_HEIGHT * .7f), sf::Vector2f(.5f, .5f), font, "Exit");
				break;
			case SceneStage::WAITINGFORGAME:
				sceneObjs.AddStandaloneRect(sf::Vector2f(SCREEN_WIDTH * .8f, SCREEN_HEIGHT * .8f), sf::Vector2f(SCREEN_WIDTH * .1f, SCREEN_HEIGHT * .1f), MENUS_BG_RECT_COLOR, 10, BTN_DEF_COLOR);
				sceneObjs.AddText("Searching for a decent(say hi to noobs) game", sf::Color::White, font, 3, 15, sf::Vector2i(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2), sf::Vector2f(.5f, .5f));
				break;
			case SceneStage::WIN:
				sceneObjs.AddText("You stand victorious above all other slithes!", sf::Color::White, font, 3, 15, sf::Vector2i(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2), sf::Vector2f(.5f, .5f), sf::Color::Red);
				sceneObjs.AddButton(btnBackToSkinSelectionID, sf::Vector2f(SCREEN_WIDTH * .2f * .8f, SCREEN_HEIGHT * .2f), sf::Vector2i(SCREEN_WIDTH * .5f, SCREEN_HEIGHT * .7f), sf::Vector2f(.5f, .5f), font, "Exit");
				break;
			}

			lastSceneStage = sceneStage;
			startFlag = false;
		}
		
		if (sceneStage == SceneStage::LOGIN)
		{
			board.ClearWindow();
			//Draw sceneObjs
			sceneObjs.DrawScene(board.window);
			board.DisplayWindow();
		}
		else if (sceneStage == SceneStage::SKIN_SELECT)
		{
			board.ClearWindow();
			//Draw sceneObjs
			sceneObjs.DrawScene(board.window);
			board.DisplayWindow();

			mouseCoords = sf::Mouse::getPosition(board.window);
			// check all the window's events that were triggered since the last iteration of the loop
			sf::Event event;
			LMBDown = false;
			while (board.window.pollEvent(event))
			{

				// "close requested" event: we close the window
				if (event.type == sf::Event::Closed)
				{
					board.window.close();
					exit(0);
				}
				else if (event.type == sf::Event::MouseButtonPressed)
				{
					if (event.mouseButton.button == sf::Mouse::Left)
					{
						LMBPressed = true;
						LMBDown = true;
					}
					else if (event.mouseButton.button == sf::Mouse::Right)
					{

					}
				}
				else if (event.type == sf::Event::MouseButtonReleased)
				{
					if (event.mouseButton.button == sf::Mouse::Left)
					{
						LMBPressed = false;
						if (lastPressedButtonID != "None")
						{
							if (sceneObjs.buttons[lastPressedButtonID].CheckCursorCollision(mouseCoords) && sceneObjs.buttons[lastPressedButtonID].GetInteractable())
							{
								//Button functionality
								ButtonFunctionality();
							}
						}						
					}
				}

				//Set button colors (Default - Hover - Click)
				UpdateButtonAppearance(sceneObjs.buttons[btnRedID], btnRedID);
				UpdateButtonAppearance(sceneObjs.buttons[btnOrgID], btnOrgID);
				UpdateButtonAppearance(sceneObjs.buttons[btnYlwID], btnYlwID);
				UpdateButtonAppearance(sceneObjs.buttons[btnGrnID], btnGrnID);
				UpdateButtonAppearance(sceneObjs.buttons[btnTrqID], btnTrqID);
				UpdateButtonAppearance(sceneObjs.buttons[btnCynID], btnCynID);
				UpdateButtonAppearance(sceneObjs.buttons[btnIdgID], btnIdgID);
				UpdateButtonAppearance(sceneObjs.buttons[btnVltID], btnVltID);
			}
		}
		else if (sceneStage == SceneStage::WAITINGFORGAME)
		{
			board.ClearWindow();
			//Draw sceneObjs
			sceneObjs.DrawScene(board.window);
			board.DisplayWindow();
		}
		else if (sceneStage == SceneStage::GAME || sceneStage == SceneStage::DEATH || sceneStage == SceneStage::WIN)
		{
			//std::cout << "Stage draw" << std::endl;
			bool deathScreen = (sceneStage == SceneStage::DEATH);
			bool winScreen = (sceneStage == SceneStage::WIN);
			board.ClearWindow();
			board.DrawBoard(mtx_food);
			if (deathScreen || winScreen)
			{
				//Draw sceneObjs
				sceneObjs.DrawScene(board.window);
			}
			board.DisplayWindow();

			if (deathScreen || winScreen)
			{
				mouseCoords = sf::Mouse::getPosition(board.window);
				// check all the window's events that were triggered since the last iteration of the loop
				sf::Event event;
				LMBDown = false;
				while (board.window.pollEvent(event))
				{

					// "close requested" event: we close the window
					if (event.type == sf::Event::Closed)
					{
						board.window.close();
						exit(0);
					}
					else if (event.type == sf::Event::MouseButtonPressed)
					{
						if (event.mouseButton.button == sf::Mouse::Left)
						{
							LMBPressed = true;
							LMBDown = true;
						}
						else if (event.mouseButton.button == sf::Mouse::Right)
						{

						}
					}
					else if (event.type == sf::Event::MouseButtonReleased)
					{
						if (event.mouseButton.button == sf::Mouse::Left)
						{
							LMBPressed = false;
							if (sceneObjs.buttons[lastPressedButtonID].CheckCursorCollision(mouseCoords) && sceneObjs.buttons[lastPressedButtonID].GetInteractable())
							{
								//Button functionality
								ButtonFunctionality();
							}
						}
					}

					//Set button colors (Default - Hover - Click)
					UpdateButtonAppearance(sceneObjs.buttons[btnBackToSkinSelectionID], btnBackToSkinSelectionID);
				}
			}
			else
			{
				//std::cout << "Stage is GAME" << std::endl;
				board.Commands(m_player);

				sf::Vector2i sumInt = sf::Vector2i((int)(board.playerMovement.x * 1000), (int)(board.playerMovement.y * 1000));
				board.playerMovement = sf::Vector2f((float)sumInt.x / 1000.f, (float)sumInt.y / 1000.f);
				accumMove += board.playerMovement;

				sumInt = sf::Vector2i((int)(accumMove.x * 1000), (int)(accumMove.y * 1000));
				accumMove = sf::Vector2f((float)sumInt.x / 1000.f, (float)sumInt.y / 1000.f);

				////prediction movement:
				if (abs(board.playerMovement.x) + abs(board.playerMovement.y) > 0)
				{
					mtx_bodies.lock();
					m_player->UpdatePosition(board.playerMovement);
					board.UpdateSlither(m_player->appId);
					mtx_bodies.unlock();
				}
			}			
		}
	}
}

void MoveSending()
{
	//sf::Clock clock;
	sf::Packet pack;

	while (true)
	{
		//sf::Time t1 = clock.getElapsedTime();
		//if (t1.asSeconds() > movementTimer)
		//{
		if (sceneStage == SceneStage::GAME)
		{
			if (abs(accumMove.x) + abs(accumMove.y) > 0)
			{
				pack.clear();
				pack << static_cast<int>(Protocol::MOVE);
				pack << m_player->appId;
				//std::cout << "APPid on move: " << m_player->appId << std::endl;
				pack << actualMoveID;

				movesMap[actualMoveID] = m_player->bodyPositions[0];
				actualMoveID++;
				pack << (int)(accumMove.x * 1000) << (int)(accumMove.y * 1000);

				if (sock.send(pack, IP, PORT) != sf::UdpSocket::Status::Done)
				{
					std::cout << "Error sending the packet" << std::endl;
				}

				accumMove = sf::Vector2f(0, 0);
			}
		}

			//std::cout << "THREADS: MOVEMENT going to sleep for " << movementTimer * 1000 << " Milliseconds." << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds((int)(movementTimer * 1000)));
			//std::cout << "THREADS: MOVEMENT awakened" << std::endl;
			//clock.restart();
		//}
	}
}

void SendAcknowledge(int idPack)
{
	sf::Packet pack;
	pack << static_cast<int>(Protocol::ACK);
	pack << idPack;
	sock.send(pack, IP, PORT);
	std::cout << "ack sent" << std::endl;
}

void InterpolatePositions()
{
	//sf::Clock clock;

	while (true)
	{
		//sf::Time t1 = clock.getElapsedTime();
		//if (t1.asSeconds() > interpolationTimer)
		//{
		if (sceneStage == SceneStage::GAME)
		{
			std::vector<std::map<int, std::vector<sf::Vector2f>>::iterator> toErase;//vector per eliminar despr�s
			mtx_bodies.lock();
			for (std::map<int, std::vector<sf::Vector2f>>::iterator it = interpolationsMap.begin(); it != interpolationsMap.end(); ++it)
			{
				
				if (playersMap[it->first]->InterpolateTo(it->second, 0.5f))
				{
					//std::cout << "interpolation arrived and erased" << std::endl;
					toErase.push_back(it);
				}
				board.UpdateSlither(it->first);
				

				//std::cout << "interpolating" << std::endl;
			}

			while (toErase.size() != 0)
			{
				interpolationsMap.erase(toErase[0]);
				toErase.erase(toErase.begin());
			}
			mtx_bodies.unlock();
		}

			//std::cout << "THREADS: INTERP going to sleep for " << interpolationTimer * 1000 << " Milliseconds." << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds((int)(interpolationTimer * 1000)));
			//std::cout << "THREADS: INTERP awakened" << std::endl;
			//clock.restart();
		//}
	}
}