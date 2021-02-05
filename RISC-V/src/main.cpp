﻿#define OLC_CGE_APPLICATION
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include "Computer/Bus.h"
#include "Computer/RAM.h"
#include "Computer/ROM.h"
#include "Computer/Screen.h"
#include "Computer/Terminal.h"
#include "Computer/CPU/CPU.h"
#include "Computer/MemoryMap.h"
#include "Drawing/Button.h"
#include "Drawing/Tabs.h"

class Visualiser : public olcConsoleGameEngine
{
public:
	Visualiser()
	{
		m_sAppName = L"RISC-V";
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
	std::wstring hex(uint32_t n, uint8_t d)
	{
		std::wstring s(d, L'0');
		for (int i = d - 1; i >= 0; i--, n >>= 4)
			s[i] = L"0123456789ABCDEF"[n & 0xF];
		return s;
	};

	void DrawMemory(int x, int y, uint32_t addr, int rows, int columns)
	{
		for (int row = 0; row < rows; row++)
		{
			std::wstring sOffset = L"$" + hex(addr, 8) + L":";
			for (int col = 0; col < columns; col++)
			{
				sOffset += L" " + hex(bus.read(addr, true), 8);
				addr += 4;
			}
			DrawString(x, y + row, sOffset, FG_WHITE | BG_DARK_BLUE);
		}
	}

	void DrawCpu(int x, int y)
	{
		DrawString(x, y, L"Instr: " + cpu->disassemble(bus.read(cpu->pc, true)), FG_WHITE | BG_DARK_BLUE);
		DrawString(x, y + 1, L"PC: 0x" + hex(cpu->pc, 8), FG_WHITE | BG_DARK_BLUE);

		for (int i = 0; i < 32; i++)
		{
			std::wstring name = cpu->regName(i);
			DrawString(x + 17 * (i % 2), y + 2 + (i / 2), name + L":" + std::wstring(4 - name.length(), ' ') + L"0x" + hex(cpu->readReg(i), 8), FG_WHITE | BG_DARK_BLUE);
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
		std::wostringstream cpsStream;
		cpsStream << std::setprecision(5) << std::noshowpoint << cps;
		std::wstring cpsString = cpsStream.str();
		DrawString(83, 1, cpsString, FG_WHITE | BG_DARK_BLUE);
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

		screen = new Screen<MemoryMap::ScreenBaseAddr, 32, 32>(FG_GREEN | BG_DARK_GREY);
		bus.connectDevice(screen);

		terminal = new Terminal<MemoryMap::TerminalAddr, 16, 40>();
		bus.connectDevice(terminal);

		cpu = new CPU([this]() mutable { running = false; playButton->colour = FG_WHITE | BG_CYAN; });
		cpu->connectBus(&bus);

		// create interface
		std::wstring tabNames[] = { std::wstring(L"Memory"), std::wstring(L"Terminal")};
		tabs = new Tabs<2>(this, FG_WHITE | BG_CYAN, FG_WHITE | BG_DARK_CYAN, tabNames);
		playButton = new Button(this, 83, 3, 6, 3, FG_WHITE | BG_CYAN, L"play",
			[this]() mutable { 
				running = !running; 
				playButton->colour = running ? FG_WHITE | BG_DARK_CYAN : FG_WHITE | BG_CYAN;
			});
		stepButton = new Button(this, 90, 3, 6, 3, FG_WHITE | BG_CYAN, L"step", [this]() mutable { if (!running) cpu->clock(); });
		increaseCpsButton = new Button(this, 83, 7, 6, 3, FG_WHITE | BG_CYAN, L"++", [this]() mutable { cps = cps * 2.0; });
		decreaseCpsButton = new Button(this, 90, 7, 6, 3, FG_WHITE | BG_CYAN, L"--", [this]() mutable { cps = cps / 2.0; });

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
		
		Fill(0, 0, m_nScreenWidth, m_nScreenHeight, ' ', BG_DARK_BLUE);

		tabs->Draw();
		if (tabs->selectedTabNumber == 0)
		{
			DrawMemory(1, 2, MemoryMap::Text.BaseAddr, 16, 4);
			DrawMemory(1, 20, MemoryMap::Data.BaseAddr, 16, 4);
		}
		else
		{
			terminal->Draw(1, 2, FG_BLACK | BG_GREY, this);
		}
		
		DrawCpu(50, 1);
		screen->Draw(50, 20, this);
		DrawButtons();

		return true;
	}
};

int main()
{
	Visualiser visualiser;
	if (visualiser.ConstructConsole(97, 37, 10, 20) == 1)
		visualiser.Start();

	return 0;
}

