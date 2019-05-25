#include "Player.h"



Player::Player()
{
}

Player::Player(sf::Packet* pack) :
	PlayerInfo(-1, "", 0, 0, SkinColors::DEFAULT)
{
	*pack >> appId;
	*pack >> alias;
	int aux;
	*pack >> aux;
	SetPlayerColor(static_cast<SkinColors>(aux));
	AddBodyPositions(pack);
	speed = 125.0f;
}

Player::~Player()
{
}

void Player::AddBodyPositions(sf::Packet* pack)
{
	int numPos;
	*pack >> numPos;
	
	for (int i = 0; i < numPos; i++)
	{
		sf::Vector2f pos;
		int x, y;
		*pack >> x;
		*pack >> y;

		pos.x = (float)x / 1000.f;
		pos.y = (float)y / 1000.f;
		bodyPositions.push_back(pos);
	}

	bodyPositions[0];
}

void Player::UpdatePosition(sf::Packet* pack)
{
	int numPos;
	*pack >> numPos;
	
	while (numPos > (int)bodyPositions.size())
	{
		CreateBodyPosition();
	}

	for (int i = 0; i < bodyPositions.size(); i++)
	{
		*pack >> bodyPositions[i].x;
		*pack >> bodyPositions[i].y;
	}
}

std::vector<sf::Vector2f> Player::GetFuturePositions(sf::Packet* pack)
{
	//std::cout << "GetFuturePositions before" << std::endl;
	int numPos;
	*pack >> numPos;

	while (numPos > (int)bodyPositions.size())
	{
		CreateBodyPosition();
	}

	std::vector<sf::Vector2f> futureBodyPositions;

	for (int i = 0; i < numPos; i++)
	{
		sf::Vector2f newPos;
		int x, y;
		*pack >> x;
		*pack >> y;
		newPos.x = (float)x / 1000.f;
		newPos.y = (float)y / 1000.f;
		futureBodyPositions.push_back(newPos);
	}

	startBodyPositions = bodyPositions;

	//std::cout << "GetFuturePositions after" << std::endl;

	return futureBodyPositions;
}

void Player::UpdateTheRestOfPositions(int numPos, sf::Vector2f headPos, sf::Packet* pack)
{
	while (numPos > (int)bodyPositions.size())
	{
		CreateBodyPosition();
	}

	bodyPositions[0] = headPos;

	for (int i = 1; i < bodyPositions.size(); i++)
	{
		int x, y;
		*pack >> x;
		*pack >> y;

		bodyPositions[i].x = (float)x / 1000.f;
		bodyPositions[i].y = (float)y / 1000.f;
	}
}


void Player::UpdatePosition(sf::Vector2f move)
{

	bodyPositions[0] += move;

	for (int i = 1; i < (int)bodyPositions.size(); i++)
	{
		sf::Vector2f dir = bodyPositions[i] - bodyPositions[i - 1];
		Normalize(dir);
		bodyPositions[i] = bodyPositions[i - 1] + dir * 10.f;
		//std::cout << "update position to "<< bodyPositions[i].x << " " << bodyPositions[i].y<< std::endl;
	}


	/*for (int i = (int)bodyPositions.size() - 1; i > 0; i--)
	{
		bodyPositions[i] = bodyPositions[i - 1];
	}
	bodyPositions[0] += move;*/
}


bool Player::InterpolateTo(std::vector<sf::Vector2f> finalBodyPositions, float percent)
{
	bool arrived = false;
	for (int i = 0; i < bodyPositions.size(); i++)
	{
		sf::Vector2f diff = finalBodyPositions[i] - startBodyPositions[i];//d'aquesta manera sempre ens mourem el mateix desplaçament

		sf::Vector2f possiblePos = bodyPositions[i] + percent * diff;

		//ajustem:
		if (!arrived)
		{
			if (Magnitude(possiblePos) - Magnitude(finalBodyPositions[i]) <= 0.001f)//no caldria fer el mateix càlcul per cada cos
			{
				bodyPositions[i] = finalBodyPositions[i];
				arrived = true;
			}
			else
			{
				bodyPositions[i] = possiblePos;
			}
		}
		else
		{
			bodyPositions[i] = finalBodyPositions[i];
		}
	}

	return arrived;
}

float Player::Magnitude(sf::Vector2f headPos)
{
	return sqrt((headPos.x * headPos.x) + (headPos.y * headPos.y));
}

