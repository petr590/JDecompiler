#ifndef JDECOMPILER_DECLARE_VARIABLE_CPP
#define JDECOMPILER_DECLARE_VARIABLE_CPP

namespace jdecompiler::operations {

	struct DeclareVariableOperation: VoidOperation {
		public:
			const Variable& variable;

			DeclareVariableOperation(const Variable& variable): variable(variable) {}

			virtual string toString(const StringifyContext& context) const override {
				return variableDeclarationToString(variable.getType(), context.classinfo, context.getCurrentScope()->getNameFor(variable));
			}
	};
}

#endif
