#pragma once
#include <string>
#include <array>
#include "Bus.h"
#include "../Drawing/olcConsoleGameEngine.h"

struct sKeyState
{
	bool bPressed;
	bool bReleased;
	bool bHeld;
};

struct sKeyMapping // The characters that a certain key should print when pressed
{
	wchar_t regular;
	wchar_t shift;
	wchar_t alt_gr;
};

class Keyboard : public BusDevice
{
public:
	Keyboard(uint32_t addr);
	~Keyboard();

public:
	void Update(olcConsoleGameEngine* cge, float fElapsedTime);

public:
	void Keyboard::write(uint32_t addr, uint32_t data, DataSize dataSize) override;
	uint32_t Keyboard::read(uint32_t addr, bool bReadOnly, DataSize dataSize, bool isSigned) override;

private:
	uint32_t addr;
	std::wstring buffer;
	bool bCaps_lock;

	// For repeating keys when held
	uint32_t repeatingKey;
	bool bRepeating;
	double timeSinceLastPress;
};
