#include <cstdint>
#include <chrono>
#include "CSR.h"
#include "CPU.h"

constexpr uint32_t MISA = 0x301;

constexpr uint32_t MTVEC = 0x305;

constexpr uint32_t MSCRATCH = 0x340;
constexpr uint32_t MEPC = 0x341;
constexpr uint32_t MCAUSE = 0x342;
constexpr uint32_t MTVAL = 0x343;

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
	validAdresses = { MISA, MTVEC, MSCRATCH, MEPC, MCAUSE, MTVAL, UREG00, CYCLE, TIME, INSTRET, CYCLEH, TIMEH, INSTRETH, 
		MVENDORID, MARCHID, MIMPID, MHARTID, DEBUG };
}

CSR::~CSR()
{
}

bool CSR::read(uint32_t address, uint32_t& value, bool bReadOnly)
{
	switch (address)
	{
	case MISA:
		value = 0b01'0000'00000000100010000000000000U;
		return true;

	case MTVEC:
		value = mtvec;
		return true;

	case MSCRATCH:
		value = mscratch;
		return true;
	case MEPC:
		value = mepc;
		return true;
	case MCAUSE:
		value = mcause;
		return true;
	case MTVAL:
		value = mtval;
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
		uint64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
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
		uint64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
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
		if (!bReadOnly)
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

	case MTVEC:
		mtvec = value & 0xFFFF'FFFDU;
		return true;

	case MSCRATCH:
		mscratch = value;
		return true;
	case MEPC:
		mepc = value & 0xFFFF'FFFCU;
		return true;
	case MCAUSE:
		mcause = value;
		return true;
	case MTVAL:
		mtval = value;
		return true;

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
	case MISA:
		return L"misa";

	case MTVEC:
		return L"mtvec";

	case MSCRATCH:
		return L"mscratch";
	case MEPC:
		return L"mepc";
	case MCAUSE:
		return L"mcause";
	case MTVAL:
		return L"mtval";

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

uint32_t CSR::executeException(uint32_t epc, uint32_t causeNum, uint32_t val, bool bInterrupt)
{
	mepc = epc;
	mcause = (bInterrupt ? 0x8000'0000 : 0x0000'0000) | causeNum;
	mtval = val;

	if (!bInterrupt || (mtvec & 0x1) == 0)
	{
		// Non-vectored interrupt
		return mtvec;
	}
	else
	{
		return (mtvec & 0xFFFF'FFFCU) + 4 * causeNum;
	}
}

uint32_t CSR::returnExcepion()
{
	return mepc;
}
