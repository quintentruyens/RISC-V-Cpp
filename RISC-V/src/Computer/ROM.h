#pragma once
#include <cstdint>
#include <array>
#include <fstream>
#include "Bus.h"

template <uint32_t START_ADDR, uint32_t END_ADDR>
class ROM : public BusDevice
{
public:
	ROM()
	{
		if (START_ADDR % 4 != 0 || END_ADDR % 4 != 3) throw "Invalid adress";

		startAddress = START_ADDR;
		endAddress = END_ADDR;

		for (uint32_t& a : memory) a = 0;
	}

	~ROM()
	{
	}

public:
	void fillFromFile(const char* fileName, uint32_t startAddr)
	{
		std::ifstream file;
		file.open(fileName, std::ios::in | std::ios::binary | std::ios::ate);

		std::streampos size = file.tellg();
		file.seekg(0, std::ios::beg);

		file.read((char*)&memory[(startAddr - startAddress) / 4], size);

		file.close();
	}

public:
	void write(uint32_t addr, uint32_t data, DataSize dataSize)
	{
	}

	uint32_t read(uint32_t addr, bool bReadOnly, enum DataSize dataSize, bool isSigned)
	{
		if (!hasAddress(addr)) return 0;

		uint32_t memoryAddr = (addr - startAddress) / 4;
		uint32_t offset = addr & 0b11;
		bool misalignedException = false;

		uint16_t data16;
		uint8_t data8;

		switch (dataSize)
		{
		case Word:
			misalignedException = offset == 0;
			return memory[memoryAddr];

		case HalfWord:
			switch (offset)
			{
			case 0:
				data16 = memory[memoryAddr] & 0x0000'FFFFU;

				return isSigned ? (uint32_t)(int32_t)(int16_t)data16 : (uint32_t)data16;
			case 2:
				data16 = (memory[memoryAddr] & 0xFFFF'0000U) >> 16;

				return isSigned ? (uint32_t)(int32_t)(int16_t)data16 : (uint32_t)data16;
			default:
				misalignedException = true;
				break;
			}
			break;

		case Byte:
		{
			uint32_t shiftAmount = offset * 8;
			uint32_t bitMask = 0xFFU << shiftAmount;

			data8 = (memory[memoryAddr] & bitMask) >> shiftAmount;

			return isSigned ? (uint32_t)(int32_t)(int8_t)data8 : (uint32_t)data8;
		}

		default:
			break;
		}

		return 0;
	}

public:
	std::array<uint32_t, (END_ADDR - START_ADDR) / 4> memory;
};

