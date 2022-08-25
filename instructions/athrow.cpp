#ifndef JDECOMPILER_ATHROW_INSTRUCTION_CPP
#define JDECOMPILER_ATHROW_INSTRUCTION_CPP

namespace jdecompiler {

	struct AThrowInstruction: Instruction {
		public: virtual const Operation* toOperation(const DecompilationContext& context) const override { return new AThrowOperation(context); }
	};
}

#endif
