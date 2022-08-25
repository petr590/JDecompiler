#ifndef JDECOMPILER_DUP_INSTRUCTION_CPP
#define JDECOMPILER_DUP_INSTRUCTION_CPP

namespace jdecompiler {

	template<TypeSize size>
	struct DupInstruction: Instruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new DupOperation<size>(context); }
	};

	using Dup1Instruction = DupInstruction<TypeSize::FOUR_BYTES>;
	using Dup2Instruction = DupInstruction<TypeSize::EIGHT_BYTES>;

	struct DupX1Instruction: Instruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new DupX1Operation(context); }
	};

	struct DupX2Instruction: Instruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new DupX2Operation(context); }
	};

	struct Dup2X1Instruction: Instruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new Dup2X1Operation(context); }
	};

	struct Dup2X2Instruction: Instruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new Dup2X2Operation(context); }
	};
}

#endif
