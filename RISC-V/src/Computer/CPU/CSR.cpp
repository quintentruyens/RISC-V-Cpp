#include <cstdint>
#include <chrono>
#include "CSR.h"
#include "CPU.h"

#include "../Timer.h"

constexpr uint32_t MSTATUS = 0x300;
constexpr uint32_t MISA = 0x301;

constexpr uint32_t MIE = 0x304;
constexpr uint32_t MTVEC = 0x305;

constexpr uint32_t MSCRATCH = 0x340;
constexpr uint32_t MEPC = 0x341;
constexpr uint32_t MCAUSE = 0x342;
constexpr uint32_t MTVAL = 0x343;
constexpr uint32_t MIP = 0x344;

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
	validAdresses = { MSTATUS, MISA, MIE, MTVEC, MSCRATCH, MEPC, MCAUSE, MTVAL, MIP, UREG00, CYCLE, TIME, INSTRET, CYCLEH, TIMEH, INSTRETH, 
		MVENDORID, MARCHID, MIMPID, MHARTID, DEBUG };
	
}

CSR::~CSR()
{
}

void CSR::reset(uint32_t cause)
{
	mstatus.MIE = 0;
	mcause = cause;
}

bool CSR::read(uint32_t address, uint32_t& value, bool bReadOnly)
{
	switch (address)
	{
	case MSTATUS:
		value = *(uint32_t*)&mstatus;
		return true;
	case MISA:
		value = 0b01'0000'00000000100010000000000000U;
		return true;

	case MIE:
		value = *(uint32_t*)&mie;
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
	case MIP:
		updateMip();
		value = *(uint32_t*)&mipInternal;
		return true;

	case UREG00:
		value = ureg00;
		return true;

	case CYCLE:
		value = this->cpu->cycle & 0xFFFF'FFFFU;
		return true;
	case TIME:
		value = this->cpu->timer->getTimeLow();
		return true;
	case INSTRET:
		value = this->cpu->instret & 0xFFFF'FFFFU;
		return true;

	case CYCLEH:
		value = this->cpu->cycle >> 32;
		return true;
	case TIMEH:
		value = this->cpu->timer->getTimeHigh();
		return true;
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
	case MSTATUS:
	{
		MStatus castValue = *(MStatus*)&value;
		mstatus.MIE = castValue.MIE;
		mstatus.MPIE = castValue.MPIE;
		return true;
	}
	case MISA:
		return true; // immutable but writable

	case MIE:
	{
		MInterruptCSR castValue = *(MInterruptCSR*)&value;
		mie.bits.MEI = castValue.bits.MEI;
		mie.bits.MSI = castValue.bits.MSI;
		mie.bits.MTI = castValue.bits.MTI;
		return true;
	}
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
	case MIP:
		// M-level ip bits are not writable directly, U- or S-level bits would be writable if they were implemented
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
	case MSTATUS:
		return L"mstatus";
	case MISA:
		return L"misa";

	case MIE:
		return L"mie";
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
	case MIP:
		return L"mip";

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

	mstatus.MPIE = mstatus.MIE;
	mstatus.MIE = 0;

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
	mstatus.MIE = mstatus.MPIE;
	mstatus.MPIE = 1;
	return mepc;
}

CSR::CheckInterruptsReturn CSR::checkInterrupts(uint32_t epc)
{
	if (!mstatus.MIE)
		return { false, 0 };

	updateMip();

	MInterruptCSR enabledInterrupts = { mipInternal.word & mie.word };
	if (enabledInterrupts.word == 0)
		return { false, 0 };
	
	uint32_t cause = -1; // This value should always be overridden, no other bits should ever be 1
	if (enabledInterrupts.bits.MEI)
		cause = 11;
	else if (enabledInterrupts.bits.MSI)
		cause = 3;
	else if (enabledInterrupts.bits.MTI)
		cause = 7;
	else if (enabledInterrupts.bits.SEI)
		cause = 9;
	else if (enabledInterrupts.bits.SSI)
		cause = 1;
	else if (enabledInterrupts.bits.STI)
		cause = 5;
	else if (enabledInterrupts.bits.UEI)
		cause = 8;
	else if (enabledInterrupts.bits.USI)
		cause = 0;
	else if (enabledInterrupts.bits.UTI)
		cause = 4;

	uint32_t newPc = executeException(epc, cause, 0, true);
	return { true, newPc };
}

void CSR::updateMip()
{
	mipInternal.bits.MTI = this->cpu->timer->hasInterrupt() ? 1 : 0;
	mipInternal.bits.MEI = this->cpu->bus->hasInterrupt() ? 1 : 0;
	// software interrupts are primarily intended for interprocessor communication, so are not implemented
}
