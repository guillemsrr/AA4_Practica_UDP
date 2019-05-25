#pragma once
#include <iostream>
#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include "ClientProxy.h"
#include "CriticPack.h"
#include <Constants.h>
#include <thread>
#include <math.h>
#include <Food.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/mysql_driver.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/prepared_statement.h>


//---------SERVIDOR---------//


sf::UdpSocket sock;
const int maxFood = 100;
const float minFoodDist = 200.f;

//lists / maps
std::map<int, ClientProxy*> clientProxies;
std::map<int, CriticPack*> criticPackets;
std::vector<Food*> foodVector;


//declarations:
//threads:
void PingThreadFunction();
void DisconnectionCheckerThreadFunction();
void CriticPacketsManagerThreadFunction();
void MovementControlThread();
void FoodUpdateThread();
//other:
void AnswerRegister(sf::IpAddress ip, unsigned short port, sf::Packet pack);
void AnswerLogin(sf::IpAddress ip, unsigned short port, sf::Packet pack);
void SetPlayerSesion(int idPlayer);
void GetPlayeBBDDInfo(int idPlayer);
void UpdatePlayerBBDDInfo(int idPlayer);
void ComputeMMRFromPlayer(int idPlayer);
void NewPlayer(sf::IpAddress ip, unsigned short port, sf::Packet pack);
void AccumMovement(sf::Packet pack);
void MovementControl(int idPlayer, int idMove);
void InitializeFood();
bool RandomPacketLost();
float Distance(sf::Vector2f v1, sf::Vector2f v2);
void PlayerCollisionCheck(int idPlayer);


int main()
{
	#pragma region Init

	srand(time(NULL));
	if (sock.bind(PORT) != sf::Socket::Status::Done)
	{
		std::cout << "Error on binding socket" << std::endl;
		//return 0;
	}

	std::cout << "Socket binded successfully" << std::endl;

	//Crear thread para PING
	std::thread pingThread(&PingThreadFunction);
	pingThread.detach();

	//Crear thread para comprobar desconexiones
	std::thread disconnectionCheckerThread(&DisconnectionCheckerThreadFunction);
	disconnectionCheckerThread.detach();

	//Crear thread para administrar paquetes críticos
	std::thread criticPacketsManagerThread(&CriticPacketsManagerThreadFunction);
	criticPacketsManagerThread.detach();

	//Thread de validació de moviment
	std::thread movementControlThread(&MovementControlThread);
	movementControlThread.detach();

	//initialize food:
	InitializeFood();

	//Thread de food
	//std::thread foodUpdateThread(&FoodUpdateThread);
	//foodUpdateThread.detach();

	#pragma endregion

	while (true)
	{
		sf::Packet pack;
		sf::IpAddress ip;
		unsigned short port;

		if (sock.receive(pack, ip, port) != sf::Socket::Status::Done)
		{
			std::cout << "Error on receiving packet ip:" << ip.toString() << std::endl;
		}

		if (RandomPacketLost())
		{
			//std::cout << "Reading package" << std::endl;
			int num;
			pack >> num;
			switch (static_cast<Protocol>(num))
			{
			case HELLO:
				std::cout << "HELLO received" << std::endl;
				NewPlayer(ip, port, pack);
				break;
			case ACK:
				std::cout << "ACK received" << std::endl;
				int auxIdPack;
				pack >> auxIdPack;
				criticPackets.erase(criticPackets.find(auxIdPack));
				break;
			case PONG:
				//std::cout << "PONG received" << std::endl;
				int pId;
				pack >> pId;
				clientProxies[pId]->numPings = 0;
				break;
			case MOVE:
				//std::cout << "MOVE received" << std::endl;
				AccumMovement(pack);
				break;
			case REGISTER:
				std::cout << "Petición de registro recibida." << std::endl;
				AnswerRegister(ip, port, pack);
				break;
			case LOGIN:
				std::cout << "Petición de login recibida." << std::endl;
				AnswerLogin(ip, port, pack);
				break;
			}
		}
		else
		{
			std::cout << "Packet lost (on purpose)" << std::endl;
		}
	}

	return 0;
}

void PingThreadFunction()
{
	//sf::Clock clock;
	sf::Packet packPing;

	packPing << static_cast<int>(Protocol::PING);

	sf::Packet packFood;
	while (true)
	{
		//sf::Time t1 = clock.getElapsedTime();
		//if (t1.asSeconds() > PINGTIMER)
		//{
			for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
			{
				if (sock.send(packPing, it->second->ip, it->second->port) != sf::UdpSocket::Status::Done)
				{
					std::cout << "Error sending PING to client with id: " << it->second->id << "and ip: " << it->second->ip.toString() << std::endl;
				}
				else
				{
					it->second->numPings++;
				}
			}

			//ERE LAYS THE CULPRIT
			for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
			{
				packFood << static_cast<int>(Protocol::FOOD_UPDATE);

				//tota la info del menjar:
				//només els que té més aprop
				std::vector<Food*> closeFood;

				for (int i = 0; i < (int)foodVector.size(); i++)
				{
					if (Distance(it->second->bodyPositions[0], foodVector[i]->position) < minFoodDist)
					{
						closeFood.push_back(foodVector[i]);
					}
				}
				//num foods:
				packFood << static_cast<int>(closeFood.size());
				for (std::vector<Food*>::iterator closeit = closeFood.begin(); closeit != closeFood.end(); ++closeit)
				{
					Food* food = *closeit;
					//pack << food->id;
					packFood << food->position.x;
					packFood << food->position.y;
					//pack << food->color; //no puc passar el color?
				}

				sock.send(packFood, it->second->ip, it->second->port);
				packFood.clear();
			}

			//std::cout << "THREADS: PING going to sleep for " << PINGTIMER * 1000 << " Milliseconds." << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds((int)(PINGTIMER * 1000)));
			//std::cout << "THREADS: PING awakened " << std::endl;
			//clock.restart();
		//}
	}
}

void DisconnectionCheckerThreadFunction()
{
	//sf::Clock clock;
	sf::Packet pack;

	while (true)
	{
		//sf::Time t1 = clock.getElapsedTime();
		//if (t1.asSeconds() > DISCONNECTTIMER)
		//{
			//std::cout << "Checking for disconnected players" << std::endl;
			//Comprobar si el numPings supera el límite y enviar el DISCONNECTED a todos los demás clientes
			for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); /*++it*/)
			{
				if (it->second->numPings >= MAXUNOPERATIVETIMER)
				{
					//Añadir el paquete a la lista de críticos
					pack << static_cast<int>(Protocol::DISCONNECTED);
					int idPack = (int)criticPackets.size();
					pack << idPack;
					pack << it->second->id;

					//Enviar DISCONNECTED
					for (std::map<int, ClientProxy*>::iterator it2 = clientProxies.begin(); it2 != clientProxies.end(); ++it2)
					{
						if (it2 != it)
						{

							CriticPack* cp = new CriticPack((int)criticPackets.size(), pack, it2->second->ip, it2->second->port);
							criticPackets[idPack] = cp;

							sock.send(pack, it2->second->ip, it2->second->port);
						}
					}

					it = clientProxies.erase(it);
				}
				else
				{
					++it;
				}
			}

			//std::cout << "THREADS: DISCONNECT going to sleep for " << DISCONNECTTIMER * 1000 << " Milliseconds." << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds((int)(DISCONNECTTIMER * 1000)));
			//std::cout << "THREADS: DISCONNECT awakened" << std::endl;
			//clock.restart();
		//}
	}
}

void CriticPacketsManagerThreadFunction()
{
	//sf::Clock clock;

	while (true)
	{
		//sf::Time t1 = clock.getElapsedTime();

		//if (t1.asSeconds() > CRITICPACKETSTIMER)
		//{
			for (std::map<int, CriticPack*>::iterator it = criticPackets.begin(); it != criticPackets.end(); ++it)
			{
				sock.send(it->second->pack, it->second->ip, it->second->port);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds((int)(CRITICPACKETSTIMER * 1000)));
			//clock.restart();
		//}
	}
}

void MovementControlThread()
{
	//sf::Clock clock;

	while (true)
	{
		//sf::Time t1 = clock.getElapsedTime();
		//if (t1.asSeconds() > MOVEMENTUPDATETIMER)
		//{
			for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
			{
				if (abs(it->second->accumMovement.x) + abs(it->second->accumMovement.y) > 0)
				{
					MovementControl(it->second->id, it->second->lastIdMove);
				}
			}

			//std::cout << "THREADS: MOVEMENT going to sleep for " << MOVEMENTUPDATETIMER * 1000 << " Milliseconds." << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds((int)(MOVEMENTUPDATETIMER * 1000)));
			//std::cout << "THREADS: MOVEMENT awakened" << std::endl;
			//clock.restart();
		//}
	}
}

void AnswerRegister(sf::IpAddress ip, unsigned short port, sf::Packet pack)
{
	std::string username;
	std::string password;
	std::string email;

	pack >> username >> password >> email;

	pack.clear();

	//Procesado de info del registro, consulta a la BBDD

	///1-Revisar que la info sea correcta
	sql::Driver* driver = sql::mysql::get_driver_instance();
	sql::Connection* conn = driver->connect("tcp://www.db4free.net:3306", "slitheradmin", "123456789Admin");
	conn->setSchema("slitherudp");

	std::cout << "Voy a procesar los datos del cliente, usuario: " << username << ", password: " << password << ", email: " << email << std::endl;

	sql::SQLString usernameBBDD = username.c_str();
	sql::SQLString emailBBDD = email.c_str();
	sql::SQLString passwordBBDD = password.c_str();

	sql::PreparedStatement* pstmt;
	sql::ResultSet* res;

	///2-Mirar en base de datos si ya existe el correo o el username

	int usernameCoincidence = 0;
	int emailCoincidence = 0;



	pstmt = conn->prepareStatement("SELECT count(*) FROM Usuarios WHERE Username=?");
	pstmt->setString(1, usernameBBDD);
	pstmt->execute();

	do
	{
		res = pstmt->getResultSet();

		while (res->next())
		{
			usernameCoincidence = res->getInt(1);
			std::cout << "Resultado de buscar el username: " << res->getInt(1) << std::endl;
		}

	} while (pstmt->getMoreResults());



	pstmt = conn->prepareStatement("SELECT count(*) FROM Usuarios WHERE Email=?");
	pstmt->setString(1, usernameBBDD);
	pstmt->execute();



	do
	{
		res = pstmt->getResultSet();

		while (res->next())
		{
			emailCoincidence = res->getInt(1);
			std::cout << "Resultado de buscar el email: " << res->getInt(1) << std::endl;
		}

	} while (pstmt->getMoreResults());

	///3-Hacer registro

	int errorCode = 0;
	/*
	ESTE ERROR CODE TIENE LAS SIGUIENTES OPCIONES:
		0 - Registro OK
		1 - Usuario o email ya registrados
	*/


	if (usernameCoincidence > 0 || emailCoincidence > 0)
	{
		//Devolver a cliente código de error usuario o email ya existen
		errorCode = 1;
	}
	else
	{
		//Registrar al usuario
		delete pstmt;
		delete res;

		pstmt = conn->prepareStatement("INSERT INTO Usuarios(Username, Pasword, Email, Dinero) VALUES(?, ?, ?, ?)");
		pstmt->setString(1, usernameBBDD);
		pstmt->setString(2, passwordBBDD);
		pstmt->setString(3, emailBBDD);
		pstmt->setInt(4, 0);
		pstmt->executeUpdate();
	}


	///4-Recoger y devolver codigo de error

	//Respuesta a cliente
	pack << static_cast<int>(Protocol::REGISTER);
	pack << errorCode;

	if (sock.send(pack, ip, port) != sf::UdpSocket::Status::Done)
		std::cout << "Error al responder al registro." << std::endl;
	else
		std::cout << "Devuelto el mensaje de registro a ip: " << ip.toString() << ", con puerto: " << port << std::endl;

	conn->close();

}

void AnswerLogin(sf::IpAddress ip, unsigned short port, sf::Packet pack)
{
	//Comprobar si el clientProxy con Ip dada ya se esta intentando logear
	for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
	{
		if (it->second->ip == ip && it->second->port == port)
		{
			if (it->second->isLogging == false && it->second->isLogged == false)
			{
				it->second->isLogging = true;

				std::string username;
				std::string password;

				pack >> username >> password;

				pack.clear();

				//Procesado de info del login, consulta a la BBDD
				sql::Driver* driver = sql::mysql::get_driver_instance();
				sql::Connection* conn = driver->connect("tcp://www.db4free.net:3306", "slitheradmin", "123456789Admin");
				conn->setSchema("slitherudp");

				std::cout << "Voy a procesar los datos del cliente, usuario: " << username << ", password: " << password << std::endl;

				sql::SQLString usernameBBDD = username.c_str();
				sql::SQLString passwordBBDD = password.c_str();

				sql::PreparedStatement* pstmt;
				sql::ResultSet* res;

				pstmt = conn->prepareStatement("SELECT id FROM Usuarios WHERE Username=? AND Pasword=?");
				pstmt->setString(1, usernameBBDD);
				pstmt->setString(2, passwordBBDD);
				pstmt->execute();

				int idUser = 0;
				int errorCode = 0;

				do
				{
					res = pstmt->getResultSet();

					while (res->next())
					{
						idUser = res->getInt(1);
						std::cout << "Resultado de consultar el inicio de sesion: " << res->getInt(1) << std::endl;
					}

				} while (pstmt->getMoreResults());


				if (idUser > 0)
				{
					//El cliente ha iniciado sesión
					errorCode = 1;

					it->second->isLogged = true;
					SetPlayerSesion(idUser);

					GetPlayeBBDDInfo(idUser);
				}
				else
				{
					errorCode = 0;
				}


				//Respuesta a cliente
				pack << static_cast<int>(Protocol::LOGIN);
				pack << errorCode;



				if (sock.send(pack, ip, port) != sf::UdpSocket::Status::Done)
					std::cout << "Error al responder al login." << std::endl;
				else
				{
					std::cout << "Devuelto el mensaje de login a ip: " << ip.toString() << ", con puerto: " << port << std::endl;
					it->second->isLogging = false;
				}


				conn->close();


			}
		}
	}

}

void SetPlayerSesion(int idPlayer)
{
	sql::Driver* driver = sql::mysql::get_driver_instance();
	sql::Connection* conn = driver->connect("tcp://www.db4free.net:3306", "slitheradmin", "123456789Admin");
	conn->setSchema("slitherudp");

	sql::PreparedStatement* pstmt;
	sql::ResultSet* res;

	pstmt = conn->prepareStatement("INSERT INTO Sesion(idUsuario, InicioSesion, FinalSesion, TotalKills, MaxLongitud) VALUES(?, CURRENT_TIME(), CURRENT_TIME(), ?, ?)");
	pstmt->setInt(1, idPlayer);
	pstmt->setInt(2, 0);
	pstmt->setInt(3, 0);
	pstmt->executeUpdate();

	conn->close();

}

void GetPlayeBBDDInfo(int idPlayer)
{
	/*Crear consultas a BBDD con id de player y sacar todos sus stats:

	Informacion a sacar(Para el matchmaking):
		-Numero de partidas jugadas
		-Numero de partidas ganadas
		-Cantidad de muertes totales
	*/

	sql::Driver* driver = sql::mysql::get_driver_instance();
	sql::Connection* conn = driver->connect("tcp://www.db4free.net:3306", "slitheradmin", "123456789Admin");
	conn->setSchema("slitherudp");

	sql::PreparedStatement* pstmt;
	sql::ResultSet* res;

	int partidasGanadas = 0;
	int partidasJugadas = 0;
	std::vector<int> idPartidasJugadas;
	int partidasNoGanadas = 0;
	int totalKills = 0;

	//1-Consultar datos usuario

	//2-Consultar datos partidas usuarios
#pragma region Partidas_Ganadas

	pstmt = conn->prepareStatement("SELECT count(*) FROM Partidas WHERE idGanador=?");
	pstmt->setInt(1, idPlayer);
	pstmt->execute();

	do
	{
		res = pstmt->getResultSet();

		while (res->next())
		{
			partidasGanadas = res->getInt(1);
			std::cout << "Resultado de consultar el la cantidad de partidas ganadas: " << res->getInt(1) << ", para el usuario con id: " << idPlayer << std::endl;
		}

	} while (pstmt->getMoreResults());

#pragma endregion

#pragma region Partidas_Jugadas

	pstmt = conn->prepareStatement("SELECT id FROM Partidas WHERE idUsuario=?");
	pstmt->setInt(1, idPlayer);
	pstmt->execute();


	int auxIdPartida = 0;
	do
	{
		res = pstmt->getResultSet();

		while (res->next())
		{
			auxIdPartida = res->getInt(1);
			idPartidasJugadas.push_back(auxIdPartida);
			//std::cout << "Resultado de consultar el la cantidad de partidas jugadas: " << res->getInt(1) << ", para el usuario con id: " << idPlayer << std::endl;
		}

	} while (pstmt->getMoreResults());

	partidasJugadas = idPartidasJugadas.size();

#pragma endregion

#pragma region Partidas_No_Ganadas

	for (int i = 0; i < idPartidasJugadas.size(); i++)
	{
		pstmt = conn->prepareStatement("SELECT idGanador FROM Partidas WHERE id=?");
		pstmt->setInt(1, idPartidasJugadas[i]);
		pstmt->execute();

		int auxId;
		do
		{
			res = pstmt->getResultSet();

			while (res->next())
			{
				auxId = res->getInt(1);
				if (auxId != idPlayer)
					partidasNoGanadas++;
				//std::cout << "Resultado de consultar el la cantidad de partidas jugadas: " << res->getInt(1) << ", para el usuario con id: " << idPlayer << std::endl;
			}

		} while (pstmt->getMoreResults());
	}

#pragma endregion

#pragma region Kills_Totales

	for (int i = 0; i < idPartidasJugadas.size(); i++)
	{
		pstmt = conn->prepareStatement("SELECT Kills FROM Partidas WHERE idUsuario=?");
		pstmt->setInt(1, idPlayer);
		pstmt->execute();

		do
		{
			res = pstmt->getResultSet();

			while (res->next())
			{
				totalKills += res->getInt(1);

				//std::cout << "Resultado de consultar el la cantidad de partidas jugadas: " << res->getInt(1) << ", para el usuario con id: " << idPlayer << std::endl;
			}

		} while (pstmt->getMoreResults());
	}

#pragma endregion

	conn->close();

	std::cout << "Datos de MMR del usuario que se ha logeado: " << std::endl;
	std::cout << "  -Partidas jugadas = " << partidasJugadas << std::endl;
	std::cout << "  -Partidas ganadas = " << partidasGanadas << std::endl;
	std::cout << "  -Total Deaths = " << partidasNoGanadas << std::endl;
	std::cout << "  -Total Kills = " << totalKills << std::endl;
	std::cout << std::endl;


	//3-LLamar a función de MMR

	//ComputeMMRFromPlayer(idPlayer);
	std::cout << "MMR value for this user === " << ((partidasGanadas / partidasJugadas) * 80) + ((totalKills / partidasNoGanadas) * 20) << std::endl;

}

void UpdatePlayerBBDDInfo(int idPlayer)
{
	/*Actualizar los datos del usuario a postpartida
		-Actualizar la partida que acaba de jugar/crearla
		-Actualizar tamaño, muertes, etc
	*/

	//1-Consultas de update de la base de datos

}

void ComputeMMRFromPlayer(int idPlayer)
{
	/*Computar el MMR del usuario
		MMR1=((NumVictorias/NumPartidasJugadas)*80)+(NumMuertesTotales*20);				+-5/+-10 para encontrar partida sería lo óptimo pero dada la escasez de jugadores para testear simplemente ordenarlos
		MMR2=((NumVictorias/NumPartidasJugadas)*80)+((NumMuertesTotales/NumPartidasNoGanadas)*20);				+-5/+-10 para encontrar partida sería lo óptimo pero dada la escasez de jugadores para testear simplemente ordenarlos
	*/


	//1-Calcular el MMR del usuario indicado

	//2-Actualizar su valor de MMR en la clase ClientProxy

}

void FoodUpdateThread()
{
	//sf::Clock clock;
	sf::Packet pack;

	while (true)
	{
		//sf::Time t1 = clock.getElapsedTime();
		//if (t1.asSeconds() > FOODUPDATETIMER)
		//{
			for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
			{
				pack << static_cast<int>(Protocol::FOOD_UPDATE);

				//tota la info del menjar:
				//només els que té més aprop
				std::vector<Food*> closeFood;
				for (int i = 0; i < (int)foodVector.size(); i++)
				{
					if (Distance(it->second->bodyPositions[0], foodVector[i]->position) < minFoodDist)
					{
						closeFood.push_back(foodVector[i]);
					}
				}
				//num foods:
				pack << static_cast<int>(closeFood.size());
				for (std::vector<Food*>::iterator closeit = closeFood.begin(); closeit != closeFood.end(); ++closeit)
				{
					Food* food = *closeit;
					//pack << food->id;
					pack << food->position.x;
					pack << food->position.y;
					//pack << food->color; //no puc passar el color?
				}

				sock.send(pack, it->second->ip, it->second->port);
				pack.clear();
			}

			//std::cout << "THREADS: FOOD going to sleep for " << FOODUPDATETIMER * 1000 << " Milliseconds." << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds((int)(FOODUPDATETIMER * 1000)));
			//std::cout << "THREADS: FOOD awakened" << std::endl;
			//clock.restart();
		//}
	}
}

void NewPlayer(sf::IpAddress ip, unsigned short port, sf::Packet pack)
{
	std::string alias;
	pack >> alias;

	bool clientExists = false;
	ClientProxy* newClient = nullptr;
	//busquem per ip i port si el client ja existeix:
	for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
	{
		ClientProxy* client = it->second;
		if (client->ip == ip && client->port == port)
		{
			std::cout << "Client already exists. The id is: "<<client->id << std::endl;
			newClient = client;
			clientExists = true;
			break;
		}
	}

	if (!clientExists)
	{
		std::cout << "creating new client" << std::endl;
		//és un nou player
		int id = (int)clientProxies.size();
		float x = rand() % SCREEN_WIDTH;
		float y = rand() % SCREEN_HEIGHT;
		sf::Vector2f headPos(x, y);
		newClient = new ClientProxy(id, alias, ip, port, headPos);
	}

	if (newClient == nullptr)
	{
		std::cout << "Problem with newClient" << std::endl;
		return;
	}

	//introduïm les dades del player
	pack.clear();
	pack << static_cast<int>(Protocol::WELCOME);
	newClient->AddDataToPacket(&pack);

	//std::cout << "INITIAL POS x (new Client) " << newClient->bodyPositions[0].x << " y: " << newClient->bodyPositions[0].y << std::endl;
	
	//els altres players:
	pack << static_cast<int>(clientProxies.size());
	for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
	{
		if (it->second->id != newClient->id)
		{
			it->second->AddDataToPacket(&pack);
		}
	}

	//tota la info del menjar:
	//només els que té més aprop
	std::vector<Food*> closeFood;
	for (int i = 0; i < (int)foodVector.size(); i++)
	{
		if (Distance(newClient->bodyPositions[0], foodVector[i]->position) < minFoodDist)
		{
			closeFood.push_back(foodVector[i]);
		}
	}

	//num foods:
	pack << static_cast<int>(closeFood.size());
	for (std::vector<Food*>::iterator it = closeFood.begin(); it != closeFood.end(); ++it)
	{
		Food* food = *it;
		pack << food->position.x;
		pack << food->position.y;
		//pack << food->color; //no puc passar el color?
	}

	sock.send(pack, ip, port);

	//Enviar NEW_PLAYER a todos los demás clientes
	for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
	{
		//com que cada vegada hem d'anar posant un id del packet diferent, anem tornant a omplir el packet
		pack.clear();
		pack << static_cast<int>(Protocol::NEW_PLAYER);
		newClient->AddDataToPacket(&pack);
		int idPack = (int)criticPackets.size();
		pack << idPack;

		CriticPack* cp = new CriticPack((int)criticPackets.size(), pack, it->second->ip, it->second->port);
		criticPackets[idPack] = cp;
		std::cout << "sending NEW_PLAYER" << std::endl;
		sock.send(pack, it->second->ip, it->second->port);
	}

	//finalment afegim el nou client al map
	if(!clientExists)
		clientProxies[newClient->id] = newClient;
}

void AccumMovement(sf::Packet pack)
{
	int idPlayer;
	int idMove;
	pack >> idPlayer>>idMove;
	int x, y;
	pack >> x >> y;

	sf::Vector2f sumPos;
	sumPos.x = (float)x / 1000.f;
	sumPos.y = (float)y / 1000.f;

	std::cout << "accummovement before" << std::endl;
	std::cout << "idPlayer: "<<idPlayer << std::endl;
	std::cout << "contains: "<< bool(clientProxies.find(idPlayer)!=clientProxies.end()) << std::endl;
	std::cout << "accummovement: " << clientProxies[idPlayer]->accumMovement.x << " " << clientProxies[idPlayer]->accumMovement.y <<std::endl;
	clientProxies[idPlayer]->accumMovement += sumPos;//en comptes de tractar-lo directament, l'acumulo i es tractarà en el thread de moviment
	clientProxies[idPlayer]->lastIdMove = idMove;
	std::cout << "accummovement after" << std::endl;
}

void FoodCollisionCheck(int pID, std::vector<sf::Vector2f> playerPositions, float playerBodyRadius)
{
	int i = 0;
	//std::cout << "Food vector size: " << (int)foodVector.size() << std::endl;
	while (i < (int)foodVector.size() - 1)
	{
		bool collided = false;
		for (int j = 0; j < (int)playerPositions.size(); j++)
		{
			if (Distance(foodVector[i]->position, playerPositions[j]) < playerBodyRadius)
			{
				collided = true;
				break;
			}
		}
		if (/*Collision*/collided)
		{
			//Food eaten
			foodVector.erase(foodVector.begin() + i);//comprovar que funcioni

			clientProxies[pID]->EatBall();
		}
		else
		{
			i++;
		}
	}
}

void MovementControl(int idPlayer, int idMove)
{
	sf::Vector2f possiblePos = clientProxies[idPlayer]->SumToHeadPosition();

	//controlar la posició
	if (possiblePos.x < 0 || possiblePos.x > SCREEN_WIDTH || possiblePos.y < 0 || possiblePos.y > SCREEN_HEIGHT)
	{
		//el posem just al límit de la pantalla? o simplement on era abans de moure's?
		//si no fem update position, simplement el retornarem on era.
	}
	else
	{
		//tot correcte, movem el player
		clientProxies[idPlayer]->UpdatePosition(possiblePos);
	}

	clientProxies[idPlayer]->accumMovement = sf::Vector2f(0,0);

	//ho enviem a tots els players
	sf::Packet pack;
	pack << static_cast<int>(Protocol::MOVE);
	pack << idPlayer;
	pack << idMove;

	clientProxies[idPlayer]->PutBodyPositions(&pack);

	std::cout << "body positions: " << (int)clientProxies[idPlayer]->bodyPositions.size() << std::endl;

	for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
	{
		sock.send(pack, it->second->ip, it->second->port);
	}

	FoodCollisionCheck(idPlayer, clientProxies[idPlayer]->bodyPositions, 13.f);//13 provisional
	PlayerCollisionCheck(idPlayer);
	//std::cout << "movement control" << std::endl;
}

void InitializeFood()
{
	for (int i = 0; i < maxFood; i++)
	{
		sf::Vector2f pos;
		pos.x = rand() % SCREEN_WIDTH;
		pos.y = rand() % SCREEN_HEIGHT;
		foodVector.push_back(new Food(pos));
	}
}

float GetRandomFloat()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0.f, 1.f);
	return dis(gen);
}

bool RandomPacketLost()
{
	return true;

	float f = GetRandomFloat();
	//std::cout << "random float is: " << f << std::endl;
	if (f > PERCENT_PACKETLOSS)
	{
		return true;//all good
	}

	return false;//packet lost
}

float Distance(sf::Vector2f v1, sf::Vector2f v2)
{
	sf::Vector2f v = v2 - v1;
	return sqrt(v.x*v.x + v.y*v.y);
}

void PlayerCollisionCheck(int idPlayer)
{
	bool kill = false;
	for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
	{
		if (it->first != idPlayer && !it->second->dead)
		{
			for (int i = 0; i < (int)it->second->bodyPositions.size(); i++)
			{
				if (Distance(clientProxies[idPlayer]->bodyPositions[0], it->second->bodyPositions[i]) <13.f)
				{
					std::cout << "collisioned" << std::endl;
					kill = true;
					break;
				}
			}
		}
	}

	if (kill)
	{
		//spawn balls
		for (int i = 0; i < (int)clientProxies[idPlayer]->bodyPositions.size(); i++)
		{
			Food* food = new Food(clientProxies[idPlayer]->bodyPositions[i]);
			foodVector.push_back(food);
		}

		sf::Packet pack;
		pack << Protocol::KILL;
		pack << idPlayer;

		for (std::map<int, ClientProxy*>::iterator it = clientProxies.begin(); it != clientProxies.end(); ++it)
		{
			if (sock.send(pack, it->second->ip, it->second->port) != sf::UdpSocket::Status::Done)
			{
				//error
			}
		}
		
		//clientProxies.erase(idPlayer);
		clientProxies[idPlayer]->dead = true;
	}
}