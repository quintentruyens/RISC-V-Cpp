#pragma once
#include <cstdint>
#include "Bus.h"

class Timer
{
public:
	Timer();
	~Timer();

public:
	bool hasInterrupt();

public:
	uint32_t getTimeLow();
	uint32_t getTimeHigh();
	uint64_t getTimeFull();

	void setTimeLow(uint32_t timeL);
	void setTimeHigh(uint32_t timeH);
	void setTimeFull(uint64_t time);

public:
	uint32_t getTimeCmpLow();
	uint32_t getTimeCmpHigh();
	uint64_t getTimeCmpFull();

	void setTimeCmpLow(uint32_t timeL);
	void setTimeCmpHigh(uint32_t timeH);
	void setTimeCmpFull(uint64_t time);

private:
	uint64_t getRealTime();

private:
	uint64_t offset = 0;
	uint64_t timeCmp = 0xFFFF'FFFF'FFFF'FFFFU;
};

class TimerDevice : public BusDevice
{
public:
	TimerDevice(uint32_t addr);
	~TimerDevice();

public:
	MemAccessResult write(uint32_t addr, uint32_t data, enum DataSize dataSize = DataSize::Word) override;
	MemAccessResult read(uint32_t addr, uint32_t& result, bool bReadOnly = false, enum DataSize dataSize = DataSize::Word, bool isSigned = true) override;

public:
	Timer timer;
	uint32_t address;
}; 
