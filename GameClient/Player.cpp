#include "Player.h"



Player::Player()
{
}

Player::Player(sf::Packet* pack)
{
	*pack >> id;
	*pack >> alias;
	AddBodyPositions(pack);
	speed = 2;
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
		sf::Vector2i pos;
		*pack >> pos.x;
		*pack >> pos.y;
		bodyPositions.push_back(pos);
	}
}

void Player::UpdatePosition(sf::Packet* pack)
{
	int numPos;
	*pack >> numPos;
	int difference = numPos - (int)bodyPositions.size();
	if (difference > 0)
	{
		//add new positions, just initialized
		for (int i = 0; i < difference; i++)
		{
			sf::Vector2i pos(0, 0);
			bodyPositions.push_back(pos);
		}
	}

	for (int i = 0; i < bodyPositions.size(); i++)
	{
		*pack >> bodyPositions[i].x;
		*pack >> bodyPositions[i].y;
	}
}

std::vector<sf::Vector2i> Player::GetFuturePositions(sf::Packet* pack)
{
	int numPos;
	*pack >> numPos;
	std::vector<sf::Vector2i> futureBodyPositions;

	for (int i = 0; i < numPos; i++)
	{
		sf::Vector2i newPos;
		*pack >> newPos.x;
		*pack >> newPos.y;
		futureBodyPositions.push_back(newPos);
	}

	startBodyPositions = bodyPositions;

	return futureBodyPositions;
}

void Player::UpdateTheRestOfPositions(int numPos, sf::Vector2i headPos, sf::Packet* pack)
{
	int difference = numPos - (int)bodyPositions.size();
	if (difference > 0)
	{
		//add new positions, just initialized
		for (int i = 0; i < difference; i++)
		{
			sf::Vector2i pos(0, 0);
			bodyPositions.push_back(pos);
		}
	}

	bodyPositions[0] = headPos;

	for (int i = 1; i < bodyPositions.size(); i++)
	{
		*pack >> bodyPositions[i].x;
		*pack >> bodyPositions[i].y;
	}
}


void Player::UpdatePosition(sf::Vector2i move)
{

	bodyPositions[0] += move;

	for (int i = 1; i < (int)bodyPositions.size(); i++)
	{
		sf::Vector2i dir = bodyPositions[i] - bodyPositions[i - 1];
		Normalize(dir);
		bodyPositions[i] = bodyPositions[i - 1] + dir * 10;
	}

	/*for (int i = (int)bodyPositions.size() - 1; i > 0; i--)
	{
		bodyPositions[i] = bodyPositions[i - 1];
	}
	bodyPositions[0] += move;*/
}


bool Player::InterpolateTo(std::vector<sf::Vector2i> finalBodyPositions, int percent)
{
	bool arrived = false;
	for (int i = 0; i < bodyPositions.size(); i++)
	{
		sf::Vector2i diff = finalBodyPositions[i] - startBodyPositions[i];//d'aquesta manera sempre ens mourem el mateix desplaçament

		sf::Vector2i possiblePos = bodyPositions[i] + percent * diff;

		//ajustem:
		if (!arrived)
		{
			if (Magnitude(possiblePos) - Magnitude(finalBodyPositions[i]) <= 1)//no caldria fer el mateix càlcul per cada cos
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

int Player::Magnitude(sf::Vector2i headPos)
{
	return (int)sqrt((headPos.x * headPos.x) + (headPos.y * headPos.y));
}

