#pragma once
#include <cstdint>
#include <array>
#include "Bus.h"
#include "../Drawing/olcConsoleGameEngine.h"

template <uint32_t START_ADDR, uint32_t ROWS, uint32_t COLUMNS>
class Screen : public BusDevice
{
public:
	// onColour as foreground, offColour as background
	Screen(short colour)
		: colour(colour)
	{
		if (START_ADDR % 4 != 0 || COLUMNS > 32 || ROWS % 2 != 0) throw "invalid template argument";

		for (uint32_t& a : memory) a = 0;
	}
	~Screen()
	{
	}

public:
	void Draw(int x, int y, olcConsoleGameEngine* cge)
	{
		for (int row = 0; row < ROWS; row += 2)
		{
			uint32_t rowData1 = memory[row];
			uint32_t rowData2 = memory[row + 1];
			for (int column = 0; column < COLUMNS; column++)
			{
				bool state1 = ((rowData1 & (1U << column)) != 0);
				bool state2 = ((rowData2 & (1U << column)) != 0);

				wchar_t character;
				if (state1)
				{
					if (state2)
						character = L'\u2588';
					else
						character = L'\u2580';
				}
				else
				{
					if (state2)
						character = L'\u2584';
					else
						character = L' ';
				}
				
				cge->Draw(x + (COLUMNS - column) - 1, y + row / 2, character, colour); 
			}
		}
	}

public:
	MemAccessResult write(uint32_t addr, uint32_t data, DataSize dataSize = Word) override
	{
		// This only handles words, other data sizes are considered misaligned
		if (addr < START_ADDR ||  START_ADDR + ROWS * 4 <= addr) 
			return NotInRange;

		if (dataSize != Word || addr % 4 != 0) 
			return Misaligned;

		uint32_t memoryAddr = (addr - START_ADDR) / 4;
		memory[memoryAddr] = data;
		return Success;
	}

	MemAccessResult read(uint32_t addr, uint32_t& result, bool bReadOnly = false, DataSize dataSize = Word, bool isSigned = true) override
	{
		return NotInRange;
	}

public:
	std::array<uint32_t, ROWS> memory;
	short colour;
};



