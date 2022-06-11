#ifndef JDECOMPILER_ARRAY_LOAD_OPERATIONS_CPP
#define JDECOMPILER_ARRAY_LOAD_OPERATIONS_CPP

namespace jdecompiler::operations {

	struct ArrayLoadOperation: Operation {
		public:
			const Operation *const index, *const array;
			const Type* const returnType;

		protected:
			ArrayLoadOperation(const Type* elementType, const DecompilationContext& context):
					index(context.stack.popAs(INT)), array(context.stack.pop()),
					returnType(array->getReturnTypeAs(new ArrayType(elementType))->elementType) {
				index->allowImplicitCast();
			}

		public:
			virtual string toString(const StringifyContext& context) const override {
				return array->toString(context) + '[' + index->toString(context) + ']';
			}

			virtual const Type* getReturnType() const override {
				return returnType;
			}
	};

	struct IALoadOperation: ArrayLoadOperation {
		IALoadOperation(const DecompilationContext& context): ArrayLoadOperation(INT, context) {}
	};

	struct LALoadOperation: ArrayLoadOperation {
		LALoadOperation(const DecompilationContext& context): ArrayLoadOperation(LONG, context) {}
	};

	struct FALoadOperation: ArrayLoadOperation {
		FALoadOperation(const DecompilationContext& context): ArrayLoadOperation(FLOAT, context) {}
	};

	struct DALoadOperation: ArrayLoadOperation {
		DALoadOperation(const DecompilationContext& context): ArrayLoadOperation(DOUBLE, context) {}
	};

	struct AALoadOperation: ArrayLoadOperation {
		AALoadOperation(const DecompilationContext& context): ArrayLoadOperation(
				context.stack.lookup(1)->getReturnTypeAs(AnyType::getArrayTypeInstance())->elementType, context) {}
	};

	struct BALoadOperation: ArrayLoadOperation {
		BALoadOperation(const DecompilationContext& context): ArrayLoadOperation(BYTE_OR_BOOLEAN, context) {}
	};

	struct CALoadOperation: ArrayLoadOperation {
		CALoadOperation(const DecompilationContext& context): ArrayLoadOperation(CHAR, context) {}
	};

	struct SALoadOperation: ArrayLoadOperation {
		SALoadOperation(const DecompilationContext& context): ArrayLoadOperation(SHORT, context) {}
	};
}

#endif
