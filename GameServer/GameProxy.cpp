#include "GameProxy.h"



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

		idPlayersInGame.erase(idPlayersInGame.begin() +aux);
		clientProxies[idPlayer]->dead = true;

		if (idPlayersInGame.size() == 1)
		{
			sf::Packet pack2;
			pack2 << Protocol::WIN;
			clientProxies[idPlayersInGame[0]]->uState = UserState::LOBBY;
			sock.send(pack2, clientProxies[idPlayersInGame[0]]->ip, clientProxies[idPlayersInGame[0]]->port);
		}
	}
}
