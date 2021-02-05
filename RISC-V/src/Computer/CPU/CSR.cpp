#include <cstdint>
#include <chrono>
#include "CSR.h"
#include "CPU.h"

constexpr uint32_t MISA = 0x301;

constexpr uint32_t UREG00 = 0x800;

constexpr uint32_t CYCLE = 0xC00;
constexpr uint32_t TIME = 0xC01;
constexpr uint32_t INSTRET = 0xC02;

constexpr uint32_t CYCLEH = 0xC80;
constexpr uint32_t TIMEH = 0xC81;
constexpr uint32_t INSTRETH = 0xC82;

constexpr uint32_t MVENDORID = 0xF11;
constexpr uint32_t MARCHID = 0xF12;
constexpr uint32_t MIMPID = 0xF13;
constexpr uint32_t MHARTID = 0xF14;

constexpr uint32_t DEBUG = 0xFC0;

CSR::CSR(CPU* cpu, const std::function<void()>& startDebug)
	: cpu(cpu), startDebug(startDebug)
{
}

CSR::~CSR()
{
}

bool CSR::read(uint32_t address, uint32_t& value)
{
	switch (address)
	{
	case MISA:
		value = 0b01'0000'00000000100010000000000000U;
		return true;

	case UREG00:
		value = ureg00;
		return true;

	case CYCLE:
		value = this->cpu->cycle & 0xFFFF'FFFFU;
		return true;
	case TIME:
	{
		// get milliseconds since 1970
		uint64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		value = time & 0xFFFF'FFFFU;
		return true;
	}
	case INSTRET:
		value = this->cpu->instret & 0xFFFF'FFFFU;
		return true;

	case CYCLEH:
		value = this->cpu->cycle >> 32;
		return true;
	case TIMEH:
	{
		// get milliseconds since 1970
		uint64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		value = time >> 32;
		return true;
	}
	case INSTRETH:
		value = this->cpu->instret >> 32;
		return true;

	case MVENDORID:
	case MARCHID:
	case MIMPID:
	case MHARTID:
		value = 0;
		return true;

	case DEBUG:
		value = 0;
		startDebug();
		return true;

	default:
		return false;
	}
}

bool CSR::write(uint32_t address, uint32_t value)
{
	switch (address)
	{
	case MISA:
		return true; // immutable but writable

	case UREG00:
		ureg00 = value;
		return true;
	
	// others are read-only, so treat as non-existant
	default:
		return false;
	}
}

std::wstring CSR::getName(uint32_t address)
{
	switch (address)
	{
	case MISA :
		return L"misa";

	case UREG00:
		return L"ureg00";

	case CYCLE:
		return L"cycle";
	case TIME:
		return L"time";
	case INSTRET:
		return L"instret";

	case CYCLEH:
		return L"cycleh";
	case TIMEH:
		return L"timeh";
	case INSTRETH:
		return L"instreth";

	case MVENDORID:
		return L"mvendorid";
	case MARCHID:
		return L"marchid";
	case MIMPID:
		return L"mimpid";
	case MHARTID:
		return L"mhartid";

	case DEBUG:
		return L"debug";

	default:
		return L"???";
	}
}
