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

CSR::CSR(CPU* cpu)
	: cpu(cpu)
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
		__debugbreak();
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

std::string CSR::getName(uint32_t address)
{
	switch (address)
	{
	case MISA :
		return "misa";

	case UREG00:
		return "ureg00";

	case CYCLE:
		return "cycle";
	case TIME:
		return "time";
	case INSTRET:
		return "instret";

	case CYCLEH:
		return "cycleh";
	case TIMEH:
		return "timeh";
	case INSTRETH:
		return "instreth";

	case MVENDORID:
		return "mvendorid";
	case MARCHID:
		return "marchid";
	case MIMPID:
		return "mimpid";
	case MHARTID:
		return "mhartid";

	case DEBUG:
		return "debug";

	default:
		return "???";
	}
}
