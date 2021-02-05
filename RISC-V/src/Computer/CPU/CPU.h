#pragma once
#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include <functional>
#include "../Bus.h"
#include "CSR.h"

namespace InstructionType
{
	struct R
	{
		unsigned opcode : 7;
		unsigned rd : 5;
		unsigned func3 : 3;
		unsigned rs1 : 5;
		unsigned rs2 : 5;
		unsigned func7 : 7;
	};

	struct I
	{
		unsigned opcode : 7;
		unsigned rd : 5;
		unsigned func3 : 3;
		unsigned rs1 : 5;
		unsigned imm : 12;
	};

	struct S
	{
		unsigned opcode : 7;
		unsigned imm4_0 : 5;
		unsigned func3 : 3;
		unsigned rs1 : 5;
		unsigned rs2 : 5;
		unsigned imm11_5 : 7;
	};

	struct B
	{
		unsigned opcode : 7;
		unsigned imm11 : 1;
		unsigned imm4_1 : 4;
		unsigned func3 : 3;
		unsigned rs1 : 5;
		unsigned rs2 : 5;
		unsigned imm10_5 : 6;
		unsigned imm12 : 1;
	};

	struct U
	{
		unsigned opcode : 7;
		unsigned rd : 5;
		unsigned imm31_12 : 20;
	};

	struct J
	{
		unsigned opcode : 7;
		unsigned rd : 5;
		unsigned imm19_12 : 8;
		unsigned imm11 : 1;
		unsigned imm10_1 : 10;
		unsigned imm20 : 1;
	};
}


class CPU
{
public:
	CPU(const std::function<void()>& startDebug);
	~CPU();

public:
	void connectBus(Bus* bus);
	void clock();
	void reset();

public:
	Bus* bus;
	CSR csr;

public:
	uint32_t pc = 0;

	std::array<uint32_t, 32> regs;

	uint32_t instruction = 0;

public:
	uint32_t readReg(uint32_t index);
	void writeReg(uint32_t index, uint32_t data);

	/*struct Opcode
	{
		std::string name;
		void (CPU::* operate)(void) = nullptr;
	};
	std::vector<Opcode> opcodeLookup;*/

	// This will define the type of arguments used with a function, eg. Registers (like 'add a0, a1, a2'), Immediate (like 'addi a0, a1, 0x5')
	// or None (like 'ecall'). This is used to disassemble instructions.
	enum ArgumentType
	{
		Immediate = 0, // eg. 'addi a0, a1, 0x5'
		Register, // eg. 'add a0, a1, a2'
		LoadType, // eg. 'lw a1, 0x5(a2)'
		StoreType, // eg. 'sw a1, 0x5(a2)'
		Upper, // eg. 'lui a0, 0x5'
		Branch, // eg. 'beq a0, a1, 0x5'
		Jump, // eg. 'jal ra, 0x5'
		FenceType, // eg. 'fence 1, 1'
		CSRRegister, // eg. 'csrrw a0, uscratch, a1'
		CSRImmediate, // eg. 'csrrwi a0, uscratch, 5'
		None // eg. 'ecall'
	};

	struct Instruction
	{
		std::wstring name;
		ArgumentType argumentType;
		void (CPU::* execute)(void) = nullptr;
	};

	typedef Instruction(CPU::* InstructionDecoder)(uint32_t instr);

	// This is an collection of 32 functions which decode an instruction. The index depends on bits 2-7 of the opcode (the first two have to be 0b11)
	std::vector<InstructionDecoder> opcodeLookup; 

public:
	// instruction decoders that will be placed in opcodeLookup
	Instruction XXX(uint32_t instr);
	Instruction OP_IMM(uint32_t instr);
	Instruction OP(uint32_t instr);
	Instruction LOAD(uint32_t instr);
	Instruction STORE(uint32_t instr);
	Instruction LUI(uint32_t instr);
	Instruction AUIPC(uint32_t instr);
	Instruction BRANCH(uint32_t instr);
	Instruction JALR(uint32_t instr);
	Instruction JAL(uint32_t instr);
	Instruction MISC_MEM(uint32_t instr);
	Instruction SYSTEM(uint32_t instr);


	// instruction execute functions
	// Immediate
	void AddI(); void SltI(); void SltIU(); void XorI(); void OrI(); void AndI(); void SllI(); void SrlI(); void SraI(); 
	// Register
	void Add(); void Sub(); void Sll(); void Slt(); void SltU(); void Xor(); void Srl(); void Sra(); void Or(); void And();
	void Mul(); void MulH(); void MulHSU(); void MulHU(); void Div(); void DivU(); void Rem(); void RemU();
	// Load
	void Lb(); void Lh(); void Lw(); void LbU(); void LhU(); 
	// Store
	void Sb(); void Sh(); void Sw();
	// Lui
	void Lui();
	// Auipc
	void Auipc();
	// Branch
	void Beq(); void Bne(); void Blt(); void Bge(); void BltU(); void BgeU();
	// Jal
	void Jal();
	// Jalr
	void Jalr();
	// Fence
	void Fence();
	// System
	void CsrRW(); void CsrRS(); void CsrRC(); void CsrRWI(); void CsrRSI(); void CsrRCI();
	// Nop
	void Nop();

public:
	// convenience functions
	template <typename T>
	T punnInstruction(uint32_t instr)
	{
		return *((T*)&instr);
	}

	std::wstring disassemble(uint32_t instr);
	std::wstring regName(uint32_t reg);
	std::wstring hex(uint32_t n);
	uint32_t getImm(InstructionType::I instr); // Signed
	uint32_t getImm(InstructionType::S instr); // Signed
	uint32_t getImm(InstructionType::B instr); // Signed
	uint32_t getImm(InstructionType::U instr); // Signed
	uint32_t getImm(InstructionType::J instr); // Signed

public:
	uint64_t instret = 0; // amount of instructions executed
	uint64_t cycle = 0; // amount of cycles
};