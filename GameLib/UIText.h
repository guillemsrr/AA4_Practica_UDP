#pragma once
#include <string>
#include <iostream>
#include <SFML/Graphics.hpp>

class UIText
{
private:
	sf::Text text;
	
	sf::Rect<int> pos;
	sf::Vector2i anchor;
	sf::Vector2f pivot;

public:
	UIText();
	UIText(std::string t, sf::Color c, sf::Font & font, float outlineThickness, int size, int xAnchor, int yAnchor, float xPivot, float yPivot);
	~UIText();

	void SetText(std::string t);
	sf::Vector2i GetSize();
	void SetPosition(sf::Vector2i anch, sf::Vector2f pvt);
	sf::Text GetText();

	void SetFont(sf::Font f);
	void SetColor(sf::Color);
};

