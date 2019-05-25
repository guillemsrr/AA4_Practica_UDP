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

enum class SceneStage
{
	LOGIN,
	SKIN_SELECT,
	GAME,
	DEATH
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
				pack >> num;
				switch (static_cast<Protocol>(num))
				{
				case WELCOME:
				{
					//create the player:
					m_player = new Player(&pack);
					m_player->color = sf::Color::Green;
					playersMap[m_player->id] = m_player;

					std::cout << "WELCOME player "<< m_player->id << std::endl;

					//Dar paso al registro y/o login

					std::thread RegisterLoginThread(RegisterLoginThreadFunction);
					RegisterLoginThread.detach();


					//ESTO DEBERIA IR DESPUES DEL LOGIN SI O SI

					//create the others:
					int sizeOthers;
					pack >> sizeOthers;
					for (int i = 0; i < sizeOthers; i++)
					{
						Player* p = new Player(&pack);
						p->color = sf::Color::Red;
						playersMap[p->id] = p;
					}

					//create the balls:
					int numFood;
					pack >> numFood;
					std::cout << "num balls: " << numFood << std::endl;

					std::vector<sf::Vector2f> foodPositions;

					for (int i = 0; i < numFood; i++)
					{
						int id;
						sf::Vector2f pos;
						pack >> pos.x;
						pack >> pos.y;
						foodPositions.push_back(pos);
					}

					if (true)
					{
						std::lock_guard<std::mutex> guard(mtx_food);
						board.foodPositions = foodPositions;//aix� s'hauria de millorar..
					}

					received = true;

					std::thread graphicsInterface(&GraphicsInterface);
					graphicsInterface.detach();

					std::thread accumControlThread(&MoveSending);
					accumControlThread.detach();

					std::thread interpolationsThread(&InterpolatePositions);
					interpolationsThread.detach();
				}
					break;
				case NEW_PLAYER:
				{
					Player* p = new Player(&pack);
					p->color = sf::Color::Red;
					playersMap[p->id] = p;
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
					pack << m_player->id;
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

					if (idPlayer == m_player->id)
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

								m_player->UpdateTheRestOfPositions(numPos, headPos, &pack);
								board.UpdateSlither(idPlayer);

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
								while (numPos > (int)m_player->bodyPositions.size())
								{
									m_player->CreateBodyPosition();
								}
								board.UpdateSlither(idPlayer);
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
						interpolationsMap[idPlayer] = playersMap[idPlayer]->GetFuturePositions(&pack);
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

					if (idPlayer == m_player->id)//myself
					{
						//std::lock_guard<std::mutex> guard(mtx_bodies);
						//m_player->bodyPositions.clear();
						//board.UpdateSlither(idPlayer);
						m_player->dead = true;

						std::cout << "YOU DIED" << std::endl;
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
				case REGISTER:
					pack >> codigoRegistro;

					std::cout << "Recibo confirmacion del registro con codigo: " << codigoRegistro << std::endl;

					registerResponse = true;
					break;
				case LOGIN:
					pack >> codigoLogin;
					std::cout << "Recibo confirmacion del login con codigo:" << codigoLogin << std::endl;
					loginResponse = true;
					break;
				break;
				}
			}
		}
	}

	return 0;
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

	sf::Clock clock;


	//Utilizar cabecera REGISTER, con username, password, email.
	sf::Packet packRegister;
	packRegister << static_cast<int>(Protocol::REGISTER);
	packRegister << username << password << email;

	while (!registerResponse)
	{
		sf::Time t1 = clock.getElapsedTime();
		if (t1.asSeconds() > helloSendingTimer)
		{
			//Enviar paquete a servidor cada x segundos, hasta que  me diga ok
			if (sock.send(packRegister, IP, PORT) != sf::UdpSocket::Status::Done)
				std::cout << "Error al enviar el registro" << std::endl;
			else
				std::cout << "Registro enviado" << std::endl;

			clock.restart();
		}

	}

	loginOrRegister = false;

}

void LoginUser()
{

	sf::Clock clock;

	//Utilizar cabecera LOGIN con username y password.
	sf::Packet packetLogin;
	packetLogin << static_cast<int>(Protocol::LOGIN);
	packetLogin << username << password;

	while (!loginResponse)
	{

		sf::Time t1 = clock.getElapsedTime();
		if (t1.asSeconds() > helloSendingTimer)
		{
			//Enviar paquete a servidor cada x segundos, hasta que me diga ok
			if (sock.send(packetLogin, IP, PORT) != sf::UdpSocket::Status::Done)
				std::cout << "Error al enviar el login" << std::endl;
			else
				std::cout << "Login enviado" << std::endl;


			clock.restart();
		}

	}

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
	}
	else if (lastPressedButtonID == btnRedID)
	{
		//Select Red skin...

		skinSelected = true;
	}
	else if (lastPressedButtonID == btnOrgID)
	{
		//Select Orange skin...

		skinSelected = true;
	}
	else if (lastPressedButtonID == btnYlwID)
	{
		//Select Yellow skin...

		skinSelected = true;
	}
	else if (lastPressedButtonID == btnGrnID)
	{
		//Select Green skin...

		skinSelected = true;
	}
	else if (lastPressedButtonID == btnTrqID)
	{
		//Select Turquoise skin...

		skinSelected = true;
	}
	else if (lastPressedButtonID == btnCynID)
	{
		//Select Cyan skin...

		skinSelected = true;
	}
	else if (lastPressedButtonID == btnIdgID)
	{
		//Select Indigo skin...

		skinSelected = true;
	}
	else if (lastPressedButtonID == btnVltID)
	{
		//Select Violet skin...

		skinSelected = true;
	}

	if (skinSelected)
	{
		//Start/join game
	}
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
		else if (sceneStage == SceneStage::GAME || sceneStage == SceneStage::DEATH)
		{
			bool deathScreen = sceneStage == SceneStage::DEATH;
			board.ClearWindow();
			board.DrawBoard(mtx_food);
			if (deathScreen)
			{
				//Draw sceneObjs
				sceneObjs.DrawScene(board.window);
			}
			board.DisplayWindow();

			if (!deathScreen)
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
				board.Commands(m_player);

				sf::Vector2i sumInt = sf::Vector2i((int)(board.playerMovement.x * 1000), (int)(board.playerMovement.y * 1000));
				board.playerMovement = sf::Vector2f((float)sumInt.x / 1000.f, (float)sumInt.y / 1000.f);
				accumMove += board.playerMovement;

				sumInt = sf::Vector2i((int)(accumMove.x * 1000), (int)(accumMove.y * 1000));
				accumMove = sf::Vector2f((float)sumInt.x / 1000.f, (float)sumInt.y / 1000.f);

				////prediction movement:
				if (abs(board.playerMovement.x) + abs(board.playerMovement.y) > 0)
				{
					m_player->UpdatePosition(board.playerMovement);
					board.UpdateSlither(m_player->id);
				}
			}			
		}
	}
}

void MoveSending()
{
	while (!startGame)
	{
		//just don't start
	}

	//sf::Clock clock;
	sf::Packet pack;

	while (true)
	{
		//sf::Time t1 = clock.getElapsedTime();
		//if (t1.asSeconds() > movementTimer)
		//{
			if (abs(accumMove.x) + abs(accumMove.y) > 0)
			{
				pack.clear();
				pack << static_cast<int>(Protocol::MOVE);
				pack << m_player->id;
				pack << actualMoveID;

				movesMap[actualMoveID] = m_player->bodyPositions[0];
				actualMoveID++;
				pack << (int)(accumMove.x*1000) << (int)(accumMove.y*1000);

				if (sock.send(pack, IP, PORT) != sf::UdpSocket::Status::Done)
				{
					std::cout << "Error sending the packet" << std::endl;
				}

				accumMove = sf::Vector2f(0, 0);
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
	while (!startGame)
	{
		//just don't start
	}

	//sf::Clock clock;

	while (true)
	{
		//sf::Time t1 = clock.getElapsedTime();
		//if (t1.asSeconds() > interpolationTimer)
		//{
			std::vector<std::map<int, std::vector<sf::Vector2f>>::iterator> toErase;//vector per eliminar despr�s
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

			//std::cout << "THREADS: INTERP going to sleep for " << interpolationTimer * 1000 << " Milliseconds." << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds((int)(interpolationTimer * 1000)));
			//std::cout << "THREADS: INTERP awakened" << std::endl;
			//clock.restart();
		//}
	}
}