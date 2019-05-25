#pragma once
#include <vector>
#include <map>
#include <string>
#include <SFML/Graphics.hpp>
#include <UIButton.h>
#include <UIText.h>

const sf::Color MENUS_BG_RECT_COLOR = sf::Color(133, 221, 189);


class SceneObjects
{
public:
	SceneObjects();
	~SceneObjects();

	std::vector<sf::RectangleShape> standaloneRects;
	std::vector<std::string> buttonIDs;
	std::map<std::string, UIButton> buttons;
	std::vector<UIText> texts;

	void AddStandaloneRect(sf::Vector2f size, sf::Vector2f position, sf::Color fillColor, float outlineThickness = 0.f, sf::Color outlineColor = sf::Color::Black);
	void AddButton(std::string btnID, sf::Vector2f size, sf::Vector2i anchorPosition, sf::Vector2f pivot, sf::Font& font, std::string text = "-");
	void AddText(std::string text, sf::Color color, sf::Font& font, float outlineThickness, int size, sf::Vector2i anchorPosition, sf::Vector2f pivot, sf::Color outlineColor = sf::Color::Black);

	void DrawScene(sf::RenderWindow& window);
};

