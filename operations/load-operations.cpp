#ifndef JDECOMPILER_LOAD_OPERATIONS_CPP
#define JDECOMPILER_LOAD_OPERATIONS_CPP

namespace jdecompiler::operations {

	struct LoadOperation: Operation {
		public:
			const uint16_t index;
			const Variable& variable;

			LoadOperation(const Type* requiredType, const DecompilationContext& context, uint16_t index):
					index(index), variable(context.getCurrentScope()->getVariable(index, true)) {
				variable.castTypeTo(requiredType);
			}

			virtual string toString(const StringifyContext& context) const override {
				return context.getCurrentScope()->getNameFor(variable);
			}

			virtual const Type* getReturnType() const override {
				return variable.getType();
			}

			virtual void onCastReturnType(const Type* newType) const override {
				variable.setType(newType);
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

		virtual bool isReferenceToThis(const ClassInfo& classinfo) const override {
			return !(classinfo.modifiers & ACC_STATIC) && index == 0;
		}
	};
}

#endif
