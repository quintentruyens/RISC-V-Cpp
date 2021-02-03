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
}

void CPU::clock()
{
	bool illegalInstruction = false;
	instruction = bus->read(pc);

	uint32_t opcode = ((InstructionType::B*)&instruction)->opcode;

	if ((opcode & 0b11) != 0b11)
	{
		illegalInstruction = true;
		return;
	}

	InstructionDecoder decoder = opcodeLookup[opcode >> 2];
	Instruction instr = (this->*decoder)(instruction);


	(this->*instr.execute)();
	pc += 4;
	
	instret++;
	cycle++;
}

void CPU::reset()
{
	pc = MemoryMap::Text.BaseAddr;

	for (auto& reg : regs)
		reg = 0;
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
	bool illegalInstruction = true;
	return { "???", ArgumentType::None, &CPU::Nop };
}

CPU::Instruction CPU::OP_IMM(uint32_t instr)
{
	InstructionType::I i = punnInstruction<InstructionType::I>(instr);

	switch (i.func3)
	{
	case 0b000:
		return { "addi", ArgumentType::Immediate, &CPU::AddI };
	case 0b010:
		return { "slti", ArgumentType::Immediate, &CPU::SltI };
	case 0b011:
		return { "sltiu", ArgumentType::Immediate, &CPU::SltIU };
	case 0b100:
		return { "xori", ArgumentType::Immediate, &CPU::XorI };
	case 0b110:
		return { "ori", ArgumentType::Immediate, &CPU::OrI };
	case 0b111:
		return { "andi", ArgumentType::Immediate, &CPU::AndI };
	case 0b001:
		if (i.imm & 0xFFE0) break;
		return { "slli", ArgumentType::Immediate, &CPU::SllI };
	case 0b101:
		switch ((i.imm & 0xFFE0) >> 5)
		{
		case 0:
			return { "srli", ArgumentType::Immediate, &CPU::SrlI };
		case 32:
			return { "srai", ArgumentType::Immediate, &CPU::SraI };
		default:
			break;
		}
		break;
	default:
		break;
	}

	return { "???", ArgumentType::None, &CPU::Nop };
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
			return { "add", ArgumentType::Register, &CPU::Add };
		case 0b001:
			return { "sll", ArgumentType::Register, &CPU::Sll };
		case 0b010:
			return { "slt", ArgumentType::Register, &CPU::Slt };
		case 0b011:
			return { "sltu", ArgumentType::Register, &CPU::SltU };
		case 0b100:
			return { "xor", ArgumentType::Register, &CPU::Xor };
		case 0b101:
			return { "srl", ArgumentType::Register, &CPU::Srl };
		case 0b110:
			return { "or", ArgumentType::Register, &CPU::Or };
		case 0b111:
			return { "and", ArgumentType::Register, &CPU::And };
		default:
			break;
		}
		break;
	case 32:
		switch (i.func3)
		{
		case 0b000:
			return { "sub", ArgumentType::Register, &CPU::Sub };
		case 0b101:
			return { "sra", ArgumentType::Register, &CPU::Sra };
		default:
			break;
		}
		break;
	case 1:
		switch (i.func3)
		{
		case 0b000:
			return { "mul", ArgumentType::Register, &CPU::Mul };
		case 0b001:
			return { "mulh", ArgumentType::Register, &CPU::MulH };
		case 0b010:
			return { "mulhsu", ArgumentType::Register, &CPU::MulHSU };
		case 0b011:
			return { "mulhu", ArgumentType::Register, &CPU::MulHU };
		case 0b100:
			return { "div", ArgumentType::Register, &CPU::Div };
		case 0b101:
			return { "divu", ArgumentType::Register, &CPU::DivU };
		case 0b110:
			return { "rem", ArgumentType::Register, &CPU::Rem };
		case 0b111:
			return { "remu", ArgumentType::Register, &CPU::RemU };
		default:
			break;
		}
		break;
	default:
		break;
	}

	return { "???", ArgumentType::None, &CPU::Nop };
}

CPU::Instruction CPU::LOAD(uint32_t instr)
{
	InstructionType::I i = punnInstruction<InstructionType::I>(instr);

	switch (i.func3)
	{
	case 0b000:
		return { "lb", ArgumentType::LoadType, &CPU::Lb };
	case 0b001:
		return { "lh", ArgumentType::LoadType, &CPU::Lh };
	case 0b010:
		return { "lw", ArgumentType::LoadType, &CPU::Lw };
	case 0b100:
		return { "lbu", ArgumentType::LoadType, &CPU::LbU };
	case 0b101:
		return { "lhu", ArgumentType::LoadType, &CPU::LhU };
	default:
		break;
	}

	return { "???", ArgumentType::None, &CPU::Nop };
}

CPU::Instruction CPU::STORE(uint32_t instr)
{
	InstructionType::I i = punnInstruction<InstructionType::I>(instr);

	switch (i.func3)
	{
	case 0b000:
		return { "sb", ArgumentType::StoreType, &CPU::Sb };
	case 0b001:
		return { "sh", ArgumentType::StoreType, &CPU::Sh };
	case 0b010:
		return { "sw", ArgumentType::StoreType, &CPU::Sw };
	default:
		break;
	}

	return { "???", ArgumentType::None, &CPU::Nop };
}

CPU::Instruction CPU::LUI(uint32_t instr)
{
	return { "lui", ArgumentType::Upper, &CPU::Lui };
}

CPU::Instruction CPU::AUIPC(uint32_t instr)
{
	return { "auipc", ArgumentType::Upper, &CPU::Auipc };
}

CPU::Instruction CPU::BRANCH(uint32_t instr)
{
	InstructionType::B i = punnInstruction<InstructionType::B>(instr);

	switch (i.func3)
	{
	case 0b000:
		return { "beq", ArgumentType::Branch, &CPU::Beq };
	case 0b001:
		return { "bne", ArgumentType::Branch, &CPU::Bne };
	case 0b100:
		return { "blt", ArgumentType::Branch, &CPU::Blt };
	case 0b101:
		return { "bge", ArgumentType::Branch, &CPU::Bge };
	case 0b110:
		return { "bltu", ArgumentType::Branch, &CPU::BltU };
	case 0b111:
		return { "bgeu", ArgumentType::Branch, &CPU::BgeU };
	default:
		break;
	}

	return { "???", ArgumentType::None, &CPU::Nop };
}

CPU::Instruction CPU::JAL(uint32_t instr)
{
	return { "jal", ArgumentType::Jump, &CPU::Jal };
}

CPU::Instruction CPU::JALR(uint32_t instr)
{
	InstructionType::I i = punnInstruction<InstructionType::I>(instr);

	if (i.func3 == 0)
		return { "jalr", ArgumentType::Immediate, &CPU::Jalr };

	return { "???", ArgumentType::None, &CPU::Nop };
}

CPU::Instruction CPU::MISC_MEM(uint32_t instr)
{
	InstructionType::I i = punnInstruction<InstructionType::I>(instr);

	if (i.func3 == 0)
		return { "fence", ArgumentType::FenceType, &CPU::Fence };

	return { "???", ArgumentType::None, &CPU::Nop };
}

CPU::Instruction CPU::SYSTEM(uint32_t instr)
{
	InstructionType::I i = punnInstruction<InstructionType::I>(instr);

	switch (i.func3)
	{
	case 0b001:
		return { "csrrw", ArgumentType::CSRRegister, &CPU::CsrRW };
	case 0b010:
		return { "csrrs", ArgumentType::CSRRegister, &CPU::CsrRS };
	case 0b011:
		return { "csrrc", ArgumentType::CSRRegister, &CPU::CsrRC };
	case 0b101:
		return { "csrrwi", ArgumentType::CSRImmediate, &CPU::CsrRWI };
	case 0b110:
		return { "csrrsi", ArgumentType::CSRImmediate, &CPU::CsrRSI };
	case 0b111:
		return { "csrrci", ArgumentType::CSRImmediate, &CPU::CsrRCI };
	default:
		break;
	}

	return { "???", ArgumentType::None, &CPU::Nop };
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
	writeReg(instr.rd, (int32_t)readReg(instr.rs1) / (int32_t)readReg(instr.rs2));
}

void CPU::DivU()
{
	InstructionType::R instr = punnInstruction<InstructionType::R>(instruction);
	writeReg(instr.rd, readReg(instr.rs1) / readReg(instr.rs2));
}

void CPU::Rem()
{
	InstructionType::R instr = punnInstruction<InstructionType::R>(instruction);
	writeReg(instr.rd, (int32_t)readReg(instr.rs1) % (int32_t)readReg(instr.rs2));
}

void CPU::RemU()
{
	InstructionType::R instr = punnInstruction<InstructionType::R>(instruction);
	writeReg(instr.rd, readReg(instr.rs1) % readReg(instr.rs2));
}

// Load
void CPU::Lb()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	writeReg(instr.rd, bus->read(getImm(instr) + readReg(instr.rs1), false, Byte, true));
}

void CPU::Lh()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	writeReg(instr.rd, bus->read(getImm(instr) + readReg(instr.rs1), false, HalfWord, true));
}

void CPU::Lw()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	writeReg(instr.rd, bus->read(getImm(instr) + readReg(instr.rs1), false, Word, true));
}

void CPU::LbU()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	writeReg(instr.rd, bus->read(getImm(instr) + readReg(instr.rs1), false, Byte, false));
}

void CPU::LhU()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	writeReg(instr.rd, bus->read(getImm(instr) + readReg(instr.rs1), false, HalfWord, false));
}

// Store
void CPU::Sb()
{
	InstructionType::S instr = punnInstruction<InstructionType::S>(instruction);
	bus->write(getImm(instr) + readReg(instr.rs1), readReg(instr.rs2), Byte);
}

void CPU::Sh()
{
	InstructionType::S instr = punnInstruction<InstructionType::S>(instruction);
	bus->write(getImm(instr) + readReg(instr.rs1), readReg(instr.rs2), HalfWord);
}

void CPU::Sw()
{
	InstructionType::S instr = punnInstruction<InstructionType::S>(instruction);
	bus->write(getImm(instr) + readReg(instr.rs1), readReg(instr.rs2), Word);
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
	{
		pc += getImm(instr) - 4; // Also account for the increment that will come after the branch targetAddr;
	}
}

void CPU::Bne()
{
	InstructionType::B instr = punnInstruction<InstructionType::B>(instruction);
	if (readReg(instr.rs1) != readReg(instr.rs2))
		pc += getImm(instr) - 4; // Also account for the increment that will come after the branch
}

void CPU::Blt()
{
	InstructionType::B instr = punnInstruction<InstructionType::B>(instruction);
	if ((int32_t)readReg(instr.rs1) < (int32_t)readReg(instr.rs2))
		pc += getImm(instr) - 4; // Also account for the increment that will come after the branch
}

void CPU::Bge()
{
	InstructionType::B instr = punnInstruction<InstructionType::B>(instruction);
	if ((int32_t)readReg(instr.rs1) >= (int32_t)readReg(instr.rs2))
		pc += getImm(instr) - 4; // Also account for the increment that will come after the branch
}

void CPU::BltU()
{
	InstructionType::B instr = punnInstruction<InstructionType::B>(instruction);
	if (readReg(instr.rs1) < readReg(instr.rs2))
		pc += getImm(instr) - 4; // Also account for the increment that will come after the branch
}

void CPU::BgeU()
{
	InstructionType::B instr = punnInstruction<InstructionType::B>(instruction);
	if (readReg(instr.rs1) >= readReg(instr.rs2))
		pc += getImm(instr) - 4; // Also account for the increment that will come after the branch
}

// Jal
void CPU::Jal()
{
	InstructionType::J instr = punnInstruction<InstructionType::J>(instruction);
	writeReg(instr.rd, pc + 4);
	pc += getImm(instr) - 4;
}

// Jalr
void CPU::Jalr()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	writeReg(instr.rd, pc + 4);
	pc = (readReg(instr.rs1) + getImm(instr) - 4) & 0xFFFF'FFFEU;
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

	if (instr.rd != 0)
	{
		uint32_t value;
		if (!csr.read(csrAddr, value))
			return; // invalid instruction exception

		writeReg(instr.rd, value);
	}

	if (!csr.write(csrAddr, readReg(instr.rs1)))
		return; // invalid instruction exception
}

void CPU::CsrRS()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	uint32_t csrAddr = instr.imm;

	// read old value
	uint32_t value;
	if (!csr.read(csrAddr, value))
		return; // invalid instruction exception

	writeReg(instr.rd, value);


	if (instr.rs1 != 0)
	{
		// write new value
		uint32_t newValue = value | readReg(instr.rs1);

		if (!csr.write(csrAddr, newValue))
			return; // invalid instruction exception
	}
}

void CPU::CsrRC()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	uint32_t csrAddr = instr.imm;

	// read old value
	uint32_t value;
	if (!csr.read(csrAddr, value))
		return; // invalid instruction exception

	writeReg(instr.rd, value);


	if (instr.rs1 != 0)
	{
		// write new value
		uint32_t newValue = value & (~readReg(instr.rs1));

		if (!csr.write(csrAddr, newValue))
			return; // invalid instruction exception
	}
}

void CPU::CsrRWI()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	uint32_t csrAddr = instr.imm;

	if (instr.rd != 0)
	{
		uint32_t value;
		if (!csr.read(csrAddr, value))
			return; // invalid instruction exception

		writeReg(instr.rd, value);
	}

	if (!csr.write(csrAddr, instr.rs1))
		return; // invalid instruction exception
}

void CPU::CsrRSI()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	uint32_t csrAddr = instr.imm;

	// read old value
	uint32_t value;
	if (!csr.read(csrAddr, value))
		return; // invalid instruction exception

	writeReg(instr.rd, value);


	if (instr.rs1 != 0)
	{
		// write new value
		uint32_t newValue = value | instr.rs1;

		if (!csr.write(csrAddr, newValue))
			return; // invalid instruction exception
	}
}

void CPU::CsrRCI()
{
	InstructionType::I instr = punnInstruction<InstructionType::I>(instruction);
	uint32_t csrAddr = instr.imm;

	// read old value
	uint32_t value;
	if (!csr.read(csrAddr, value))
		return; // invalid instruction exception

	writeReg(instr.rd, value);


	if (instr.rs1 != 0)
	{
		// write new value
		uint32_t newValue = value & (~instr.rs1);

		if (!csr.write(csrAddr, newValue))
			return; // invalid instruction exception
	}
}

// Nop
void CPU::Nop()
{
}

// Disassembly
std::string CPU::disassemble(uint32_t instr)
{
	uint32_t opcode = ((InstructionType::B*)&instr)->opcode;

	if ((opcode & 0b11) != 0b11)
		return "???";

	InstructionDecoder decoder = opcodeLookup[opcode >> 2];
	Instruction decodedInstr = (this->*decoder)(instr);

	std::string name = decodedInstr.name;
	std::string args;
	switch (decodedInstr.argumentType)
	{
	case CPU::Immediate:
	{
		InstructionType::I i = punnInstruction<InstructionType::I>(instr);

		args = regName(i.rd) + ", " + regName(i.rs1) + ", " + hex(getImm(i));
		break;
	}
	case CPU::Register:
	{
		InstructionType::R i = punnInstruction<InstructionType::R>(instr);

		args = regName(i.rd) + ", " + regName(i.rs1) + ", " + regName(i.rs2);
		break;
	}
	case CPU::LoadType:
	{
		InstructionType::I i = punnInstruction<InstructionType::I>(instr);

		args = regName(i.rd) + ", " + hex(getImm(i)) + "(" + regName(i.rs1) + ")";
		break;
	}
	case CPU::StoreType:
	{
		InstructionType::S i = punnInstruction<InstructionType::S>(instr);

		args = regName(i.rs2) + ", " + hex(getImm(i)) + "(" + regName(i.rs1) + ")";
		break;
	}
	case CPU::Upper:
	{
		InstructionType::U i = punnInstruction<InstructionType::U>(instr);
		
		args = regName(i.rd) + ", " + hex(getImm(i));
		break;
	}
	case CPU::Branch:
	{
		InstructionType::B i = punnInstruction<InstructionType::B>(instr);

		args = regName(i.rs1) + ", " + regName(i.rs2) + ", " + hex(getImm(i));
		break;
	}
	case CPU::Jump:
	{
		InstructionType::J i = punnInstruction<InstructionType::J>(instr);

		args = regName(i.rd) + ", " + hex(getImm(i));
		break;
	}
	case CPU::FenceType:
	{
		InstructionType::I i = punnInstruction<InstructionType::I>(instr);

		uint32_t imm = getImm(i);
		args = ((imm & 0x0F0) >> 4) + ", " + (imm & 0x00F);
		break;
	}
	case CPU::CSRRegister:
	{
		InstructionType::I i = punnInstruction<InstructionType::I>(instr);

		args = regName(i.rd) + ", " + this->csr.getName(i.imm) + ", " + regName(i.rs1);
		break;
	}
	case CPU::CSRImmediate:
	{
		InstructionType::I i = punnInstruction<InstructionType::I>(instr);

		args = regName(i.rd) + ", " + this->csr.getName(i.imm) + ", " + std::to_string(i.rs1);
		break;
	}
	case CPU::None:
		args = "";
		break;
	default:
		break;
	}

	return name + " " + args;
}

std::string CPU::regName(uint32_t reg)
{
	static const std::array<std::string, 32> names = {
		"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0",  "a1",  "a2", "a3", "a4", "a5", 
		"a6",   "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
	};

	return names[reg];
}

std::string CPU::hex(uint32_t n)
{
	std::string s("0x\0\0\0\0\0\0\0", 10);
	for (int i = 9; i >= 2; i--, n >>= 4)
		s[i] = "0123456789abcdef"[n & 0xF];
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
