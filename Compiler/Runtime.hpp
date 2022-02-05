#pragma once

// Standard
#include <string>
#include <vector>


namespace Instructions {
	enum : uint16_t {
		ALLOCATE_MEMORY,
		ADD_U64_MM
	};
}


class Runtime {
public:
	std::vector<uint8_t> executable;
	uint64_t instr_ptr;

	std::vector<uint8_t> memory;
	uint64_t stack_ptr;

public:
	/* Instructions */

	void allocate();

	// add 2 variables and store the result in first variable
	void addU64_MM();


	/*  */

	template<typename T>
	T read(uint64_t src_adress)
	{
		T var;
		std::memcpy(&var, memory.data() + src_adress, sizeof(T));
		return var;
	}

	template<typename T>
	void write(T value, uint64_t dest_adress)
	{
		std::memcpy(memory.data() + dest_adress, &value, sizeof(T));
	}

	void executeInstruction();
};
