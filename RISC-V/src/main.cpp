#define OLC_PGE_APPLICATION
#include <iostream>
#include <string>
#include <fstream>
#include "Bus.h"
#include "RAM.h"
#include "ROM.h"
#include "CPU.h"
#include "MemoryMap.h"
#include "olcPixelGameEngine.h"

class Visualiser : public olc::PixelGameEngine
{
public:
	Visualiser()
	{
		sAppName = "RISC-V";
	}

public:
	Bus bus;
	RAM<MemoryMap::Data.BaseAddr, MemoryMap::Data.LimitAddr>* ram;
	ROM<MemoryMap::Text.BaseAddr, MemoryMap::Text.LimitAddr>* rom;
	CPU* cpu;

	std::string hex(uint32_t n, uint8_t d)
	{
		std::string s(d, '0');
		for (int i = d - 1; i >= 0; i--, n >>= 4)
			s[i] = "0123456789ABCDEF"[n & 0xF];
		return s;
	};

	void DrawMemory(int x, int y, uint32_t addr, int rows, int columns)
	{
		int ramX = x;
		int ramY = y;
		for (int row = 0; row < rows; row++)
		{
			std::string sOffset = "$" + hex(addr, 8) + ":";
			for (int col = 0; col < columns; col++)
			{
				sOffset += " " + hex(bus.read(addr, true), 8);
				addr += 4;
			}
			DrawString(ramX, ramY, sOffset);
			ramY += 10;
		}
	}

	void DrawCpu(int x, int y)
	{
		DrawString(x, y, "Instr: " + cpu->disassemble(bus.read(cpu->pc, true)));
		DrawString(x, y + 10, "PC: 0x" + hex(cpu->pc, 8));

		for (int i = 0; i < 32; i++)
		{
			DrawString(x, y + 20 + 10 * i, cpu->regName(i) + ": 0x" + hex(cpu->readReg(i), 8));
		}
	}

public:
	bool OnUserCreate() override
	{
		ram = new RAM<MemoryMap::Data.BaseAddr, MemoryMap::Data.LimitAddr>();
		ram->fillFromFile("data.bin", MemoryMap::Data.BaseAddr);

		bus.connectDevice(ram);

		rom = new ROM<MemoryMap::Text.BaseAddr, MemoryMap::Text.LimitAddr>();
		rom->fillFromFile("text.bin", MemoryMap::Text.BaseAddr);
		bus.connectDevice(rom);

		cpu = new CPU();
		cpu->connectBus(&bus);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (GetKey(olc::Key::SPACE).bPressed)
		{
			cpu->clock();
		}

		Clear(olc::DARK_BLUE);

		DrawMemory(2, 2, MemoryMap::Text.BaseAddr, 16, 8);
		DrawMemory(2, 200, MemoryMap::Data.BaseAddr, 16, 8);
		DrawCpu(670, 2);

		return true;
	}
};

#if 1
int main()
{
	Visualiser visualiser;
	if (visualiser.Construct(1000, 450, 2, 2))
		visualiser.Start();

	return 0;
}
#endif

