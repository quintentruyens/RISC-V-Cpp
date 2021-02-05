#include "Button.h"

Button::Button(olcConsoleGameEngine* cge, int x, int y, int w, int h, short colour, std::wstring text, const std::function<void()>& onClick)
	: cge(cge), x(x), y(y), w(w), h(h), colour(colour), text(text), onClick(onClick)
{
}

Button::~Button()
{
}

void Button::Draw()
{
	cge->Fill(x, y, x + w, y + h, L' ', colour);

	int stringX = x + (w - text.length()) / 2;
	int stringY = y + h / 2;

	cge->DrawString(stringX, stringY, text, colour);
}

void Button::Update()
{
	if (cge->GetMouse(0).bPressed)
	{
		int mouseX = cge->GetMouseX();
		int mouseY = cge->GetMouseY();

		if (x <= mouseX && mouseX < x + w && y <= mouseY && mouseY < y + h)
		{
			onClick();
		}
	}
}
