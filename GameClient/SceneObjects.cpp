#include "SceneObjects.h"



SceneObjects::SceneObjects()
{
}

SceneObjects::~SceneObjects()
{
}

void SceneObjects::AddStandaloneRect(sf::Vector2f size, sf::Vector2f position, sf::Color fillColor, float outlineThickness, sf::Color outlineColor)
{
	sf::RectangleShape rect = sf::RectangleShape();
	rect.setPosition(position);
	rect.setSize(size);
	rect.setFillColor(fillColor);
	rect.setOutlineThickness(outlineThickness);
	rect.setOutlineColor(outlineColor);
	standaloneRects.push_back(rect);
}

void SceneObjects::AddButton(std::string btnID, sf::Vector2f size, sf::Vector2i anchorPosition, sf::Vector2f pivot, sf::Font& font, std::string text)
{
	buttonIDs.push_back(btnID);
	UIButton btn = UIButton(text, font, size, anchorPosition, pivot);
	buttons[btnID] = btn;
}

void SceneObjects::AddText(std::string text, sf::Color color, sf::Font& font, float outlineThickness, int size, sf::Vector2i anchorPosition, sf::Vector2f pivot, sf::Color outlineColor)
{
	UIText txt = UIText(text, color, font, outlineThickness, size, anchorPosition.x, anchorPosition.y, pivot.x, pivot.y);
	txt.GetText().setOutlineColor(outlineColor);
	texts.push_back(txt);
}

void SceneObjects::DrawScene(sf::RenderWindow & window)
{
	//window.clear(); //No podem cridar board.draw al mateix temps per aixo
	for (int i = 0; i < (int)standaloneRects.size(); i++)
	{
		window.draw(standaloneRects[i]);
	}

	for (int i = 0; i < (int)buttonIDs.size(); i++)
	{
		window.draw(buttons[buttonIDs[i]].GetRectangle());
		if (buttons[buttonIDs[i]].showText)
			window.draw(buttons[buttonIDs[i]].GetText());
	}

	for (int i = 0; i < (int)texts.size(); i++)
	{
		window.draw(texts[i].GetText());
	}
	//window.display();
}
