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
		if (ROWS == 0 || COLUMNS == 0) throw "invalid template argument";

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
	MemAccessResult write(uint32_t addr, uint32_t data, DataSize dataSize = Word) override
	{
		if (addr != ADDR) return NotInRange;
		// Misaligned is no longer possible, if addr == ADDR, it is a multiple of 4 and always aligned

		wchar_t character = (wchar_t)data;
		if (character == 10 || character == 13) // Newline
		{
			newLine();
			return Success;
		}
		if (character == 12) // Clear screen
		{
			clear();
			return Success;
		}

		std::wstring& rowString = rows[currentRow];

		if (character == 8) // Backspace
		{
			if (rowString.length() > 0)
				rowString.pop_back();
			return Success;
		}
		if ((32 <= character && character < 127) || character > 160)
		{
			if (rowString.length() == COLUMNS)
				newLine();

			rows[currentRow].push_back((wchar_t)data);
			return Success;
		}
	}

	MemAccessResult read(uint32_t addr, uint32_t& result, bool bReadOnly = false, DataSize dataSize = Word, bool isSigned = true) override
	{
		return NotInRange;
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
		currentRow = 0;
	}

private:
	std::array<std::wstring, ROWS> rows;
	uint32_t currentRow = 0;
};

