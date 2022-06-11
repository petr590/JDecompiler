#ifndef JDECOMPILER_POP_OPERATION_CPP
#define JDECOMPILER_POP_OPERATION_CPP

namespace jdecompiler::operations {

	template<TypeSize size>
	struct PopOperation: VoidOperation, TypeSizeTemplatedOperation<size> {
		const Operation* const operation;

		PopOperation(const DecompilationContext& context): operation(context.stack.pop()) {
			TypeSizeTemplatedOperation<size>::checkTypeSize(operation->getReturnType());
		}

		virtual string toString(const StringifyContext& context) const override {
			return operation->toString(context);
		}
	};
}

#endif
