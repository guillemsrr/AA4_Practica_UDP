#include "UIToggle.h"

UIToggle::UIToggle(int id, sf::Vector2f size, sf::Vector2i anch, sf::Vector2f pvt) :
	ID(id)
{
	rectangle.setSize(size);
	pos = sf::Rect<int>{ anch.x - static_cast<int>(pvt.x * size.x) , anch.y - static_cast<int>(pvt.y * size.y), static_cast<int>(size.x), static_cast<int>(size.y) };
	posCheckmark = sf::Rect<int>{pos.left, pos.top, static_cast<int>(size.x - 4.0f), static_cast<int>(size.y - 4.0f) };
	posCheckmark.left = pos.left + pos.width / 2.0f - posCheckmark.width / 2.0f;
	posCheckmark.top = pos.top + pos.height / 2.0f - posCheckmark.height / 2.0f;
	rectangle.setPosition(pos.left, pos.top);
	checkmark.setPosition(posCheckmark.left, posCheckmark.top);
	interactable = true;
	isOn = false;

	checkmark.setSize(size - sf::Vector2f(4.0f, 4.0f));

	rectangle.setOutlineThickness(5.0f);
	rectangle.setFillColor(sf::Color(25.0f, 25.0f, 25.0f, 255.0f));
	rectangle.setOutlineColor(TGL_DEF_COLOR);
	
	checkmark.setFillColor(TGL_DEF_COLOR);
}


UIToggle::~UIToggle()
{
}

void UIToggle::Toggle()
{
	isOn = !isOn;
}

sf::RectangleShape UIToggle::GetRectangle()
{
	return rectangle;
}

sf::RectangleShape UIToggle::GetCheckmark()
{
	return checkmark;
}

bool UIToggle::GetIsOn()
{
	return isOn;
}

int UIToggle::GetID()
{
	return ID;
}

void UIToggle::SetPosition(sf::Vector2i newAnchor, sf::Vector2f newPivot)
{
	pos.left = newAnchor.x - static_cast<int>(pos.width * newPivot.x);
	pos.top = newAnchor.y - static_cast<int>(pos.height * newPivot.y);

	posCheckmark.left = pos.left + pos.width / 2.0f - posCheckmark.width / 2.0f;
	posCheckmark.top = pos.top + pos.height / 2.0f - posCheckmark.height / 2.0f;

	pivot = newPivot;
	anchor = newAnchor;

	rectangle.setPosition(pos.left, pos.top);
	checkmark.setPosition(posCheckmark.left, posCheckmark.top);
}

bool UIToggle::CheckCursorCollision(sf::Vector2i mouseCoords)
{
	return mouseCoords.x > rectangle.getGlobalBounds().left && mouseCoords.x < rectangle.getGlobalBounds().left + rectangle.getGlobalBounds().width &&
		mouseCoords.y > rectangle.getGlobalBounds().top && mouseCoords.y < rectangle.getGlobalBounds().top + rectangle.getGlobalBounds().height;
}

void UIToggle::SetInteractable(bool b)
{
	interactable = b;
	if (!b)
	{
		rectangle.setOutlineColor(TGL_DIS_COLOR);
		checkmark.setFillColor(TGL_DIS_COLOR);
	}
	else
	{
		rectangle.setOutlineColor(TGL_DEF_COLOR);
		checkmark.setFillColor(TGL_DEF_COLOR);
	}
}

bool UIToggle::GetInteractable()
{
	return interactable;
}

void UIToggle::SetHover()
{
	rectangle.setOutlineColor(TGL_HOV_COLOR);
	checkmark.setFillColor(TGL_HOV_COLOR);
}

void UIToggle::SetPressed()
{
	rectangle.setOutlineColor(TGL_CLK_COLOR);
	checkmark.setFillColor(TGL_CLK_COLOR);
	Toggle();
}

void UIToggle::SetDefault()
{
	rectangle.setOutlineColor(TGL_DEF_COLOR);
	checkmark.setFillColor(TGL_DEF_COLOR);
}
