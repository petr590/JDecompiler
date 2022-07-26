#ifndef JDECOMPILER_ATHROW_OPERATION_CPP
#define JDECOMPILER_ATHROW_OPERATION_CPP

namespace jdecompiler::operations {

	struct AThrowOperation: VoidOperation {
		const Operation* const exception;

		AThrowOperation(const DecompilationContext& context): exception(context.stack.popAs(THROWABLE)) {}

		virtual string toString(const StringifyContext& context) const override {
			return "throw " + exception->toString(context);
		}
	};
}

#endif
