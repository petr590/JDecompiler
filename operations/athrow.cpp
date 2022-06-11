#ifndef JDECOMPILER_ATHROW_OPERATION_CPP
#define JDECOMPILER_ATHROW_OPERATION_CPP

namespace jdecompiler::operations {

	struct AThrowOperation: VoidOperation {
		protected: const Operation* const exceptionOperation;

		public:
			AThrowOperation(const DecompilationContext& context): exceptionOperation(context.stack.pop()) {}

			virtual string toString(const StringifyContext& context) const override {
				return "throw " + exceptionOperation->toString(context);
			}
	};
}

#endif
