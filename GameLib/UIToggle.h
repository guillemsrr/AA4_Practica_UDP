#pragma once
#include <SFML/Graphics.hpp>

//btn rectangle colors
const sf::Color TGL_DEF_COLOR = sf::Color(219, 161, 37);
const sf::Color TGL_HOV_COLOR = sf::Color(255, 205, 7);
const sf::Color TGL_CLK_COLOR = sf::Color(153, 113, 27);
const sf::Color TGL_DIS_COLOR = sf::Color(94);

class UIToggle
{
public:
	//UIToggle();
	UIToggle(int id, sf::Vector2f size, sf::Vector2i anch, sf::Vector2f pvt);
	~UIToggle();

private:
	sf::RectangleShape rectangle;
	sf::RectangleShape checkmark;
	bool interactable;
	bool isOn = false;

	void Toggle();

	sf::Rect<int> pos;
	sf::Rect<int> posCheckmark;
	sf::Vector2i anchor;
	sf::Vector2f pivot;

	int ID;
public:
	int GetID();
	void SetPosition(sf::Vector2i newAnchor, sf::Vector2f newPivot);
	bool CheckCursorCollision(sf::Vector2i mouseCoords);
	void SetInteractable(bool b);
	bool GetInteractable();
	void SetHover();
	void SetPressed();
	void SetDefault();

	sf::RectangleShape GetRectangle();
	sf::RectangleShape GetCheckmark();
	bool GetIsOn();

};

