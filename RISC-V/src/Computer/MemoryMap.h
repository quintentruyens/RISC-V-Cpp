#pragma once
#include <cstdint>

namespace MemoryMap
{
	struct Range
	{
		const uint32_t BaseAddr;
		const uint32_t LimitAddr;
	};

	constexpr Range RAM  = { 0x0000'0000, 0x03FF'FFFFU };

	constexpr Range Text = { 0x0040'0000, 0x00FF'FFFFU };
	constexpr Range Data = { 0x0101'0000, 0x0103'FFFFU };

	constexpr uint32_t HeapBaseAddr = 0x0104'0000U;
	constexpr uint32_t StackBaseAddr = 0x03FF'FFFCU;

	constexpr uint32_t ScreenBaseAddr = 0xF000'0000U;
	constexpr uint32_t TerminalAddr = 0xF000'0080U;
	constexpr uint32_t KeyboardAddr = 0xF000'0080U;

	constexpr uint32_t TimerAddr = 0xF000'0090U;
}