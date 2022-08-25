#ifndef JDECOMPILER_IINC_INSTRUCTION_CPP
#define JDECOMPILER_IINC_INSTRUCTION_CPP

namespace jdecompiler {

	struct IIncInstruction: InstructionWithIndex {
		const int16_t value;

		IIncInstruction(uint16_t index, int16_t value): InstructionWithIndex(index), value(value) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override {
			return new IIncOperation(context, index, value);
		}
	};

	IIncOperation::IIncOperation(const DecompilationContext& context, uint16_t index, int16_t value):
			variable(context.getCurrentScope()->getVariable(index, true)), value(value),
			isShortInc(value == 1 || value == -1) /* isShortInc true when we can write ++ or -- */ {

		const Type* variableType = variable.castTypeTo(ANY_INT);

		const ILoadOperation* iloadOperation = context.stack.empty() ? nullptr : dynamic_cast<const ILoadOperation*>(context.stack.top());

		if(isShortInc && iloadOperation != nullptr && iloadOperation->variable == variable) {
			context.stack.pop();
			returnType = variableType;
			isPostInc = true;
		} else {
			const ILoadInstruction* iloadInstruction =
					dynamic_cast<const ILoadInstruction*>(context.getInstructionNoexcept(context.index + 1));
			if(iloadInstruction != nullptr && iloadInstruction->index == index) {
				iloadInstruction->setNullOperation();
				returnType = variableType;
			} else {
				returnType = VOID;
			}
		}
	}
}

#endif
