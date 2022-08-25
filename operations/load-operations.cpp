#ifndef JDECOMPILER_LOAD_OPERATIONS_CPP
#define JDECOMPILER_LOAD_OPERATIONS_CPP

namespace jdecompiler {

	struct LoadOperation: Operation {
		public:
			const uint16_t index;
			const Variable& variable;

		protected:
			const Operation* incOperation = nullptr;

		public:
			LoadOperation(const Type*, const DecompilationContext&, uint16_t);

			virtual string toString(const StringifyContext& context) const override {
				return incOperation != nullptr ? incOperation->toString(context) : context.getCurrentScope()->getNameFor(variable);
			}

			virtual const Type* getReturnType() const override {
				return variable.getType();
			}

			virtual bool isIncrement() const override {
				return incOperation != nullptr;
			}

			virtual void onCastReturnType(const Type* newType) const override {
				variable.setTypeShrinking(newType);
			}

			virtual void addVariableName(const string& name) const override {
				variable.addName(name);
			}
	};

	struct ILoadOperation: LoadOperation {
		ILoadOperation(const DecompilationContext& context, uint16_t index): LoadOperation(ANY_INT_OR_BOOLEAN, context, index) {}
	};

	struct LLoadOperation: LoadOperation {
		LLoadOperation(const DecompilationContext& context, uint16_t index): LoadOperation(LONG, context, index) {}
	};

	struct FLoadOperation: LoadOperation {
		FLoadOperation(const DecompilationContext& context, uint16_t index): LoadOperation(FLOAT, context, index) {}
	};

	struct DLoadOperation: LoadOperation {
		DLoadOperation(const DecompilationContext& context, uint16_t index): LoadOperation(DOUBLE, context, index) {}
	};

	struct ALoadOperation: LoadOperation {
		ALoadOperation(const DecompilationContext& context, uint16_t index): LoadOperation(AnyObjectType::getInstance(), context, index) {}

		virtual bool isReferenceToThis(const StringifyContext& context) const override {
			return !(context.modifiers & ACC_STATIC) && index == 0;
		}
	};
}

#endif
