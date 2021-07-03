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
	void reset(uint32_t cause);

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
	// Checks for interrupts. If there are any, executes them and returns true and the new pc, otherwise, returns false
	// epc is the epc that will be used if there are any interrupts
	struct CheckInterruptsReturn {
		bool hasInterrupt; uint32_t newPc;
	} checkInterrupts(uint32_t epc);

public:
	// For drawing
	std::vector<uint32_t> validAdresses;

private:
	const CPU* cpu;

private:
	struct MStatus {
		unsigned UIE : 1;
		unsigned SIE : 1;
		unsigned UNUSED0 : 1;
		unsigned MIE : 1;
		unsigned UPIE : 1;
		unsigned SPIE : 1;
		unsigned UNUSED1 : 1;
		unsigned MPIE : 1;
		unsigned SPP : 1;
		unsigned UNUSED2 : 2;
		unsigned MPP : 2;
		unsigned FS : 2;
		unsigned XS : 2;
		unsigned MPRV : 1;
		unsigned SUM : 1;
		unsigned MXR : 1;
		unsigned TVM : 1;
		unsigned TW : 1;
		unsigned TSR : 1;
		unsigned UNUSED3 : 8;
		unsigned SD : 1;
	};
	MStatus mstatus = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	uint32_t mtvec = 0;

	uint32_t mscratch = 0;
	uint32_t mepc = 0;
	uint32_t mcause = 0;
	uint32_t mtval = 0;

	union MInterruptCSR
	{
		uint32_t word;
		struct {
			unsigned USI : 1;
			unsigned SSI : 1;
			unsigned UNUSED1 : 1;
			unsigned MSI : 1;
			unsigned UTI : 1;
			unsigned STI : 1;
			unsigned UNUSED2 : 1;
			unsigned MTI : 1;
			unsigned UEI : 1;
			unsigned SEI : 1;
			unsigned UNUSED3 : 1;
			unsigned MEI : 1;
			unsigned UNUSED4 : 20;
		} bits;
	};
	
	MInterruptCSR mipInternal = { 0 };
	MInterruptCSR mie = { 0 };

	void updateMip();

	uint32_t ureg00 = 0;
	std::function<void()> startDebug;
};

