#ifndef JDECOMPILER_POP_INSTRUCTION_CPP
#define JDECOMPILER_POP_INSTRUCTION_CPP

namespace jdecompiler {

	template<TypeSize size>
	struct PopInstruction: Instruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new PopOperation<size>(context); }
	};
}

#endif
