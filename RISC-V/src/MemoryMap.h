#pragma once
#include <cstdint>

namespace MemoryMap
{
	struct Range
	{
		const uint32_t BaseAddr;
		const uint32_t LimitAddr;
	};

	constexpr Range Text = { 0x0040'0000, 0x00FF'FFFFU };
	constexpr Range Data = { 0x0101'0000, 0x0103'FFFFU };

	constexpr uint32_t HeapBaseAddr = 0x0104'0000U;
	constexpr uint32_t StackBaseAddr = 0x03FF'FFFC;

	constexpr uint32_t ScreenBaseAddr = 0xF000'0000;
}