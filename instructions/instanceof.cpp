#ifndef JDECOMPILER_INSTANCEOF_INSTRUCTION_CPP
#define JDECOMPILER_INSTANCEOF_INSTRUCTION_CPP

namespace jdecompiler {

	struct InstanceofInstruction: InstructionWithIndex {
		InstanceofInstruction(uint16_t index): InstructionWithIndex(index) {}
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new InstanceofOperation(context, index); }
	};
}

#endif
