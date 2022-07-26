#ifndef JDECOMPILER_INSTRUCTION_CPP
#define JDECOMPILER_INSTRUCTION_CPP

#include "instruction.h"

namespace jdecompiler {

	const Block* Instruction::toBlock(const DisassemblerContext&) const {
		return nullptr;
	}
}

#endif
