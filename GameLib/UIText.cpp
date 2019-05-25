#include "UIText.h"

UIText::UIText() :
	anchor(0, 0),
	pivot(0.0f, 0.0f)
{
}

UIText::UIText(std::string t, sf::Color c, sf::Font & font, float outlineThickness, int size, int xAnchor, int yAnchor, float xPivot, float yPivot) :
	pivot(xPivot, yPivot),
	anchor(xAnchor, yAnchor)
{
	text = * new sf::Text(t, font, size);
	text.setFillColor(c);
	text.setStyle(sf::Text::Bold);

	text.setOutlineThickness(outlineThickness);
	
	pos = sf::Rect<int>{ xAnchor - static_cast<int>(xPivot * text.getGlobalBounds().width) , yAnchor - static_cast<int>(yPivot * text.getGlobalBounds().height), static_cast<int>(text.getGlobalBounds().width), static_cast<int>(text.getGlobalBounds().height)};

	text.setPosition(pos.left, pos.top);
}

UIText::~UIText()
{
}

void UIText::SetText(std::string t)
{
	text.setString(t);

	pos.left = anchor.x - static_cast<int>(text.getGlobalBounds().width * pivot.x);
	pos.top = anchor.y - static_cast<int>(text.getGlobalBounds().height * pivot.y);

	text.setPosition(pos.left, pos.top);
}

sf::Vector2i UIText::GetSize()
{
	return sf::Vector2i(text.getGlobalBounds().width, text.getGlobalBounds().height);
}

void UIText::SetPosition(sf::Vector2i newAnchor, sf::Vector2f newPivot)
{
	pos.left = newAnchor.x - static_cast<int>(text.getGlobalBounds().width * newPivot.x);
	pos.top = newAnchor.y - static_cast<int>(text.getGlobalBounds().height * newPivot.y);

	pivot = newPivot;
	anchor = newAnchor;

	text.setPosition(pos.left, pos.top);
}

sf::Text UIText::GetText()
{
	return text;
}

void UIText::SetFont(sf::Font f)
{
	text.setFont(f);
}

void UIText::SetColor(sf::Color color)
{
	text.setFillColor(color);
}
