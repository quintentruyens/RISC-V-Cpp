#pragma once
#include <array>
#include "Button.h"

template <size_t NUMBER>
class Tabs
{
public:
	Tabs(olc::PixelGameEngine* pge, std::string names[NUMBER])
	{
		int x = 5;
		for (size_t i = 0; i < NUMBER; i++)
		{
			int width = pge->GetTextSize(names[i]).x + 10;
			buttons[i] = new Button(pge, x, 5, width, 20, olc::VERY_DARK_CYAN, names[i], 
				[this, i]() mutable {
					buttons[selectedTabNumber]->bgColor = olc::VERY_DARK_CYAN;
					selectedTabNumber = i;
					buttons[i]->bgColor = olc::DARK_CYAN;
				}
			);
			x += width + 2;
		}
		buttons[0]->bgColor = olc::DARK_CYAN;
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

