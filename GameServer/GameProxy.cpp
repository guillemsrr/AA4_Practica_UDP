#include "GameProxy.h"
#include <jdbc/mysql_connection.h>
#include <jdbc/mysql_driver.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/prepared_statement.h>



GameProxy::GameProxy()
{
}

GameProxy::GameProxy(int _id, std::vector<int> _arr)
{
	id = _id;
	//queryIDGame = 0;
	idPlayersInGame = _arr;
	InitializeFood();
}


GameProxy::~GameProxy()
{
}

void GameProxy::InitializeFood()
{
	std::lock_guard<std::mutex> guard(mtx_food);
	for (int i = 0; i < maxFood; i++)
	{
		sf::Vector2f pos;
		pos.x = rand() % SCREEN_WIDTH;
		pos.y = rand() % SCREEN_HEIGHT;
		foodVector.push_back(new Food(pos));
	}
}

void GameProxy::FoodCollisionCheck(std::map<int, ClientProxy*> &clientProxies, int pID, std::vector<sf::Vector2f> playerPositions, float playerBodyRadius)
{
	std::lock_guard<std::mutex> guard(mtx_food);
	int i = 0;
	//std::cout << "Food vector size: " << (int)foodVector.size() << std::endl;
	while (i < (int)foodVector.size() - 1)
	{
		bool collided = false;
		mtx_body.lock();
		for (int j = 0; j < (int)playerPositions.size(); j++)
		{
			if (Distance(foodVector[i]->position, playerPositions[j]) < playerBodyRadius)
			{
				collided = true;
				break;
			}
		}
		mtx_body.unlock();
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

float GameProxy::Distance(sf::Vector2f v1, sf::Vector2f v2)
{
	sf::Vector2f v = v2 - v1;
	return sqrt(v.x*v.x + v.y*v.y);
}

void GameProxy::MovementControl(sf::UdpSocket &sock, std::map<int, ClientProxy*> &clientProxies, int idPlayer, int idMove)
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

	clientProxies[idPlayer]->accumMovement = sf::Vector2f(0, 0);

	//ho enviem a tots els players
	sf::Packet pack;
	pack << static_cast<int>(Protocol::MOVE);
	pack << idPlayer;
	pack << idMove;

	mtx_body.lock();
	clientProxies[idPlayer]->PutBodyPositions(&pack);

	std::cout << "body positions: " << (int)clientProxies[idPlayer]->bodyPositions.size() << std::endl;
	mtx_body.unlock();

	for (int i=0; i<idPlayersInGame.size(); i++)
	{
		sock.send(pack, clientProxies[idPlayersInGame[i]]->ip, clientProxies[idPlayersInGame[i]]->port);
	}

	FoodCollisionCheck(clientProxies, idPlayer, clientProxies[idPlayer]->bodyPositions, 13.f);//13 provisional
	PlayerCollisionCheck(sock, clientProxies, idPlayer);
	//std::cout << "movement control" << std::endl;
}

void GameProxy::PlayerCollisionCheck(sf::UdpSocket &sock, std::map<int, ClientProxy*> &clientProxies, int idPlayer)
{
	bool kill = false;
	for (int i = 0; i < idPlayersInGame.size(); i++)
	{
		if (idPlayersInGame[i] != idPlayer && !clientProxies[idPlayersInGame[i]]->dead)
		{
			mtx_body.lock();
			for (int j = 0; j < (int)clientProxies[idPlayersInGame[i]]->bodyPositions.size(); j++)
			{
				if (Distance(clientProxies[idPlayer]->bodyPositions[0], clientProxies[idPlayersInGame[i]]->bodyPositions[j]) < 13.f)
				{
					std::cout << "collisioned" << std::endl;
					kill = true;
					clientProxies[idPlayersInGame[i]]->totalKills += 1;
					//std::cout << "ID al morir: " << idPlayersInGame[i] << ", " << "ID player: " << idPlayer << std::endl;
					clientProxies[idPlayer]->uState = UserState::LOBBY;
					break;
				}
			}
			mtx_body.unlock();
		}
	}

	if (kill)
	{
		//spawn balls
		mtx_body.lock();
		for (int i = 0; i < (int)clientProxies[idPlayer]->bodyPositions.size(); i++)
		{
			Food* food = new Food(clientProxies[idPlayer]->bodyPositions[i]);
			mtx_food.lock();
			foodVector.push_back(food);
			mtx_food.unlock();
		}
		mtx_body.unlock();

		sf::Packet pack;
		pack << Protocol::KILL;
		pack << idPlayer;

		clientProxies[idPlayer]->maxLongitud = clientProxies[idPlayer]->bodyPositions.size();
		std::cout << "LONGITUD DEL PERDEDOR: " << clientProxies[idPlayer]->bodyPositions.size() << std::endl;

		int aux = -1;

		for (int i = 0; i < idPlayersInGame.size(); i++)
		{
			if (sock.send(pack, clientProxies[idPlayersInGame[i]]->ip, clientProxies[idPlayersInGame[i]]->port) != sf::UdpSocket::Status::Done)
			{
				//error
			}
			if (idPlayersInGame[i] == idPlayer)
				aux = i;
		}
		UpdateGameBBDD(idPlayer, clientProxies[idPlayer]->queryId, clientProxies[idPlayer]->totalKills, clientProxies[idPlayer]->maxLongitud, false);
		idPlayersInGame.erase(idPlayersInGame.begin() +aux);
		querysIDGame.erase(querysIDGame.begin() + aux);
		clientProxies[idPlayer]->dead = true;
		


		if (idPlayersInGame.size() == 1)
		{
			sf::Packet pack2;
			pack2 << Protocol::WIN;
			clientProxies[idPlayersInGame[0]]->maxLongitud = clientProxies[idPlayersInGame[0]]->bodyPositions.size();
			clientProxies[idPlayersInGame[0]]->uState = UserState::LOBBY;
			sock.send(pack2, clientProxies[idPlayersInGame[0]]->ip, clientProxies[idPlayersInGame[0]]->port);
			UpdateGameBBDD(idPlayersInGame[0], clientProxies[idPlayersInGame[0]]->queryId, clientProxies[idPlayersInGame[0]]->totalKills, clientProxies[idPlayersInGame[0]]->maxLongitud, true);

			idPlayersInGame.clear();
			querysIDGame.clear();
		}
	}
}

void GameProxy::UpdateGameBBDD(int idPlayer, int queryId, int kills, int longitud, bool ganador)
{
	std::cout << "idplayer = " << idPlayer << ", kills = " << kills << ", longitud = " << longitud << ", ganador = " << ganador << std::endl;

	sql::Driver* driver = sql::mysql::get_driver_instance();
	sql::Connection* conn = driver->connect("tcp://www.db4free.net:3306", "slitheradmin", "123456789Admin");
	conn->setSchema("slitherudp");

	sql::PreparedStatement* pstmt;
	sql::ResultSet* res;

	for (int j = 0; j <idPlayersInGame.size(); j++)
	{
		if (idPlayersInGame[j] == idPlayer)
		{
			if (ganador)
			{
				pstmt = conn->prepareStatement("UPDATE Partidas SET Kills=?, MaxLongitud=?, idGanador=? WHERE id=?");
				pstmt->setInt(1, kills);
				pstmt->setInt(2, longitud);
				pstmt->setInt(3, queryId);
				pstmt->setInt(4, querysIDGame[j]);
				pstmt->executeUpdate();
			}
			else
			{
				std::cout << "idplayer = " << idPlayer << ", kills = " << kills << ", longitud = " << longitud << ", ganador = " << ganador << ", idGame = " << querysIDGame[j] << std::endl;

				pstmt = conn->prepareStatement("UPDATE Partidas SET Kills=?, MaxLongitud=? WHERE id=?");
				pstmt->setInt(1, kills);
				pstmt->setInt(2, longitud);
				pstmt->setInt(3, querysIDGame[j]);
				pstmt->executeUpdate();
			}

		}
	}

	conn->close();
}
