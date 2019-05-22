#include "Player.h"



Player::Player()
{
}

Player::Player(sf::Packet* pack)
{
	*pack >> id;
	*pack >> alias;
	AddBodyPositions(pack);
	speed = 100.0f;
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
			sf::Vector2f pos(0.f, 0.f);
			bodyPositions.push_back(pos);
		}
	}

	for (int i = 0; i < bodyPositions.size(); i++)
	{
		*pack >> bodyPositions[i].x;
		*pack >> bodyPositions[i].y;
	}
}

std::vector<sf::Vector2f> Player::GetFuturePositions(sf::Packet* pack)
{
	int numPos;
	*pack >> numPos;
	std::vector<sf::Vector2f> futureBodyPositions;

	for (int i = 0; i < numPos; i++)
	{
		sf::Vector2f newPos;
		*pack >> newPos.x;
		*pack >> newPos.y;
		futureBodyPositions.push_back(newPos);
	}

	startBodyPositions = bodyPositions;

	return futureBodyPositions;
}

void Player::UpdateTheRestOfPositions(int numPos, sf::Vector2f headPos, sf::Packet* pack)
{
	int difference = numPos - (int)bodyPositions.size();
	if (difference > 0)
	{
		//add new positions, just initialized
		for (int i = 0; i < difference; i++)
		{
			sf::Vector2f pos(0.f, 0.f);
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


void Player::UpdatePosition(sf::Vector2f move)
{
	for (int i = (int)bodyPositions.size() - 1; i > 0; i--)
	{
		bodyPositions[i] = bodyPositions[i - 1];
	}
	bodyPositions[0] += move;
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

