#include <cstdint>
#include "Bus.h"
#include "CPU/CPU.h"

// BUS
Bus::Bus()
	:cpu(nullptr)
{
}

Bus::~Bus()
{
	for (BusDevice* device : devices)
		delete device;
}

void Bus::connectDevice(BusDevice* device)
{
	devices.push_back(device);
	device->connect(this);
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
		if (accessResult == Success || accessResult == Misaligned)
			return accessResult;
	}

	return NotInRange;
}

MemAccessResult Bus::read(uint32_t addr, uint32_t& result, bool bReadOnly, enum DataSize dataSize, bool isSigned)
{
	for (BusDevice* device : devices)
	{
		MemAccessResult accessResult = device->read(addr, result, bReadOnly, dataSize, isSigned);
		if (accessResult == Success || accessResult == Misaligned)
			return accessResult;
	}

	return NotInRange;
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
