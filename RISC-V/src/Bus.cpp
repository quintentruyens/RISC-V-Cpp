#include <cstdint>
#include "Bus.h"
#include "CPU.h"

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

void Bus::write(uint32_t addr, uint32_t data, enum DataSize dataSize)
{
	for (BusDevice* device : devices)
		device->write(addr, data, dataSize);
}

uint32_t Bus::read(uint32_t addr, bool bReadOnly, enum DataSize dataSize, bool isSigned)
{
	for (BusDevice* device : devices)
		if (device->hasAddress(addr))
			return device->read(addr, bReadOnly, dataSize, isSigned);

	return 0;
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

bool BusDevice::hasAddress(uint32_t addr)
{
	return startAddress <= addr && addr <= endAddress;
}
