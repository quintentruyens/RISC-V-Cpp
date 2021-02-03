#define OLC_PGE_APPLICATION
#include <iostream>
#include <string>
#include <fstream>
#include "Computer/Bus.h"
#include "Computer/RAM.h"
#include "Computer/ROM.h"
#include "Computer/Screen.h"
#include "Computer/Terminal.h"
#include "Computer/CPU/CPU.h"
#include "Computer/MemoryMap.h"
#include "Drawing/olcPixelGameEngine.h"
#include "Drawing/Button.h"
#include "Drawing/Tabs.h"

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
	Terminal<MemoryMap::TerminalAddr, 16, 40>* terminal;
	CPU* cpu;

private:
	Tabs<2>* tabs = nullptr;
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
		DrawString(x, y, "Instr: " + cpu->disassemble(bus.read(cpu->pc, true)), olc::WHITE, 2U);
		DrawString(x, y + 20, "PC: 0x" + hex(cpu->pc, 8), olc::WHITE, 2U);

		for (int i = 0; i < 32; i++)
		{
			std::string name = cpu->regName(i);
			DrawString(x + 280 * (i % 2), y + 40 + 20 * (i / 2), name + ":" + std::string(5 - name.length(), ' ') + "0x" + hex(cpu->readReg(i), 8), olc::WHITE, 2U);
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
		DrawString(1337, 380, cpsString, olc::WHITE, 3U);
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

		terminal = new Terminal<MemoryMap::TerminalAddr, 16, 40>();
		bus.connectDevice(terminal);

		cpu = new CPU([this]() mutable { running = false; });
		cpu->connectBus(&bus);

		// create interface
		std::string tabNames[] = { std::string("Memory"), std::string("Terminal")};
		tabs = new Tabs<2>(this, tabNames);
		playButton = new Button(this, 1337, 410, 50, 50, olc::DARK_CYAN, "play", 
			[this]() mutable { 
				running = !running; 
				playButton->bgColor = running ? olc::VERY_DARK_CYAN : olc::DARK_CYAN;
			});
		stepButton = new Button(this, 1392, 410, 50, 50, olc::DARK_CYAN, "step", [this]() mutable { if (!running) cpu->clock(); });
		increaseCpsButton = new Button(this, 1447, 410, 22, 22, olc::DARK_CYAN, "+", [this]() mutable { cps = cps * 2.0; });
		decreaseCpsButton = new Button(this, 1447, 438, 22, 22, olc::DARK_CYAN, "-", [this]() mutable { cps = cps / 2.0; });

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		tabs->Update();
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

		tabs->Draw();
		if (tabs->selectedTabNumber == 0)
		{
			DrawMemory(4, 30, MemoryMap::Text.BaseAddr, 16, 8);
			DrawMemory(4, 200, MemoryMap::Data.BaseAddr, 16, 8);
		}
		else
		{
			terminal->Draw(4, 30, 3, this);
		}
		
		DrawCpu(980, 4);
		screen->Draw(980, 380, 11, this);
		DrawButtons();

		return true;
	}
};

#if 1
int main()
{
	Visualiser visualiser;
	if (visualiser.Construct(1530, 750, 1, 1))
		visualiser.Start();

	return 0;
}
#endif

