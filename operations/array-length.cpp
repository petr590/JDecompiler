#ifndef JDECOMPILER_ARRAY_LENGTH_OPERATION_CPP
#define JDECOMPILER_ARRAY_LENGTH_OPERATION_CPP

namespace jdecompiler::operations {

	struct ArrayLengthOperation: IntOperation {
		protected:
			const Operation* const array;

		public:
			ArrayLengthOperation(const DecompilationContext& context): array(context.stack.popAs(AnyType::getArrayTypeInstance())) {}

			virtual string toString(const StringifyContext& context) const override {
				return toStringPriority(array, context, Associativity::LEFT) + ".length";
			}
	};
}

#endif
