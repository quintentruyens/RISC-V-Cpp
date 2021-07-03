#include <cstdint>
#include "Bus.h"
#include "Timer.h"
#include "MemoryMap.h"
#include "CPU/CPU.h"

// BUS
Bus::Bus(std::vector<BusDevice*> devices)
	:cpu(nullptr)
{
	TimerDevice* timerDevice = new TimerDevice(MemoryMap::TimerAddr);
	devices.push_back(timerDevice);
	this->devices = devices;
	timer = &timerDevice->timer;

	for (BusDevice* device : devices)
		device->connect(this);
}

Bus::~Bus()
{
	for (BusDevice* device : devices)
		delete device;
}

void Bus::connectCPU(CPU* cpu)
{
	this->cpu = cpu;
	cpu->connectBus(this);
}

MemAccessResult Bus::write(uint32_t addr, uint32_t data, enum DataSize dataSize)
{
	for (BusDevice* device : devices)
	{
		MemAccessResult accessResult = device->write(addr, data, dataSize);
		if (accessResult == MemAccessResult::Success || accessResult == MemAccessResult::Misaligned)
			return accessResult;
	}

	return MemAccessResult::NotInRange;
}

MemAccessResult Bus::read(uint32_t addr, uint32_t& result, bool bReadOnly, enum DataSize dataSize, bool isSigned)
{
	for (BusDevice* device : devices)
	{
		MemAccessResult accessResult = device->read(addr, result, bReadOnly, dataSize, isSigned);
		if (accessResult == MemAccessResult::Success || accessResult == MemAccessResult::Misaligned)
			return accessResult;
	}

	return MemAccessResult::NotInRange;
}

bool Bus::hasInterrupt()
{
	return activeInterrupts != 0;
}

void Bus::postInterrupt()
{
	activeInterrupts++;
}

void Bus::clearInterrupt()
{
	activeInterrupts--;
}

// BUSDEVICE
BusDevice::BusDevice()
{
	bus = nullptr;
}

BusDevice::~BusDevice()
{
}

void BusDevice::connect(Bus* bus)
{
	this->bus = bus;
}
