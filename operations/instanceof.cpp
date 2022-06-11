#ifndef JDECOMPILER_INSTANCEOF_OPERATION_CPP
#define JDECOMPILER_INSTANCEOF_OPERATION_CPP

namespace jdecompiler::operations {

	struct InstanceofOperation: BooleanOperation {
		protected:
			const Type* const type;
			const Operation* const object;

		public:
			InstanceofOperation(const DecompilationContext& context, uint16_t index):
					type(parseReferenceType(context.constPool.get<ClassConstant>(index)->name)), object(context.stack.pop()) {}

			virtual string toString(const StringifyContext& context) const override {
				return toStringPriority(object, context, Associativity::LEFT) + " instanceof " + type->toString(context.classinfo);
			}

			virtual Priority getPriority() const override {
				return Priority::INSTANCEOF;
			}
	};
}

#endif
