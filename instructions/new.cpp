#ifndef JDECOMPILER_NEW_INSTRUCTION_CPP
#define JDECOMPILER_NEW_INSTRUCTION_CPP

namespace jdecompiler {

	struct NewInstruction: InstructionWithIndex {
		NewInstruction(uint16_t classIndex): InstructionWithIndex(classIndex) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new NewOperation(context, index); }
	};
}

#endif
