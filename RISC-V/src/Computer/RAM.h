#pragma once
#include <cstdint>
#include <array>
#include "Bus.h"

template <uint32_t START_ADDR, uint32_t END_ADDR>
class RAM : public BusDevice
{
public:
	RAM()
	{
		if (START_ADDR % 4 != 0 || END_ADDR % 4 != 3) throw "Invalid adress";

		for (uint32_t& a : memory) a = 0;
	}

	~RAM()
	{
	}

public:
	void fillFromFile(const char* fileName, uint32_t startAddr)
	{
		std::ifstream file;
		file.open(fileName, std::ios::in | std::ios::binary | std::ios::ate);

		std::streampos size = file.tellg();
		file.seekg(0, std::ios::beg);

		file.read((char*)&memory[(startAddr - START_ADDR) / 4], size);

		file.close();
	}

public:
	MemAccessResult write(uint32_t addr, uint32_t data, DataSize dataSize = Word) override
	{
		if (addr < START_ADDR || END_ADDR < addr) return NotInRange;

		uint32_t memoryAddr = (addr - START_ADDR) / 4;
		uint32_t offset = addr & 0b11;

		switch (dataSize)
		{
		case Word:
			if (offset != 0)
				return Misaligned;
			memory[memoryAddr] = data;
			return Success;

		case HalfWord:
			switch (offset)
			{
			case 0:
				memory[memoryAddr] = (memory[memoryAddr] & 0xFFFF'0000U) | (data & 0x0000'FFFFU);
				return Success;
			case 2:
				memory[memoryAddr] = (memory[memoryAddr] & 0x0000'FFFFU) | ((data << 16) & 0xFFFF'0000U);
				return Success;
			default:
				return Misaligned;
			}

		case Byte:
		{
			uint32_t bitMask = 0xFFU;
			uint32_t shiftAmount = offset * 8;

			memory[memoryAddr] = (memory[memoryAddr] & ~(bitMask << shiftAmount)) | ((data & bitMask) << shiftAmount);
			return Success;
		}

		default: // Shouldn't happen
			return Misaligned;
		}
	}

	MemAccessResult read(uint32_t addr, uint32_t& result, bool bReadOnly = false, DataSize dataSize = Word, bool isSigned = true) override
	{
		if (addr < START_ADDR || END_ADDR < addr) return NotInRange;

		uint32_t memoryAddr = (addr - START_ADDR) / 4;
		uint32_t offset = addr & 0b11;

		uint16_t data16;
		uint8_t data8;

		switch (dataSize)
		{
		case Word:
			if (offset != 0)
				return Misaligned;
			result = memory[memoryAddr];
			return Success;

		case HalfWord:
			switch (offset)
			{
			case 0:
				data16 = memory[memoryAddr] & 0x0000'FFFFU;
				result = isSigned ? (uint32_t)(int32_t)(int16_t)data16 : (uint32_t)data16;
				return Success;
			case 2:
				data16 = (memory[memoryAddr] & 0xFFFF'0000U) >> 16;
				result = isSigned ? (uint32_t)(int32_t)(int16_t)data16 : (uint32_t)data16;
				return Success;
			default:
				return Misaligned;
			}

		case Byte:
		{
			uint32_t shiftAmount = offset * 8;
			uint32_t bitMask = 0xFFU << shiftAmount;

			data8 = (memory[memoryAddr] & bitMask) >> shiftAmount;

			result = isSigned ? (uint32_t)(int32_t)(int8_t)data8 : (uint32_t)data8;
			return Success;
		}
		
		default: // Shouldn't happen
			return Misaligned;
		}
	}

public:
	std::array<uint32_t, (END_ADDR - START_ADDR) / 4> memory;
};

