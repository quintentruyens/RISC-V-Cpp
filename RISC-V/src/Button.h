#pragma once
#include <functional>
#include "olcPixelGameEngine.h"

class Button
{
public:
	Button(olc::PixelGameEngine* pge, int x, int y, int w, int h, olc::Pixel bgColor, std::string text, const std::function<void()>& onClick);
	~Button();

public:
	void Draw();
	void Update();

private:
	olc::PixelGameEngine* pge;
	int x;
	int y;
	int w;
	int h;
	olc::Pixel bgColor;
	std::string text;
	std::function<void()> onClick;
};

