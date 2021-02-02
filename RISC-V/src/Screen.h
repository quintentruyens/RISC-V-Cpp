#pragma once
#include <cstdint>
#include <array>
#include "Bus.h"
#include "olcPixelGameEngine.h"

template <uint32_t START_ADDR, uint32_t ROWS, uint32_t COLUMNS>
class Screen : public BusDevice
{
public:
	Screen()
	{
		if (START_ADDR % 4 != 0 || COLUMNS > 32) throw "invalid template argument";

		startAddress = START_ADDR;
		endAddress = START_ADDR + ROWS * 4;

		for (uint32_t& a : memory) a = 0;
	}
	~Screen()
	{
	}

public:
	void Draw(int x, int y, int pixelSize, olc::PixelGameEngine* pge)
	{
		pge->FillRect(x, y, pixelSize * COLUMNS, pixelSize * ROWS, olc::DARK_GREY); // draw background

		for (int row = 0; row < ROWS; row++)
		{
			uint32_t rowData = memory[row];
			for (int column = 0; column < COLUMNS; column++)
			{
				if ((rowData & (1U << column)) != 0)
				{
					pge->FillRect(x + pixelSize * (COLUMNS - column - 1), y + pixelSize * row, pixelSize, pixelSize, olc::GREEN);
				}
			}
		}
	}

public:
	void write(uint32_t addr, uint32_t data, DataSize dataSize)
	{
		// This ignores dataSize and doesn't cause address misaligned exceptions
		if (!hasAddress(addr)) return;

		uint32_t memoryAddr = (addr - startAddress) / 4;
		memory[memoryAddr] = data;
	}

	uint32_t read(uint32_t addr, bool bReadOnly, DataSize dataSize, bool isSigned)
	{
		return 0;
	}

public:
	std::array<uint32_t, ROWS> memory;
};



