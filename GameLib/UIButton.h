#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include "UIText.h"


#pragma region COLOR_CONSTANTS
///Color button colors
//Color button outline color
const sf::Color CBTN_OUTLINE_COLOR = sf::Color(255, 205, 7);

//Color button inner colors
//RED - RED
const sf::Color BTN_RED_DEF_COLOR = sf::Color(255, 45, 45);
const sf::Color BTN_RED_HOV_COLOR = sf::Color(252, 98, 98);
const sf::Color BTN_RED_CLK_COLOR = sf::Color(196, 54, 54);
const sf::Color BTN_RED_DIS_COLOR = sf::Color(94);

//ORANGE - ORG
const sf::Color BTN_ORG_DEF_COLOR = sf::Color(255, 114, 0);
const sf::Color BTN_ORG_HOV_COLOR = sf::Color(255, 151, 66);
const sf::Color BTN_ORG_CLK_COLOR = sf::Color(196, 94, 11);
const sf::Color BTN_ORG_DIS_COLOR = sf::Color(94);

//YELLOW - YLW
const sf::Color BTN_YLW_DEF_COLOR = sf::Color(255, 225, 0);
const sf::Color BTN_YLW_HOV_COLOR = sf::Color(255, 232, 63);
const sf::Color BTN_YLW_CLK_COLOR = sf::Color(214, 189, 0);
const sf::Color BTN_YLW_DIS_COLOR = sf::Color(94);

//GREEN - GRN
const sf::Color BTN_GRN_DEF_COLOR = sf::Color(77, 255, 22);
const sf::Color BTN_GRN_HOV_COLOR = sf::Color(125, 255, 86);
const sf::Color BTN_GRN_CLK_COLOR = sf::Color(64, 206, 20);
const sf::Color BTN_GRN_DIS_COLOR = sf::Color(94);

//TURQUOISE - TRQ
const sf::Color BTN_TRQ_DEF_COLOR = sf::Color(0, 255, 157);
const sf::Color BTN_TRQ_HOV_COLOR = sf::Color(66, 255, 182);
const sf::Color BTN_TRQ_CLK_COLOR = sf::Color(19, 214, 139);
const sf::Color BTN_TRQ_DIS_COLOR = sf::Color(94);

//CYAN - CYN
const sf::Color BTN_CYN_DEF_COLOR = sf::Color(0, 216, 255);
const sf::Color BTN_CYN_HOV_COLOR = sf::Color(73, 227, 255);
const sf::Color BTN_CYN_CLK_COLOR = sf::Color(21, 185, 214);
const sf::Color BTN_CYN_DIS_COLOR = sf::Color(94);

//INDIGO - IDG
const sf::Color BTN_IDG_DEF_COLOR = sf::Color(2, 32, 229);
const sf::Color BTN_IDG_HOV_COLOR = sf::Color(55, 81, 252);
const sf::Color BTN_IDG_CLK_COLOR = sf::Color(18, 43, 204);
const sf::Color BTN_IDG_DIS_COLOR = sf::Color(94);

//VIOLET - VLT
const sf::Color BTN_VLT_DEF_COLOR = sf::Color(118, 21, 237);
const sf::Color BTN_VLT_HOV_COLOR = sf::Color(129, 46, 232);
const sf::Color BTN_VLT_CLK_COLOR = sf::Color(97, 19, 193);
const sf::Color BTN_VLT_DIS_COLOR = sf::Color(94);

///Default button colors
//btn rectangle colors
const sf::Color BTN_DEF_COLOR = sf::Color(219, 161, 37);
const sf::Color BTN_HOV_COLOR = sf::Color(255, 205, 7);
const sf::Color BTN_CLK_COLOR = sf::Color(153, 113, 27);
const sf::Color BTN_DIS_COLOR = sf::Color(94);

//Btn text colors
const sf::Color TXT_DEF_COLOR = sf::Color(89);
const sf::Color TXT_HOV_COLOR = sf::Color(120);
const sf::Color TXT_CLK_COLOR = sf::Color(60);
const sf::Color TXT_DIS_COLOR = sf::Color(40);

#pragma endregion

enum class ButtonState
{
	DEFAULT,
	HOVER,
	CLICK
};

class UIButton
{
public:
	UIButton();
	UIButton(std::string t, sf::Font & font, sf::Vector2f size, sf::Vector2i anch, sf::Vector2f pvt);
	~UIButton();

	bool showText;
private:
	sf::RectangleShape rectangle;
	UIText btnText;
	std::string text;
	bool interactable;

	ButtonState btnState;

	sf::Rect<int> pos;
	sf::Vector2i anchor;
	sf::Vector2f pivot;

	sf::Color btn_def_color;
	sf::Color btn_hov_color;
	sf::Color btn_clk_color;
	sf::Color btn_dis_color;

	sf::Color txt_def_color;
	sf::Color txt_hov_color;
	sf::Color txt_clk_color;
	sf::Color txt_dis_color;

public:
	void SetPosition(sf::Vector2i newAnchor, sf::Vector2f newPivot);
	bool CheckCursorCollision(sf::Vector2i mouseCoords);
	void SetInteractable(bool b);
	bool GetInteractable();
	void SetHover();
	void SetPressed();
	void SetDefault();

	void OverrideBtnInnerColors(sf::Color defColor, sf::Color hovColor, sf::Color clkColor, sf::Color disColor);
	void OverrideBtnOutlineColors(sf::Color outlineColor);
	void OverrideTxtColors(sf::Color defColor, sf::Color hovColor, sf::Color clkColor, sf::Color disColor);

	ButtonState GetButtonState();

	void SetFont(sf::Font f);

	sf::RectangleShape GetRectangle();
	sf::Text GetText();
};

