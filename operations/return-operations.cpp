#ifndef JDECOMPILER_RETURN_OPERATIONS_CPP
#define JDECOMPILER_RETURN_OPERATIONS_CPP

namespace jdecompiler {

	struct ReturnOperation: VoidOperation {
		public:
			const Operation* const value;

			ReturnOperation(const DecompilationContext& context, const Type* type):
					value(context.stack.popAs(type->castTo(context.descriptor.returnType))) {
				value->allowImplicitCast();
			}

			virtual string toString(const StringifyContext& context) const override {
				return "return " + value->toString(context);
			}
	};


	struct IReturnOperation: ReturnOperation {
		IReturnOperation(const DecompilationContext& context): ReturnOperation(context, ANY_INT_OR_BOOLEAN) {}
	};

	struct LReturnOperation: ReturnOperation {
		LReturnOperation(const DecompilationContext& context): ReturnOperation(context, LONG) {}
	};

	struct FReturnOperation: ReturnOperation {
		FReturnOperation(const DecompilationContext& context): ReturnOperation(context, FLOAT) {}
	};

	struct DReturnOperation: ReturnOperation {
		DReturnOperation(const DecompilationContext& context): ReturnOperation(context, DOUBLE) {}
	};

	struct AReturnOperation: ReturnOperation {
		AReturnOperation(const DecompilationContext& context): ReturnOperation(context, AnyObjectType::getInstance()) {}
	};
}

#endif
