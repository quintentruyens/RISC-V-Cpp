#pragma once
#include <array>
#include "Button.h"

template <size_t NUMBER>
class Tabs
{
public:
	Tabs(olcConsoleGameEngine* cge, short onColour, short offColour, std::wstring names[NUMBER])
	{
		int x = 1;
		for (size_t i = 0; i < NUMBER; i++)
		{
			int width = names[i].length() + 2;
			buttons[i] = new Button(cge, x, 0, width, 1, offColour, names[i], 
				[this, i, onColour, offColour]() mutable {
					buttons[selectedTabNumber]->colour = offColour;
					selectedTabNumber = i;
					buttons[i]->colour = onColour;
				}
			);
			x += width + 1;
		}
		buttons[0]->colour = onColour;
	}

	~Tabs()
	{
		for (Button* b : buttons)
			delete b;
	}

public:
	size_t selectedTabNumber = 0;

public:
	void Update()
	{
		for (Button* b : buttons)
			b->Update();
	}

	void Draw()
	{
		for (Button* b : buttons)
			b->Draw();
	}

private:
	std::array<Button*, NUMBER> buttons;
};

