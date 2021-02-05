#pragma once
#include <cstdint>
#include <array>
#include "Bus.h"
#include "../Drawing/olcConsoleGameEngine.h"

template <uint32_t ADDR, size_t ROWS, size_t COLUMNS>
class Terminal : public BusDevice
{
public:
	Terminal()
	{
		if (ADDR % 4 != 0 || ROWS == 0 || COLUMNS == 0) throw "invalid template argument";

		startAddress = ADDR;
		endAddress = ADDR + 3;

		for (size_t i = 0; i < ROWS; i++)
			rows[i].reserve(COLUMNS);
	}

	~Terminal()
	{
	}

public:
	void Draw(int x, int y, short colour, olcConsoleGameEngine* cge)
	{
		cge->Fill(x, y, x + COLUMNS, y + ROWS, L' ', colour);
		for (int row = 0; row < ROWS; row++)
		{
			cge->DrawString(x, y + row, rows[row], colour);
		}
	}

public:
	void write(uint32_t addr, uint32_t data, DataSize dataSize)
	{
		// This ignores dataSize and doesn't cause address misaligned exceptions
		if (addr != ADDR) return;

		wchar_t character = (wchar_t)data;
		if (character == 10 || character == 13) // Newline
		{
			newLine();
			return;
		}
		if (character == 12) // Clear screen
		{
			clear();
			return;
		}

		std::wstring& rowString = rows[currentRow];

		if (character == 8) // Backspace
		{
			rowString.pop_back();
			return;
		}
		if ((32 <= character && character < 127) || character > 160)
		{
			rowString.push_back((wchar_t)data);

			if (rowString.length() == COLUMNS)
				newLine();
		}
	}

	uint32_t read(uint32_t addr, bool bReadOnly, DataSize dataSize, bool isSigned)
	{
		return 0;
	}

private:
	void newLine()
	{
		if (currentRow == ROWS - 1) {
			// Move upwards
			for (int i = 0; i < ROWS - 1; i++)
				rows[i] = rows[i + 1];
			rows[currentRow].clear();
		}
		else
		{
			currentRow++;
		}
	}

	void clear()
	{
		for (std::wstring& row : rows)
			row.clear();
	}

private:
	std::array<std::wstring, ROWS> rows;
	uint32_t currentRow = 0;
};

