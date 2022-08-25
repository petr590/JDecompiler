#ifndef JDECOMPILER_SWAP_INSTRUCTION_CPP
#define JDECOMPILER_SWAP_INSTRUCTION_CPP

namespace jdecompiler {

	struct SwapInstruction: Instruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override {
			context.stack.push(context.stack.pop(), context.stack.pop());
			return nullptr;
		}
	};
}

#endif
