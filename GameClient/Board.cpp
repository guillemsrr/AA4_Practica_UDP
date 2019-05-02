#include "Board.h"



Board::Board()
{
}


Board::~Board()
{
}

void Board::InitializeSlither(Player* player)
{
	Slither* s = new Slither(player);
	slithersMap[player->id] = s;
}

void Board::UpdateSlithers()
{
	for (std::map<int, Slither*>::iterator it = slithersMap.begin(); it != slithersMap.end(); ++it)
	{
		Slither* slither = it->second;
		slither->UpdateSlitherPosition();
	}
}

void Board::UpdateSlither(int id)
{
	slithersMap[id]->UpdateSlitherPosition();
}


void Board::DrawBoard()
{
	window.clear();

	//paint all players
	for (std::map<int, Slither*>::iterator it = slithersMap.begin(); it != slithersMap.end(); ++it)
	{
		Slither* slither = it->second;
		for each (sf::CircleShape circle in slither->bodyCircles)
		{
			window.draw(circle);
		}
	}

	window.display();
}

void Board::Commands()
{
	sf::Event event;
	playerMovement = sf::Vector2f(0, 0);
	//std::cout << "commands"<<std::endl;

	while (window.pollEvent(event))
	{
		//std::cout << "commands while"<<std::endl;
		switch (event.key.code)
		{
		case sf::Keyboard::Key::Escape:
			window.close();
			break;
		case sf::Keyboard::Key::Left:
			playerMovement.x -= speed;
			break;
		case sf::Keyboard::Key::Right:
			playerMovement.x += speed;
			break;
		case sf::Keyboard::Key::Up:
			playerMovement.y -= speed;
			break;
		case sf::Keyboard::Key::Down:
			playerMovement.y += speed;
			break;
		default:
			break;

		}
	}
}
