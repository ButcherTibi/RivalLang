
// Header
#include "Runtime.hpp"


void Runtime::allocate()
{
	// uint64_t operand_0_adress = read<uint64_t>(instr_ptr + 1);
	// uint64_t operand_0_value = read<uint64_t>(operand_0_adress);
}

void Runtime::addU64_MM()
{
	uint64_t operand_0_adress = read<uint64_t>(instr_ptr + 2);
	uint64_t operand_1_adress = read<uint64_t>(instr_ptr + 6);

	uint64_t operand_0_value = read<uint64_t>(operand_0_adress);
	uint64_t operand_1_value = read<uint64_t>(operand_1_adress);

	write<uint64_t>(operand_0_value + operand_1_value, operand_0_adress);

	instr_ptr += 10;
}

void Runtime::executeInstruction()
{
	uint8_t instr = executable[instr_ptr];

	switch (instr) {
	case Instructions::ADD_U64_MM: {
		addU64_MM();
		break;
	}

	default:
		throw;
	}
}
