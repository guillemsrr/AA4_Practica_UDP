#include "Board.h"



Board::Board()
{
	for (int i = 0; i < 100; i++)
	{
		sf::CircleShape* newFood = new sf::CircleShape(5.f);
		newFood->setOrigin(2.5f, 2.5f);
		newFood->setPosition(0.f, 0.f);
		newFood->setFillColor(sf::Color::Blue);

		foodShapeBuffer.push_back(newFood);
	}
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

	//paint foodballs:
	for (int i = 0; i < (int)foodPositions.size(); i++)
	{
		foodShapeBuffer[i]->setPosition(foodPositions[i]);
		window.draw(*foodShapeBuffer[i]);
	}

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

void Board::DrawFood(sf::CircleShape ball)
{
	window.draw(ball);
}

void Board::Commands(Player* player)
{
	float deltaTime = frameTimeClock.getElapsedTime().asSeconds();
	frameTimeClock.restart();
	sf::Event event;
	playerMovement = sf::Vector2f(0, 0);
	//std::cout << "commands"<<std::endl;

	while (window.pollEvent(event))
	{
		//std::cout << "commands while"<<std::endl;
		if (event.key.code == sf::Keyboard::Key::Escape)
		{
			window.close();
		}
	}

	if (window.hasFocus())
	{
		bool left = sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
		bool right = sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
		bool up = sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
		bool down = sf::Keyboard::isKeyPressed(sf::Keyboard::Down);

		if (left || right || up || down)
		{
			//std::cout << "Player moving" << std::endl;
			bool sprint = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

			sf::Vector2f mvDir = sf::Vector2f(0.f, 0.f);

			mvDir.x += (right ? 1.f : 0.f) - (left ? 1.f : 0.f);
			mvDir.y += (down ? 1.f : 0.f) - (up ? 1.f : 0.f);


			sf::Vector2f zero = sf::Vector2f(0.f, 0.f);
			sf::Vector2f v = zero - mvDir;
			float dist = sqrt(v.x*v.x + v.y*v.y);
			mvDir /= dist;

			//std::cout << "Player movement: " << mvDir.x << ", " << mvDir.y << std::endl;
			//std::cout << "Bools: right - " << right << ", left - " << left << ", up - " << up << ", down - " << down << std::endl;
			playerMovement += mvDir * (sprint ? 2.f : 1.f) * player->speed * deltaTime;
		}
	}	
}
