#ifndef JDECOMPILER_ARRAY_STORE_OPERATIONS_CPP
#define JDECOMPILER_ARRAY_STORE_OPERATIONS_CPP

namespace jdecompiler {

	struct ArrayStoreOperation: VoidOperation {
		protected:
			const Operation *const value, *const index, *const array;
			bool isInitializer = false;

		public:
			ArrayStoreOperation(const Type* elementType, const DecompilationContext& context): value(context.stack.popAs(elementType)),
					index(context.stack.popAs(INT)), array(context.stack.popAs(new ArrayType(elementType))) {

				index->allowImplicitCast();
				value->allowImplicitCast();
				//checkDup<Dup1Operation>(context, array);

				if(const Dup1Operation* dupArray = dynamic_cast<const Dup1Operation*>(array)) {
					if(const NewArrayOperation* newArray = dynamic_cast<const NewArrayOperation*>(dupArray->operation)) {
						newArray->initializer.push_back(value);
						isInitializer = true;
					}
				}
			};

			virtual string toString(const StringifyContext& context) const override {
				return isInitializer ? value->toString(context) :
						array->toString(context) + '[' + index->toString(context) + "] = " + value->toString(context);
			}

			virtual bool canAddToCode() const override {
				return !isInitializer;
			}

			virtual Priority getPriority() const override {
				return Priority::ASSIGNMENT;
			}
	};


	struct IAStoreOperation: ArrayStoreOperation {
		IAStoreOperation(const DecompilationContext& context): ArrayStoreOperation(INT, context) {}
	};

	struct LAStoreOperation: ArrayStoreOperation {
		LAStoreOperation(const DecompilationContext& context): ArrayStoreOperation(LONG, context) {}
	};

	struct FAStoreOperation: ArrayStoreOperation {
		FAStoreOperation(const DecompilationContext& context): ArrayStoreOperation(FLOAT, context) {}
	};

	struct DAStoreOperation: ArrayStoreOperation {
		DAStoreOperation(const DecompilationContext& context): ArrayStoreOperation(DOUBLE, context) {}
	};

	struct AAStoreOperation: ArrayStoreOperation {
		AAStoreOperation(const DecompilationContext& context): ArrayStoreOperation(
				context.stack.lookup(2)->getReturnTypeAs(AnyType::getArrayTypeInstance())->elementType, context) {}
	};

	struct BAStoreOperation: ArrayStoreOperation {
		BAStoreOperation(const DecompilationContext& context): ArrayStoreOperation(BYTE, context) {}
	};

	struct CAStoreOperation: ArrayStoreOperation {
		CAStoreOperation(const DecompilationContext& context): ArrayStoreOperation(CHAR, context) {}
	};

	struct SAStoreOperation: ArrayStoreOperation {
		SAStoreOperation(const DecompilationContext& context): ArrayStoreOperation(SHORT, context) {}
	};
}

#endif
