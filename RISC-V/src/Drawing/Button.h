#pragma once
#include <functional>
#include "olcConsoleGameEngine.h"

class Button
{
public:
	Button(olcConsoleGameEngine* cge, int x, int y, int w, int h, short colour, std::wstring text, const std::function<void()>& onClick);
	~Button();

public:
	void Draw();
	void Update();

public:
	olcConsoleGameEngine* cge;
	int x;
	int y;
	int w;
	int h;
	short colour;
	std::wstring text;
	std::function<void()> onClick;
};

