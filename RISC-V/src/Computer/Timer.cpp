#include <chrono>
#include "Timer.h"

// TIMER CLASS
Timer::Timer()
{
}

Timer::~Timer()
{
}

bool Timer::hasInterrupt()
{
	return getTimeFull() >= timeCmp;
}

uint32_t Timer::getTimeLow()
{
	return getTimeFull() & 0xFFFF'FFFFU;
}

uint32_t Timer::getTimeHigh()
{
	return getTimeFull() >> 32;
}

uint64_t Timer::getTimeFull()
{
	return getRealTime() - offset;
}

void Timer::setTimeLow(uint32_t timeL)
{
	setTimeFull((getTimeFull() & 0xFFFF'FFFF'0000'0000U) | timeL);
}

void Timer::setTimeHigh(uint32_t timeH)
{
	setTimeFull((getTimeFull() & 0x0000'0000'FFFF'FFFFU) | ((uint64_t)timeH << 32));
}

void Timer::setTimeFull(uint64_t time)
{
	offset = getRealTime() - time;
}

uint32_t Timer::getTimeCmpLow()
{
	return timeCmp & 0xFFFF'FFFFU;
}

uint32_t Timer::getTimeCmpHigh()
{
	return timeCmp >> 32;
}

uint64_t Timer::getTimeCmpFull()
{
	return timeCmp;
}

void Timer::setTimeCmpLow(uint32_t timeL)
{
	timeCmp = (timeCmp & 0xFFFF'FFFF'0000'0000U) | timeL;
}

void Timer::setTimeCmpHigh(uint32_t timeH)
{
	timeCmp = (timeCmp & 0x0000'0000'FFFF'FFFFU) | ((uint64_t)timeH << 32);
}

void Timer::setTimeCmpFull(uint64_t time)
{
	timeCmp = time;
}

uint64_t Timer::getRealTime()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();
}

// TIMERDEVICE CLASS
TimerDevice::TimerDevice(uint32_t addr)
	: address(addr)
{
}

TimerDevice::~TimerDevice()
{
}

MemAccessResult TimerDevice::write(uint32_t addr, uint32_t data, enum DataSize dataSize)
{
	if (addr < address || (address + 16) <= addr)
		return MemAccessResult::NotInRange;

	if (dataSize != DataSize::Word || addr % 4 != 0)
		return MemAccessResult::Misaligned;

	if (addr == address)
		timer.setTimeLow(data);
	else if (addr == address + 4)
		timer.setTimeHigh(data);
	else if (addr == address + 8)
		timer.setTimeCmpLow(data);
	else
		timer.setTimeCmpHigh(data);

	return MemAccessResult::Success;
}

MemAccessResult TimerDevice::read(uint32_t addr, uint32_t& result, bool bReadOnly, enum DataSize dataSize, bool isSigned)
{
	if (addr < address || (address + 16) <= addr)
		return MemAccessResult::NotInRange;

	if (dataSize != DataSize::Word || addr % 4 != 0)
		return MemAccessResult::Misaligned;

	if (addr == address)
		result = timer.getTimeLow();
	else if (addr == address + 4)
		result = timer.getTimeHigh();
	else if (addr == address + 8)
		result = timer.getTimeCmpLow();
	else
		result = timer.getTimeCmpHigh();

	return MemAccessResult::Success;
}
