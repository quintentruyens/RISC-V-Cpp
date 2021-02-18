#pragma once
#include <cstdint>
#include <vector>

enum DataSize {
	Word = 0, HalfWord, Byte
};

enum MemAccessResult {
	Success = 0, NotInRange, Misaligned
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
	MemAccessResult write(uint32_t addr, uint32_t data, enum DataSize dataSize = Word);
	MemAccessResult read(uint32_t add, uint32_t& result, bool bReadOnly = false, enum DataSize dataSize = Word, bool isSigned = true);

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
	virtual MemAccessResult write(uint32_t addr, uint32_t data, DataSize dataSize = Word) = 0;
	// Should not write result unless the read succeeded
	virtual MemAccessResult read(uint32_t addr, uint32_t& result, bool bReadOnly = false, DataSize dataSize = Word, bool isSigned = true) = 0;

	void connect(Bus* bus);

public:
	Bus* bus;
};

