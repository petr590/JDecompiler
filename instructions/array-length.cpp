#ifndef JDECOMPILER_ARRAY_LENGTH_INSTRUCTION_CPP
#define JDECOMPILER_ARRAY_LENGTH_INSTRUCTION_CPP

namespace jdecompiler {

	struct ArrayLengthInstruction: Instruction {
		public: virtual const Operation* toOperation(const DecompilationContext& context) const override { return new ArrayLengthOperation(context); }
	};
}

#endif
