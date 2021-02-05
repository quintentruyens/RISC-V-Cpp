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
	// For drawing
	std::vector<uint32_t> validAdresses;

private:
	const CPU* cpu;

private:
	uint32_t ureg00 = 0;
	std::function<void()> startDebug;
};

