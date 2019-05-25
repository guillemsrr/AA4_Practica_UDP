#include "UIButton.h"



UIButton::UIButton()
{
}

UIButton::UIButton(std::string t, sf::Font & font, sf::Vector2f size, sf::Vector2i anch, sf::Vector2f pvt)
{
	rectangle.setSize(size);
	pos = sf::Rect<int>{ anch.x - static_cast<int>(pvt.x * size.x) , anch.y - static_cast<int>(pvt.y * size.y), static_cast<int>(size.x), static_cast<int>(size.y)};
	text = t;
	rectangle.setPosition(pos.left, pos.top);
	interactable = true;

	btn_def_color = BTN_DEF_COLOR;
	btn_hov_color = BTN_HOV_COLOR;
	btn_clk_color = BTN_CLK_COLOR;
	btn_dis_color = BTN_DIS_COLOR;

	txt_def_color = TXT_DEF_COLOR;
	txt_hov_color = TXT_HOV_COLOR;
	txt_clk_color = TXT_CLK_COLOR;
	txt_dis_color = TXT_DIS_COLOR;

	btnText = UIText(t, txt_def_color, font, .3f, 22, pos.left + pos.width / 2.0f, pos.top + pos.height / 2.0f, 0.5f, 0.5f);

	rectangle.setFillColor(btn_def_color);
	rectangle.setOutlineThickness(2.0f);
	rectangle.setOutlineColor(sf::Color::Black);

	
	showText = true;
	btnState = ButtonState::DEFAULT;
}



UIButton::~UIButton()
{
}

void UIButton::SetPosition(sf::Vector2i newAnchor, sf::Vector2f newPivot)
{
	pos.left = newAnchor.x - static_cast<int>(rectangle.getGlobalBounds().width * newPivot.x);
	pos.top = newAnchor.y - static_cast<int>(rectangle.getGlobalBounds().height * newPivot.y);

	pivot = newPivot;
	anchor = newAnchor;

	rectangle.setPosition(pos.left, pos.top);
}

bool UIButton::CheckCursorCollision(sf::Vector2i mouseCoords)
{
	return mouseCoords.x > rectangle.getGlobalBounds().left && mouseCoords.x < rectangle.getGlobalBounds().left + rectangle.getGlobalBounds().width &&
		mouseCoords.y > rectangle.getGlobalBounds().top && mouseCoords.y < rectangle.getGlobalBounds().top + rectangle.getGlobalBounds().height;
}

void UIButton::SetInteractable(bool b)
{
	interactable = b;
	if (!b)
	{
		rectangle.setFillColor(btn_dis_color);
		btnText.SetColor(txt_dis_color);
	}
	else
	{
		switch (btnState)
		{
		case ButtonState::DEFAULT:
			rectangle.setFillColor(btn_def_color);
			btnText.SetColor(txt_def_color);
			break;
		case ButtonState::HOVER:
			rectangle.setFillColor(btn_hov_color);
			btnText.SetColor(txt_hov_color);
			break;
		case ButtonState::CLICK:
			rectangle.setFillColor(btn_clk_color);
			btnText.SetColor(txt_clk_color);
			break;
		}		
	}
}

bool UIButton::GetInteractable()
{
	return interactable;
}

void UIButton::SetHover()
{
	rectangle.setFillColor(btn_hov_color);
	btnText.SetColor(txt_hov_color);
	btnState = ButtonState::HOVER;
}

void UIButton::SetPressed()
{
	rectangle.setFillColor(btn_clk_color);
	btnText.SetColor(txt_clk_color);
	btnState = ButtonState::CLICK;
}

void UIButton::SetDefault()
{
	rectangle.setFillColor(btn_def_color);
	btnText.SetColor(txt_def_color);
	btnState = ButtonState::DEFAULT;
}

void UIButton::OverrideBtnInnerColors(sf::Color defColor, sf::Color hovColor, sf::Color clkColor, sf::Color disColor)
{
	btn_def_color = defColor;
	btn_hov_color = hovColor;
	btn_clk_color = clkColor;
	btn_dis_color = disColor;

	if (interactable)
	{
		switch (btnState)
		{
		case ButtonState::DEFAULT:
			rectangle.setFillColor(btn_def_color);
			break;
		case ButtonState::HOVER:
			rectangle.setFillColor(btn_hov_color);
			break;
		case ButtonState::CLICK:
			rectangle.setFillColor(btn_clk_color);
			break;
		}
	}
	else
	{
		rectangle.setFillColor(btn_dis_color);
	}
}

void UIButton::OverrideBtnOutlineColors(sf::Color outlineColor)
{
	rectangle.setOutlineColor(outlineColor);
}

void UIButton::OverrideTxtColors(sf::Color defColor, sf::Color hovColor, sf::Color clkColor, sf::Color disColor)
{
	txt_def_color = defColor;
	txt_hov_color = hovColor;
	txt_clk_color = clkColor;
	txt_dis_color = disColor;

	if (interactable)
	{
		switch (btnState)
		{
		case ButtonState::DEFAULT:
			btnText.SetColor(txt_def_color);
			break;
		case ButtonState::HOVER:
			btnText.SetColor(txt_hov_color);
			break;
		case ButtonState::CLICK:
			btnText.SetColor(txt_clk_color);
			break;
		}
	}
	else
	{
		btnText.SetColor(txt_dis_color);
	}
}

ButtonState UIButton::GetButtonState()
{
	return btnState;
}

void UIButton::SetFont(sf::Font f)
{
	btnText.SetFont(f);
	btnText.SetText(text);
}

sf::RectangleShape UIButton::GetRectangle()
{
	return rectangle;
}

sf::Text UIButton::GetText()
{
	return btnText.GetText();
}
