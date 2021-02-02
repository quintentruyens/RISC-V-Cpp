#define OLC_PGE_APPLICATION
#include <iostream>
#include <string>
#include <fstream>
#include "Bus.h"
#include "RAM.h"
#include "ROM.h"
#include "Screen.h"
#include "CPU.h"
#include "MemoryMap.h"
#include "olcPixelGameEngine.h"
#include "Button.h"

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
	Screen<MemoryMap::ScreenBaseAddr, 32, 32>* screen;
	CPU* cpu;

private:
	Button* playButton = nullptr;
	Button* stepButton = nullptr;
	Button* increaseCpsButton = nullptr;
	Button* decreaseCpsButton = nullptr;

private:
	bool running = false;
	double cps = 1.0; // clocks per second
	double timeSinceLastCycle = 0.0;

public:
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
			DrawString(x + 140 * (i % 2), y + 20 + 10 * (i / 2), cpu->regName(i) + ": 0x" + hex(cpu->readReg(i), 8));
		}
	}
	
	void UpdateButtons()
	{
		playButton->Update();
		stepButton->Update();
		increaseCpsButton->Update();
		decreaseCpsButton->Update();
	}

	void DrawButtons()
	{
		playButton->Draw();
		stepButton->Draw();
		// convert cps to string
		std::ostringstream cpsStream;
		cpsStream << std::setprecision(5) << std::noshowpoint << cps;
		std::string cpsString = cpsStream.str();
		DrawString(900, 310, cpsString, olc::WHITE, 2U);
		increaseCpsButton->Draw();
		decreaseCpsButton->Draw();
	}

public:
	bool OnUserCreate() override
	{
		// Build computer
		ram = new RAM<MemoryMap::Data.BaseAddr, MemoryMap::Data.LimitAddr>();
		ram->fillFromFile("data.bin", MemoryMap::Data.BaseAddr);
		bus.connectDevice(ram);

		rom = new ROM<MemoryMap::Text.BaseAddr, MemoryMap::Text.LimitAddr>();
		rom->fillFromFile("text.bin", MemoryMap::Text.BaseAddr);
		bus.connectDevice(rom);

		screen = new Screen<MemoryMap::ScreenBaseAddr, 32, 32>();
		bus.connectDevice(screen);

		cpu = new CPU([this]() mutable { running = false; });
		cpu->connectBus(&bus);

		// create interface
		playButton = new Button(this, 900, 200, 50, 50, olc::DARK_CYAN, "play", [this]() mutable { running = !running; });
		stepButton = new Button(this, 900, 255, 50, 50, olc::DARK_CYAN, "step", [this]() mutable { if (!running) cpu->clock(); });
		increaseCpsButton = new Button(this, 900, 330, 15, 15, olc::DARK_CYAN, "+", [this]() mutable { cps = cps * 2.0; });
		decreaseCpsButton = new Button(this, 920, 330, 15, 15, olc::DARK_CYAN, "-", [this]() mutable { cps = cps / 2.0; });

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		UpdateButtons();

		if (running)
		{
			timeSinceLastCycle += fElapsedTime;

			while (timeSinceLastCycle > 1 / cps)
			{
				cpu->clock();
				timeSinceLastCycle -= 1 / cps;
			}
		}
		
		Clear(olc::DARK_BLUE);

		DrawMemory(4, 30, MemoryMap::Text.BaseAddr, 16, 8);
		DrawMemory(4, 200, MemoryMap::Data.BaseAddr, 16, 8);
		DrawCpu(670, 4);
		screen->Draw(670, 200, 7, this);
		DrawButtons();

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

