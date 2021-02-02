#include "Button.h"

Button::Button(olc::PixelGameEngine* pge, int x, int y, int w, int h, olc::Pixel bgColor, std::string text, const std::function<void()>& onClick)
	: pge(pge), x(x), y(y), w(w), h(h), bgColor(bgColor), text(text), onClick(onClick)
{
}

Button::~Button()
{
}

void Button::Draw()
{
	pge->FillRect(x, y, w, h, bgColor);
	
	int stringX = x + w / 2.0 - (pge->GetTextSize(text).x / 2.0);
	int stringY = y + h / 2.0 - (pge->GetTextSize(text).y / 2.0);

	pge->DrawString(stringX, stringY, text);
}

void Button::Update()
{
	if (pge->GetMouse(0).bPressed)
	{
		int mouseX = pge->GetMouseX();
		int mouseY = pge->GetMouseY();

		if (x <= mouseX && mouseX < x + w && y <= mouseY && mouseY < y + h)
		{
			onClick();
		}
	}
}
