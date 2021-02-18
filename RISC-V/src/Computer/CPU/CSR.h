#pragma once
#include <cstdint>
#include <string>
#include <functional>

class CPU;

class CSR
{
public:
	CSR(CPU* cpu, const std::function<void()>& startDebug);
	~CSR();

public:
	bool read(uint32_t address, uint32_t& value, bool bReadOnly); // return true if successful, false if exception
	bool write(uint32_t address, uint32_t value); // return true if successful, false if exception

	std::wstring getName(uint32_t address);

public:
	// sets all CSR's to appropriate values for the given exception and returns the new PC
	// causeNum does not include the top interrupt bit, this is given separately as bInterrupt
	uint32_t executeException(uint32_t epc, uint32_t causeNum, uint32_t val, bool bInterrupt);
	// sets all CSR's appropriate values for returning from an exception and returns the new PC
	uint32_t returnExcepion();

public:
	// For drawing
	std::vector<uint32_t> validAdresses;

private:
	const CPU* cpu;

private:
	uint32_t mtvec = 0;

	uint32_t mscratch = 0;
	uint32_t mepc = 0;
	uint32_t mcause = 0;
	uint32_t mtval = 0;

	uint32_t ureg00 = 0;
	std::function<void()> startDebug;
};

