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
	MemAccessResult write(uint32_t addr, uint32_t data, DataSize dataSize = DataSize::Word) override;
	MemAccessResult read(uint32_t addr, uint32_t& result, bool bReadOnly = false, DataSize dataSize = DataSize::Word, bool isSigned = true) override;
	
private:
	void addCharacter(wchar_t character);

private:
	uint32_t addr;
	std::wstring buffer;
	bool bCaps_lock;

	// For repeating keys when held
	uint32_t repeatingKey;
	bool bRepeating;
	double timeSinceLastPress;
};
