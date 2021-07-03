#pragma once
#include <cstdint>
#include <vector>

enum class DataSize {
	Word = 0, HalfWord, Byte
};

enum class MemAccessResult {
	Success = 0, NotInRange, Misaligned
};

class CPU;
class Timer;
class BusDevice;

class Bus
{
public:
	Bus(std::vector<BusDevice*> devices);
	~Bus();

public:
	void connectCPU(CPU* cpu);

public:
	MemAccessResult write(uint32_t addr, uint32_t data, enum DataSize dataSize = DataSize::Word);
	MemAccessResult read(uint32_t add, uint32_t& result, bool bReadOnly = false, enum DataSize dataSize = DataSize::Word, bool isSigned = true);

public:
	Timer* timer = nullptr;

public:
	bool hasInterrupt();
	void postInterrupt();
	void clearInterrupt();

private:
	std::vector<BusDevice*> devices;
	CPU* cpu = nullptr;
	uint32_t activeInterrupts = 0;
};

class BusDevice
{
public:
	BusDevice();
	virtual ~BusDevice();

public:
	virtual MemAccessResult write(uint32_t addr, uint32_t data, DataSize dataSize = DataSize::Word) = 0;
	// Should not write result unless the read succeeded
	virtual MemAccessResult read(uint32_t addr, uint32_t& result, bool bReadOnly = false, DataSize dataSize = DataSize::Word, bool isSigned = true) = 0;

	void connect(Bus* bus);

public:
	Bus* bus;
};

