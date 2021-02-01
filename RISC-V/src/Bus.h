#pragma once
#include <cstdint>
#include <vector>

enum DataSize {
	Word = 0, HalfWord, Byte
};

class CPU;
class BusDevice;

class Bus
{
public:
	Bus();
	~Bus();

public:
	void connectDevice(BusDevice* device);
	void connectCPU(CPU* cpu);

public:
	void write(uint32_t addr, uint32_t data, DataSize dataSize = Word);
	uint32_t read(uint32_t add, bool bReadOnly = false, DataSize dataSize = Word, bool isSigned = true);

private:
	std::vector<BusDevice*> devices;
	CPU* cpu = nullptr;
};

class BusDevice
{
public:
	BusDevice();
	virtual ~BusDevice();

public:
	virtual void write(uint32_t addr, uint32_t data, DataSize dataSize = Word) = 0;
	virtual uint32_t read(uint32_t addr, bool bReadOnly = false, DataSize dataSize = Word, bool isSigned = true) = 0;

	void connect(Bus* bus);

	bool hasAddress(uint32_t addr);

public:
	Bus* bus;

	uint32_t startAddress = 0;
	uint32_t endAddress = 0;
};

