#ifndef JDECOMPILER_CMP_INSTRUCTIONS_CPP
#define JDECOMPILER_CMP_INSTRUCTIONS_CPP

namespace jdecompiler {

	struct LCmpInstruction: Instruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new LCmpOperation(context); }
	};

	struct FCmpInstruction: Instruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new FCmpOperation(context); }
	};

	struct DCmpInstruction: Instruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new DCmpOperation(context); }
	};
}

#endif
