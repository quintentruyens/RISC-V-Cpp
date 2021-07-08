#include <cstdint>
#include <iostream>
#include "../MemoryMap.h"
#include "CPU.h"
#include "CSR.h"

CPU::CPU(const std::function<void()>& startDebug)
	: csr(this, startDebug)
{
	opcodeLookup = {
		&CPU::LOAD,   &CPU::XXX,  &CPU::XXX, &CPU::MISC_MEM, &CPU::OP_IMM, &CPU::AUIPC, &CPU::XXX, &CPU::XXX,
		&CPU::STORE,  &CPU::XXX,  &CPU::XXX, &CPU::XXX,      &CPU::OP,     &CPU::LUI,   &CPU::XXX, &CPU::XXX,
		&CPU::XXX,    &CPU::XXX,  &CPU::XXX, &CPU::XXX,      &CPU::XXX,    &CPU::XXX,   &CPU::XXX, &CPU::XXX,
		&CPU::BRANCH, &CPU::JALR, &CPU::XXX, &CPU::JAL,      &CPU::SYSTEM, &CPU::XXX,   &CPU::XXX, &CPU::XXX,
	};

	reset();
}

CPU::~CPU()
{
}

void CPU::connectBus(Bus* bus)
{
	this->bus = bus;
	timer = bus->timer;
}

void CPU::clock()
{
	csr.clock();

	currentExceptionType = ExceptionType::NoException;
	newPc = pc + 4;
	MemAccessResult instrAccessResult = bus->read(pc, instruction);
	if (instrAccessResult == MemAccessResult::NotInRange)
		createException(ExceptionType::InstructionAccessFault, pc);
	else if (instrAccessResult == MemAccessResult::Misaligned) // This should not be possible, pc should always be 4-byte aligned
		throw "instruction accesses should never be misaligned";
	else
	{
		// Succes
		uint32_t opcode = ((InstructionType::B*)&instruction)->opcode;

		if ((opcode & 0b11) != 0b11)
		{
			createException(ExceptionType::IllegalInstruction, instruction);
		}
		else
		{
			InstructionDecoder decoder = opcodeLookup[opcode >> 2];
			Instruction instr = (this->*decoder)(instruction);
		
			(this->*instr.execute)();

			if ((newPc & 3) != 0)
				createException(ExceptionType::InstructionAddressMisaligned, newPc);
		}
	}

	if (currentExceptionType != ExceptionType::NoException)
	{
		// An exception occured
		pc = csr.executeException(pc, getCause(currentExceptionType), exceptionVal, false);
	}
	else
		pc = newPc;

	// Interrupts are checked at the very end to simulate being at the very front while keeping both exception types close together
	// Being at the front avoids getting the wrong mepc when an interrupt is available immediately upon executing an mret, as
	// well as the problem of an exception getting lost when an interrupt occurs on the same instruction.
	auto interrupts = csr.checkInterrupts(newPc);

	if (interrupts.hasInterrupt) {
		pc = interrupts.newPc;
	}
}

void CPU::reset()
{
	pc = MemoryMap::Text.BaseAddr;

	csr.reset(0);
}

uint32_t CPU::readReg(uint32_t index)
{
	if (index == 0) return 0;

	return regs[index];
}

void CPU::writeReg(uint32_t index, uint32_t data)
{
	if (index != 0)
		regs[index] = data;
}

// OPCODES
CPU::Instruction CPU::XXX(uint32_t instr)
{
	createException(ExceptionType::IllegalInstruction, instruction);
	return { L"???", ArgumentType::None, &CPU::Nop };
}

CPU::Instruction CPU::OP_IMM(uint32_t instr)
{
	InstructionType::I i = punnInstruction<InstructionType::I>(instr);

	switch (i.func3)
	{
	case 0b000:
		return { L"addi", ArgumentType::Immediate, &CPU::AddI };
	case 0b010:
		return { L"slti", ArgumentType::Immediate, &CPU::SltI };
	case 0b011:
		return { L"sltiu", ArgumentType::Immediate, &CPU::SltIU };
	case 0b100:
		return { L"xori", ArgumentType::Immediate, &CPU::XorI };
	case 0b110:
		return { L"ori", ArgumentType::Immediate, &CPU::OrI };
	case 0b111:
		return { L"andi", ArgumentType::Immediate, &CPU::AndI };
	case 0b001:
		if (i.imm & 0xFFE0) break;
		return { L"slli", ArgumentType::Immediate, &CPU::SllI };
	case 0b101:
		switch ((i.imm & 0xFFE0) >> 5)
		{
		case 0:
			return { L"srli", ArgumentType::Immediate, &CPU::SrlI };
		case 32:
			return { L"srai", ArgumentType::Immediate, &CPU::SraI };
		default:
			break;
		}
		break;
	default:
		break;
	}

	createException(ExceptionType::IllegalInstruction, instruction);
	return { L"???", ArgumentType::None, &CPU::Nop };
}

CPU::Instruction CPU::OP(uint32_t instr)
{
	InstructionType::R i = punnInstruction<InstructionType::R>(instr);

	switch (i.func7)
	{
	case 0:
		switch (i.func3)
		{
		case 0b000:
			return { L"add", ArgumentType::Register, &CPU::Add };
		case 0b001:
			return { L"sll", ArgumentType::Register, &CPU::Sll };
		case 0b010:
			return { L"slt", ArgumentType::Register, &CPU::Slt };
		case 0b011:
			return { L"sltu", ArgumentType::Register, &CPU::SltU };
		case 0b100:
			return { L"xor", ArgumentType::Register, &CPU::Xor };
		case 0b101:
			return { L"srl", ArgumentType::Register, &CPU::Srl };
		case 0b110:
			return { L"or", ArgumentType::Register, &CPU::Or };
		case 0b111:
			return { L"and", ArgumentType::Register, &CPU::And };
		default:
			break;
		}
		break;
	case 32:
		switch (i.func3)
		{
		case 0b000:
			return { L"sub", ArgumentType::Register, &CPU::Sub };
		case 0b101:
			return { L"sra", ArgumentType::Register, &CPU::Sra };
		default:
			break;
		}
		break;
	case 1:
		switch (i.func3)
		{
		case 0b000:
			return { L"mul", ArgumentType::Register, &CPU::Mul };
		case 0b001:
			return { L"mulh", ArgumentType::Register, &CPU::MulH };
		case 0b010:
			return { L"mulhsu", ArgumentType::Register, &CPU::MulHSU };
		case 0b011:
			return { L"mulhu", ArgumentType::Register, &CPU::MulHU };
		case 0b100:
			return { L"div", ArgumentType::Register, &CPU::Div };
		case 0b101:
			return { L"divu", ArgumentType::Register, &CPU::DivU };
		case 0b110:
			return { L"rem", ArgumentType::Register, &CPU::Rem };
		case 0b111:
			return { L"remu", ArgumentType::Register, &CPU::RemU };
		default:
			break;
		}
		break;
	default:
		break;
	}

	createException(ExceptionType::IllegalInstruction, instruction);
	return { L"???", ArgumentType::None, &CPU::Nop };
}

CPU::Instruction CPU::LOAD(uint32_t instr)
{
	InstructionType::I i = punnInstruction<InstructionType::I>(instr);

	switch (i.func3)
	{
	case 0b000:
		return { L"lb", ArgumentType::LoadType, &CPU::Lb };
	case 0b001:
		return { L"lh", ArgumentType::LoadType, &CPU::Lh };
	case 0b010:
		return { L"lw", ArgumentType::LoadType, &CPU::Lw };
	case 0b100:
		return { L"lbu", ArgumentType::LoadType, &CPU::LbU };
	case 0b101:
		return { L"lhu", ArgumentType::LoadType, &CPU::LhU };
	default:
		break;
	}

	createException(ExceptionType::IllegalInstruction, instruction);
	return { L"???", ArgumentType::None, &CPU::Nop };
}

CPU::Instruction CPU::STORE(uint32_t instr)
{
	InstructionType::I i = punnInstruction<InstructionType::I>(instr);

	switch (i.func3)
	{
	case 0b000:
		return { L"sb", ArgumentType::StoreType, &CPU::Sb };
	case 0b001:
		return { L"sh", ArgumentType::StoreType, &CPU::Sh };
	case 0b010:
		return { L"sw", ArgumentType::StoreType, &CPU::Sw };
	default:
		break;
	}

	createException(ExceptionType::IllegalInstruction, instruction);
	return { L"???", ArgumentType::None, &CPU::Nop };
}

CPU::Instruction CPU::LUI(uint32_t instr)
{
	return { L"lui", ArgumentType::Upper, &CPU::Lui };
}

CPU::Instruction CPU::AUIPC(uint32_t instr)
{
	return { L"auipc", ArgumentType::Upper, &CPU::Auipc };
}

CPU::Instruction CPU::BRANCH(uint32_t instr)
{
	InstructionType::B i = punnInstruction<InstructionType::B>(instr);

	switch (i.func3)
	{
	case 0b000:
		return { L"beq", ArgumentType::Branch, &CPU::Beq };
	case 0b001:
		return { L"bne", ArgumentType::Branch, &CPU::Bne };
	case 0b100:
		return { L"blt", ArgumentType::Branch, &CPU::Blt };
	case 0b101:
		return { L"bge", ArgumentType::Branch, &CPU::Bge };
	case 0b110:
		return { L"bltu", ArgumentType::Branch, &CPU::BltU };
	case 0b111:
		return { L"bgeu", ArgumentType::Branch, &CPU::BgeU };
	default:
		break;
	}

	createException(ExceptionType::IllegalInstruction, instruction);
	return { L"???", ArgumentType::None, &CPU::Nop };
}

CPU::Instruction CPU::JAL(uint32_t instr)
{
	return { L"jal", ArgumentType::Jump, &CPU::Jal };
}

CPU::Instruction CPU::JALR(uint32_t instr)
{
	InstructionType::I i = punnInstruction<InstructionType::I>(instr);

	if (i.func3 == 0)
		return { L"jalr", ArgumentType::Immediate, &CPU::Jalr };

	createException(ExceptionType::IllegalInstruction, instruction);
	return { L"???", ArgumentType::None, &CPU::Nop };
}

CPU::Instruction CPU::MISC_MEM(uint32_t instr)
{
	InstructionType::I i = punnInstruction<InstructionType::I>(instr);

	if (i.func3 == 0)
		return { L"fence", ArgumentType::FenceType, &CPU::Fence };

	createException(ExceptionType::IllegalInstruction, instruction);
	return { L"???", ArgumentType::None, &CPU::Nop };
}

CPU::Instruction CPU::SYSTEM(uint32_t instr)
{
	InstructionType::R i = punnInstruction<InstructionType::R>(instr);

	switch (i.func3)
	{
	case 0b000:
		if (i.rd == 0 && i.rs1 == 0)
			switch (i.func7)
			{
			case 0b0000000:
				if (i.rs2 == 0)
					return { L"ecall", ArgumentType::None, &CPU::Ecall };
				else if (i.rs2 == 1)
					return { L"ebreak", ArgumentType::None, &CPU::Ebreak };
				break;
			case 0b0011000:
				if (i.rs2 == 2)
					return { L"mret", ArgumentType::None, &CPU::Mret };
				break;
			default:
				break;
			}
		break;
	case 0b001:
		return { L"csrrw", ArgumentType::CSRRegister, &CPU::CsrRW };
	case 0b010:
		return { L"csrrs", ArgumentType::CSRRegister, &CPU::CsrRS };
	case 0b011:
		return { L"csrrc", ArgumentType::CSRRegister, &CPU::CsrRC };
	case 0b101:
		return { L"csrrwi", ArgumentType::CSRImmediate, &CPU::CsrRWI };
	case 0b110:
		return { L"csrrsi", ArgumentType::CSRImmediate, &CPU::CsrRSI };
	case 0b111:
		return { L"csrrci", ArgumentType::CSRImmediate, &CPU::CsrRCI };
	default:
		break;
	}

	createException(ExceptionType::IllegalInstruction, instruction);
	return { L"???", ArgumentType::None, &CPU::Nop };
}

// Instructions
// Immediate
void CPU::AddI()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	writeReg(instr.rd, readReg(instr.rs1) + getImm(instr));
}

void CPU::SltI()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	writeReg(instr.rd, ((int32_t)readReg(instr.rs1) < (int32_t)getImm(instr)) ? 1 : 0);
}

void CPU::SltIU()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	writeReg(instr.rd, (readReg(instr.rs1) < getImm(instr)) ? 1 : 0);
}

void CPU::XorI()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	writeReg(instr.rd, readReg(instr.rs1) ^ getImm(instr));
}

void CPU::OrI()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	writeReg(instr.rd, readReg(instr.rs1) | getImm(instr));
}

void CPU::AndI()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	writeReg(instr.rd, readReg(instr.rs1) & getImm(instr));
}

void CPU::SllI()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	writeReg(instr.rd, readReg(instr.rs1) << (instr.imm & 0x1F));
}

void CPU::SrlI()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	writeReg(instr.rd, readReg(instr.rs1) >> (instr.imm & 0x1F));
}

void CPU::SraI()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	writeReg(instr.rd, (int32_t)readReg(instr.rs1) >> (instr.imm & 0x1F));
}

// Register
void CPU::Add()
{
	InstructionType::R instr = punnInstruction<InstructionType::R>(instruction);
	writeReg(instr.rd, readReg(instr.rs1) + readReg(instr.rs2));
}

void CPU::Sub()
{
	InstructionType::R instr = punnInstruction<InstructionType::R>(instruction);
	writeReg(instr.rd, readReg(instr.rs1) - readReg(instr.rs2));
}

void CPU::Sll()
{
	InstructionType::R instr = punnInstruction<InstructionType::R>(instruction);
	writeReg(instr.rd, readReg(instr.rs1) << (readReg(instr.rs2) & 0x1F));
}

void CPU::Slt()
{
	InstructionType::R instr = punnInstruction<InstructionType::R>(instruction);
	writeReg(instr.rd, ((int32_t)readReg(instr.rs1) < (int32_t)readReg(instr.rs2)) ? 1 : 0);
}

void CPU::SltU()
{
	InstructionType::R instr = punnInstruction<InstructionType::R>(instruction);
	writeReg(instr.rd, (readReg(instr.rs1) < readReg(instr.rs2)) ? 1 : 0);
}

void CPU::Xor()
{
	InstructionType::R instr = punnInstruction<InstructionType::R>(instruction);
	writeReg(instr.rd, readReg(instr.rs1) ^ readReg(instr.rs2));
}

void CPU::Srl()
{
	InstructionType::R instr = punnInstruction<InstructionType::R>(instruction);
	writeReg(instr.rd, readReg(instr.rs1) >> (readReg(instr.rs2) & 0x1F));
}

void CPU::Sra()
{
	InstructionType::R instr = punnInstruction<InstructionType::R>(instruction);
	writeReg(instr.rd, (int32_t)readReg(instr.rs1) >> (readReg(instr.rs2) & 0x1F));
}

void CPU::Or()
{
	InstructionType::R instr = punnInstruction<InstructionType::R>(instruction);
	writeReg(instr.rd, readReg(instr.rs1) | readReg(instr.rs2));
}

void CPU::And()
{
	InstructionType::R instr = punnInstruction<InstructionType::R>(instruction);
	writeReg(instr.rd, readReg(instr.rs1) & readReg(instr.rs2));
}

void CPU::Mul()
{
	InstructionType::R instr = punnInstruction<InstructionType::R>(instruction);
	writeReg(instr.rd, readReg(instr.rs1) * readReg(instr.rs2));
}

void CPU::MulH()
{
	InstructionType::R instr = punnInstruction<InstructionType::R>(instruction);
	writeReg(instr.rd, (uint64_t)((int64_t)(int32_t)readReg(instr.rs1) * (int64_t)(int32_t)readReg(instr.rs2)) >> 32);
}

void CPU::MulHSU()
{
	InstructionType::R instr = punnInstruction<InstructionType::R>(instruction);
	writeReg(instr.rd, (uint64_t)((int64_t)(int32_t)readReg(instr.rs1) * (int64_t)(uint64_t)readReg(instr.rs2)) >> 32);
}

void CPU::MulHU()
{
	InstructionType::R instr = punnInstruction<InstructionType::R>(instruction);
	writeReg(instr.rd, ((uint64_t)readReg(instr.rs1) * (uint64_t)readReg(instr.rs2)) >> 32);
}

void CPU::Div()
{
	InstructionType::R instr = punnInstruction<InstructionType::R>(instruction);
	int32_t divisor = (int32_t)readReg(instr.rs2);
	if (divisor == 0)
		writeReg(instr.rd, 0xFFFF'FFFF);
	else
		writeReg(instr.rd, (int32_t)readReg(instr.rs1) / divisor);
}

void CPU::DivU()
{
	InstructionType::R instr = punnInstruction<InstructionType::R>(instruction);
	uint32_t divisor = readReg(instr.rs2);
	if (divisor == 0)
		writeReg(instr.rd, 0xFFFF'FFFF);
	else
		writeReg(instr.rd, readReg(instr.rs1) / divisor);
}

void CPU::Rem()
{
	InstructionType::R instr = punnInstruction<InstructionType::R>(instruction);
	int32_t divisor = (int32_t)readReg(instr.rs2);
	if (divisor == 0)
		writeReg(instr.rd, readReg(instr.rs1));
	else
		writeReg(instr.rd, (int32_t)readReg(instr.rs1) % divisor);
}

void CPU::RemU()
{
	InstructionType::R instr = punnInstruction<InstructionType::R>(instruction);
	uint32_t divisor = readReg(instr.rs2);
	if (divisor == 0)
		writeReg(instr.rd, readReg(instr.rs1));
	else
		writeReg(instr.rd, readReg(instr.rs1) % divisor);
}

// Load
void CPU::Lb()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	uint32_t addr = getImm(instr) + readReg(instr.rs1);
	uint32_t value;
	MemAccessResult accessResult = bus->read(addr, value, false, DataSize::Byte, true);
	switch (accessResult)
	{
	case MemAccessResult::Success:
		writeReg(instr.rd, value);
		return;
	case MemAccessResult::NotInRange:
		createException(ExceptionType::LoadAccessFault, addr);
		return;
	case MemAccessResult::Misaligned:
		createException(ExceptionType::LoadAddressMisaligned, addr);
		return;
	default:
		return;
	}
}

void CPU::Lh()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	uint32_t addr = getImm(instr) + readReg(instr.rs1);
	uint32_t value;
	MemAccessResult accessResult = bus->read(addr, value, false, DataSize::HalfWord, true);
	switch (accessResult)
	{
	case MemAccessResult::Success:
		writeReg(instr.rd, value);
		return;
	case MemAccessResult::NotInRange:
		createException(ExceptionType::LoadAccessFault, addr);
		return;
	case MemAccessResult::Misaligned:
		createException(ExceptionType::LoadAddressMisaligned, addr);
		return;
	default:
		return;
	}
}

void CPU::Lw()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	uint32_t addr = getImm(instr) + readReg(instr.rs1);
	uint32_t value;
	MemAccessResult accessResult = bus->read(addr, value, false, DataSize::Word, true);
	switch (accessResult)
	{
	case MemAccessResult::Success:
		writeReg(instr.rd, value);
		return;
	case MemAccessResult::NotInRange:
		createException(ExceptionType::LoadAccessFault, addr);
		return;
	case MemAccessResult::Misaligned:
		createException(ExceptionType::LoadAddressMisaligned, addr);
		return;
	default:
		return;
	}
}

void CPU::LbU()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	uint32_t addr = getImm(instr) + readReg(instr.rs1);
	uint32_t value;
	MemAccessResult accessResult = bus->read(addr, value, false, DataSize::Byte, false);
	switch (accessResult)
	{
	case MemAccessResult::Success:
		writeReg(instr.rd, value);
		return;
	case MemAccessResult::NotInRange:
		createException(ExceptionType::LoadAccessFault, addr);
		return;
	case MemAccessResult::Misaligned:
		createException(ExceptionType::LoadAddressMisaligned, addr);
		return;
	default:
		return;
	}
}

void CPU::LhU()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	uint32_t addr = getImm(instr) + readReg(instr.rs1);
	uint32_t value;
	MemAccessResult accessResult = bus->read(addr, value, false, DataSize::HalfWord, false);
	switch (accessResult)
	{
	case MemAccessResult::Success:
		writeReg(instr.rd, value);
		return;
	case MemAccessResult::NotInRange:
		createException(ExceptionType::LoadAccessFault, addr);
		return;
	case MemAccessResult::Misaligned:
		createException(ExceptionType::LoadAddressMisaligned, addr);
		return;
	default:
		return;
	}
}

// Store
void CPU::Sb()
{
	InstructionType::S instr = punnInstruction<InstructionType::S>(instruction);
	uint32_t addr = getImm(instr) + readReg(instr.rs1);
	MemAccessResult accessResult = bus->write(addr, readReg(instr.rs2), DataSize::Byte);
	
	if (accessResult == MemAccessResult::NotInRange)
		createException(ExceptionType::StoreAccessFault, addr);
	else if (accessResult == MemAccessResult::Misaligned)
		createException(ExceptionType::StoreAddressMisaligned, addr);
}

void CPU::Sh()
{
	InstructionType::S instr = punnInstruction<InstructionType::S>(instruction);
	uint32_t addr = getImm(instr) + readReg(instr.rs1);
	MemAccessResult accessResult = bus->write(addr, readReg(instr.rs2), DataSize::HalfWord);

	if (accessResult == MemAccessResult::NotInRange)
		createException(ExceptionType::StoreAccessFault, addr);
	else if (accessResult == MemAccessResult::Misaligned)
		createException(ExceptionType::StoreAddressMisaligned, addr);
}

void CPU::Sw()
{
	InstructionType::S instr = punnInstruction<InstructionType::S>(instruction);
	uint32_t addr = getImm(instr) + readReg(instr.rs1);
	MemAccessResult accessResult = bus->write(addr, readReg(instr.rs2), DataSize::Word);

	if (accessResult == MemAccessResult::NotInRange)
		createException(ExceptionType::StoreAccessFault, addr);
	else if (accessResult == MemAccessResult::Misaligned)
		createException(ExceptionType::StoreAddressMisaligned, addr);
}

// Lui
void CPU::Lui()
{
	InstructionType::U instr = punnInstruction<InstructionType::U>(instruction);
	writeReg(instr.rd, getImm(instr));
}

// Auipc
void CPU::Auipc()
{
	InstructionType::U instr = punnInstruction<InstructionType::U>(instruction);
	writeReg(instr.rd, getImm(instr) + pc);
}

// Branch
void CPU::Beq()
{
	InstructionType::B instr = punnInstruction<InstructionType::B>(instruction);
	if (readReg(instr.rs1) == readReg(instr.rs2))
		newPc = pc + getImm(instr);
}

void CPU::Bne()
{
	InstructionType::B instr = punnInstruction<InstructionType::B>(instruction);
	if (readReg(instr.rs1) != readReg(instr.rs2))
		newPc = pc + getImm(instr);
}

void CPU::Blt()
{
	InstructionType::B instr = punnInstruction<InstructionType::B>(instruction);
	if ((int32_t)readReg(instr.rs1) < (int32_t)readReg(instr.rs2))
		newPc = pc + getImm(instr);
}

void CPU::Bge()
{
	InstructionType::B instr = punnInstruction<InstructionType::B>(instruction);
	if ((int32_t)readReg(instr.rs1) >= (int32_t)readReg(instr.rs2))
		newPc = pc + getImm(instr);
}

void CPU::BltU()
{
	InstructionType::B instr = punnInstruction<InstructionType::B>(instruction);
	if (readReg(instr.rs1) < readReg(instr.rs2))
		newPc = pc + getImm(instr);
}

void CPU::BgeU()
{
	InstructionType::B instr = punnInstruction<InstructionType::B>(instruction);
	if (readReg(instr.rs1) >= readReg(instr.rs2))
		newPc = pc + getImm(instr);
}

// Jal
void CPU::Jal()
{
	InstructionType::J instr = punnInstruction<InstructionType::J>(instruction);
	writeReg(instr.rd, pc + 4);
	newPc = pc + getImm(instr);
}

// Jalr
void CPU::Jalr()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	writeReg(instr.rd, pc + 4);
	newPc = (readReg(instr.rs1) + getImm(instr)) & 0xFFFF'FFFEU;
}

// Fence
void CPU::Fence()
{ // Can be implemented as nop, memory ordering is already strict
}

// System
void CPU::CsrRW()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	uint32_t csrAddr = instr.imm;

	uint32_t writeVal = readReg(instr.rs1);

	if (instr.rd != 0)
	{
		uint32_t value;
		if (!csr.read(csrAddr, value, false)) {
			createException(ExceptionType::IllegalInstruction, instruction);
			return;
		}

		writeReg(instr.rd, value);
	}

	if (!csr.write(csrAddr, writeVal)) {
		createException(ExceptionType::IllegalInstruction, instruction);
		return;
	}
}

void CPU::CsrRS()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	uint32_t csrAddr = instr.imm;

	// read old value
	uint32_t value;
	if (!csr.read(csrAddr, value, false)) {
		createException(ExceptionType::IllegalInstruction, instruction);
		return;
	}

	writeReg(instr.rd, value);


	if (instr.rs1 != 0)
	{
		// write new value
		uint32_t newValue = value | readReg(instr.rs1);

		if (!csr.write(csrAddr, newValue)) {
			createException(ExceptionType::IllegalInstruction, instruction);
			return;
		}
	}
}

void CPU::CsrRC()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	uint32_t csrAddr = instr.imm;

	// read old value
	uint32_t value;
	if (!csr.read(csrAddr, value, false)) {
		createException(ExceptionType::IllegalInstruction, instruction);
		return;
	}

	writeReg(instr.rd, value);


	if (instr.rs1 != 0)
	{
		// write new value
		uint32_t newValue = value & (~readReg(instr.rs1));

		if (!csr.write(csrAddr, newValue)) {
			createException(ExceptionType::IllegalInstruction, instruction);
			return;
		}
	}
}

void CPU::CsrRWI()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	uint32_t csrAddr = instr.imm;

	if (instr.rd != 0)
	{
		uint32_t value;
		if (!csr.read(csrAddr, value, false)) {
			createException(ExceptionType::IllegalInstruction, instruction);
			return;
		}

		writeReg(instr.rd, value);
	}

	if (!csr.write(csrAddr, instr.rs1)) {
		createException(ExceptionType::IllegalInstruction, instruction);
		return;
	}
}

void CPU::CsrRSI()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	uint32_t csrAddr = instr.imm;

	// read old value
	uint32_t value;
	if (!csr.read(csrAddr, value, false)) {
		createException(ExceptionType::IllegalInstruction, instruction);
		return;
	}

	writeReg(instr.rd, value);


	if (instr.rs1 != 0)
	{
		// write new value
		uint32_t newValue = value | instr.rs1;

		if (!csr.write(csrAddr, newValue)) {
			createException(ExceptionType::IllegalInstruction, instruction);
			return;
		}
	}
}

void CPU::CsrRCI()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	uint32_t csrAddr = instr.imm;

	// read old value
	uint32_t value;
	if (!csr.read(csrAddr, value, false)) {
		createException(ExceptionType::IllegalInstruction, instruction);
		return;
	}

	writeReg(instr.rd, value);


	if (instr.rs1 != 0)
	{
		// write new value
		uint32_t newValue = value & (~instr.rs1);

		if (!csr.write(csrAddr, newValue)) {
			createException(ExceptionType::IllegalInstruction, instruction);
			return;
		}
	}
}

void CPU::Ebreak()
{
	createException(ExceptionType::EnvironmentBreak);
}

void CPU::Ecall()
{
	createException(ExceptionType::MEnvironmentCall);
}

void CPU::Mret()
{
	newPc = csr.returnExcepion();
}

// Nop
void CPU::Nop()
{
}

// Exceptions
void CPU::createException(ExceptionType type, uint32_t value)
{
	if (type > currentExceptionType)
	{
		currentExceptionType = type;
		exceptionVal = value;
	}
}

uint32_t CPU::getCause(ExceptionType type)
{
	switch (type)
	{
	case CPU::ExceptionType::LoadAccessFault:
		return 5;
	case CPU::ExceptionType::StoreAccessFault:
		return 7;
	case CPU::ExceptionType::LoadAddressMisaligned:
		return 4;
	case CPU::ExceptionType::StoreAddressMisaligned:
		return 6;
	case CPU::ExceptionType::EnvironmentBreak:
		return 3;
	case CPU::ExceptionType::UEnvironmentCall:
		return 8;
	case CPU::ExceptionType::SEnvironmentCall:
		return 9;
	case CPU::ExceptionType::MEnvironmentCall:
		return 11;
	case CPU::ExceptionType::InstructionAddressMisaligned:
		return 0;
	case CPU::ExceptionType::IllegalInstruction:
		return 2;
	case CPU::ExceptionType::InstructionAccessFault:
		return 1;
	default:
		return 0; // Includes NoException, shouldn't happen
	}
}

// Disassembly and convenience functions
std::wstring CPU::disassemble(uint32_t instr)
{
	uint32_t opcode = ((InstructionType::B*)&instr)->opcode;

	if ((opcode & 0b11) != 0b11)
		return L"???";

	InstructionDecoder decoder = opcodeLookup[opcode >> 2];
	Instruction decodedInstr = (this->*decoder)(instr);

	std::wstring name = decodedInstr.name;
	std::wstring args;
	switch (decodedInstr.argumentType)
	{
	case CPU::ArgumentType::Immediate:
	{
		InstructionType::I i = punnInstruction<InstructionType::I>(instr);

		args = regName(i.rd) + L", " + regName(i.rs1) + L", " + hex(getImm(i));
		break;
	}
	case CPU::ArgumentType::Register:
	{
		InstructionType::R i = punnInstruction<InstructionType::R>(instr);

		args = regName(i.rd) + L", " + regName(i.rs1) + L", " + regName(i.rs2);
		break;
	}
	case CPU::ArgumentType::LoadType:
	{
		InstructionType::I i = punnInstruction<InstructionType::I>(instr);

		args = regName(i.rd) + L", " + hex(getImm(i)) + L"(" + regName(i.rs1) + L")";
		break;
	}
	case CPU::ArgumentType::StoreType:
	{
		InstructionType::S i = punnInstruction<InstructionType::S>(instr);

		args = regName(i.rs2) + L", " + hex(getImm(i)) + L"(" + regName(i.rs1) + L")";
		break;
	}
	case CPU::ArgumentType::Upper:
	{
		InstructionType::U i = punnInstruction<InstructionType::U>(instr);
		
		args = regName(i.rd) + L", " + hex(getImm(i));
		break;
	}
	case CPU::ArgumentType::Branch:
	{
		InstructionType::B i = punnInstruction<InstructionType::B>(instr);

		args = regName(i.rs1) + L", " + regName(i.rs2) + L", " + hex(getImm(i));
		break;
	}
	case CPU::ArgumentType::Jump:
	{
		InstructionType::J i = punnInstruction<InstructionType::J>(instr);

		args = regName(i.rd) + L", " + hex(getImm(i));
		break;
	}
	case CPU::ArgumentType::FenceType:
	{
		InstructionType::I i = punnInstruction<InstructionType::I>(instr);

		uint32_t imm = getImm(i);
		args = ((imm & 0x0F0) >> 4) + L", " + (imm & 0x00F);
		break;
	}
	case CPU::ArgumentType::CSRRegister:
	{
		InstructionType::I i = punnInstruction<InstructionType::I>(instr);

		args = regName(i.rd) + L", " + this->csr.getName(i.imm) + L", " + regName(i.rs1);
		break;
	}
	case CPU::ArgumentType::CSRImmediate:
	{
		InstructionType::I i = punnInstruction<InstructionType::I>(instr);

		args = regName(i.rd) + L", " + this->csr.getName(i.imm) + L", " + std::to_wstring(i.rs1);
		break;
	}
	case CPU::ArgumentType::None:
		args = L"";
		break;
	default:
		break;
	}

	return name + L" " + args;
}

std::wstring CPU::regName(uint32_t reg)
{
	static const std::array<std::wstring, 32> names = {
		L"zero", L"ra", L"sp", L"gp", L"tp", L"t0", L"t1", L"t2", L"s0", L"s1", L"a0",  L"a1",  L"a2", L"a3", L"a4", L"a5", 
		L"a6",   L"a7", L"s2", L"s3", L"s4", L"s5", L"s6", L"s7", L"s8", L"s9", L"s10", L"s11", L"t3", L"t4", L"t5", L"t6"
	};

	return names[reg];
}

std::wstring CPU::hex(uint32_t n)
{
	std::wstring s(L"0x\0\0\0\0\0\0\0", 10);
	for (int i = 9; i >= 2; i--, n >>= 4)
		s[i] = L"0123456789abcdef"[n & 0xF];
	return s;
};

uint32_t signExtend12(uint32_t imm)
{
	return (imm & 0xFFF) | ((imm & 0x800) ? 0xFFFFF000 : 0x0000'0000);
}

uint32_t signExtend13(uint32_t imm)
{
	return (imm & 0x1FFF) | ((imm & 0x1000) ? 0xFFFFE000 : 0x0000'0000);
}

uint32_t signExtend21(uint32_t imm)
{
	return (imm & 0x1FFFFF) | ((imm & 0x100000) ? 0xFFE00000 : 0x0000'0000);
}

uint32_t CPU::getImm(InstructionType::I instr)
{
	return signExtend12(instr.imm);
}

uint32_t CPU::getImm(InstructionType::S instr)
{
	return signExtend12(
		instr.imm11_5 << 5 | 
		instr.imm4_0
	);
}

uint32_t CPU::getImm(InstructionType::B instr)
{
	return signExtend13(
		instr.imm12   << 12 |
		instr.imm11   << 11 |
		instr.imm10_5 <<  5 |
		instr.imm4_1  <<  1
	);
}

uint32_t CPU::getImm(InstructionType::U instr)
{
	return instr.imm31_12 << 12;
}

uint32_t CPU::getImm(InstructionType::J instr)
{
	return signExtend21(
		instr.imm20    << 20 |
		instr.imm19_12 << 12 |
		instr.imm11    << 11 |
		instr.imm10_1  <<  1
	);
}
